#include "adaboost.h"
#include "audio_clip.h"
#include "audiofilereader.h"
#include "assert.h"
#include "constant.h"
#include "hmm_smoother.h"
#include "feature_extractor.h"
#include "music_noise_classifier.h"

#include <ctime>
#include <iostream>
using namespace std;

Feature_extractor fe_mnc;
Adaboost ada;
HMM_smoother hmms;

bool MNC_DEBUG_FLAG = false;

Music_noise_classifier::Music_noise_classifier(const char* f_ada, const char* f_em) {
    load_adaboost_paras(f_ada, f_em);
}

Music_noise_classifier::~Music_noise_classifier() {
    int count = all_clips.size();
    for (int i = 0; i < count; i++) {
        delete(all_clips[i]);
    }
    
}

void Music_noise_classifier::load_adaboost_paras(const char* f_ada, const char* f_em) {
#ifdef DEBUG
    cout << "Loading AdaBoost parameters..." << endl;
#endif
	ada.load_classifier(f_ada);
	ada.load_eigenmusic_inv(f_em);
}

void Music_noise_classifier::do_music_noise_classify(const char *filename, vector<Audio_clip *> &clips) {
	vector<int> pred_result, result_no_smooth, result_with_smooth;
	vector<float> ada_raw_result;
    
//#ifdef DEBUG
    clock_t start, end;
//#endif
    
    start = clock();
	do_adaboost(filename, pred_result, ada_raw_result);
    end = clock();
    cout << "Running Time for adaboost : " << (double) (end - start) / CLOCKS_PER_SEC << endl;

    start = clock();
	hmms.do_smooth(ada_raw_result, result_with_smooth);
    end = clock();
    cout << "Running Time for hmm : " << (double) (end - start) / CLOCKS_PER_SEC << endl;

	do_gen_clips(filename, result_with_smooth, clips);
}

void Music_noise_classifier::do_adaboost(const char *filename, vector<int> &pred_result, vector<float> &ada_raw_result) {
	int i, count = 0;
	Audio_file_reader reader;
	vector<float> aver_spec, spectrum;
	float ada_raw;
    clock_t start, end;
    
    double running_ada = 0;
    
	//no overlapping
	fe_mnc.set_parameters(SAMPLES_PER_FRAME / RESAMPLE_FREQ, SAMPLES_PER_FRAME / RESAMPLE_FREQ);
	
    reader.open(filename, fe_mnc, RESAMPLE_FREQ, MNC_DEBUG_FLAG);
    
    /*
    string t = "resample_" + string(filename);
    reader.open(t.c_str(), fe_mnc, 0, MNC_DEBUG_FLAG);
     */
    
    //init aver_spec
    aver_spec.assign(SAMPLES_PER_FRAME / 2 + 1, 0);
	
	while (1) {
		if (count != 0 && count % NUM_AVER == 0) {

			// take the average of NUM_AVER windows
			for (i = 0; i < aver_spec.size(); i++) {
				aver_spec[i] /= NUM_AVER;
			}
            start = clock();
            pred_result.push_back(ada.do_prediction(aver_spec, &ada_raw));
            end = clock();
            
            running_ada += (double)(end - start) / CLOCKS_PER_SEC;
            
			ada_raw_result.push_back(ada_raw);
			// reassign the average to 0
			aver_spec.assign(SAMPLES_PER_FRAME / 2 + 1, 0);
		}

		if (fe_mnc.get_spectrum(reader, spectrum, MNC_DEBUG_FLAG)) {            
#ifdef DEBUG
			assert(aver_spec.size() == spectrum.size());
			assert(spectrum.size() == SAMPLES_PER_FRAME / 2 + 1);
#endif
			for (i = 0; i < aver_spec.size(); i++) {
				aver_spec[i] += spectrum[i]; 
			}
			count++;
		} else {
			// handle the last frame, if any
			if (count % NUM_AVER != 0) {
				for (i = 0; i < aver_spec.size(); i++) {
					aver_spec[i] /= count % NUM_AVER;
				}
				pred_result.push_back(ada.do_prediction(aver_spec, &ada_raw));
            
                start = clock();
				ada_raw_result.push_back(ada_raw);
                end = clock();
                
                running_ada += (double)(end - start) / CLOCKS_PER_SEC;
			}
			break;
		}
	}
    
    cout << "Running Time for p_ada : " << running_ada << endl;

}

void Music_noise_classifier::do_gen_clips(const char* filename, vector<int> result_with_smooth, vector<Audio_clip *> &clips) {
	bool new_clip_flag = true;
	int cur;
    Audio_clip *clip;
    int count = result_with_smooth.size();
	for (int i = 0; i < count; ) {
        //Audio_clip clip(filename);
		if (new_clip_flag) {
            clip = new Audio_clip(filename);
			clip->set_start(i);
			clip->is_music = (result_with_smooth[i] == 1);
			cur = result_with_smooth[i];
			i++;
			new_clip_flag = false;
			continue;
		}
        
		if (result_with_smooth[i] != cur || i == count - 1) {
            int offset = (int)(i == count - 1);
			clip->set_end(i-1 + offset);
            
			if (clip->is_music && clip->get_end() - clip->get_start() >= int(MIN_LENGTH_SECS / (NUM_AVER * SAMPLES_PER_FRAME / RESAMPLE_FREQ) + 0.5)) {
				clips.push_back(clip);
                all_clips.push_back(clip);
			}
			new_clip_flag = true;
			continue;
		} 
		i++;
	}
}

