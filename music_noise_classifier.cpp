/*
 *  music_noise_classifier.cpp
 *  rehearsal-seg-cluster
 *
 *  Created by Dawen Liang on 1/26/12.
 *  Copyright 2012 Carnegie Mellon University. All rights reserved.
 *
 */

#include "music_noise_classifier.h"
#include "adaboost.h"
#include "hmm_smoother.h"
#include "audioreader.h"
#include "audiofilereader.h"

#define DEBUG

Feature_extractor fe;
Adaboost ada;

void Music_Noise_Classifier::load_adaboost_paras(const char* f_ada, const char* f_em) {
	ada.load_classifier(f_ada);
	ada.load_eigenmusic(f_em);
}


void Music_Noise_Classifier::do_music_noise_classify(const char *filename, vector<Audio_clip> &clips) {
	vector<int> pred_result, result_no_smooth, result_with_smooth;
	vector<float> data_db;

	do_adaboost(filename, pred_result, data_db);
	do_classify_no_smooth(pred_result, data_db, result_no_smooth);
	do_hmm(result_no_smooth, result_with_smooth);

	do_gen_clips(result_with_smooth, clips);
}

vector<int> Music_Noise_Classifier::do_adaboost(const char *filename, vector<int> &pred_result, vector<float> &data_db) {
	int i, count = 0;
	Audio_reader reader;
	vector<float> aver_spec, spectrum;
	float db;
	//no overlapping
	fe.set_parameters(SAMPLES_PER_FRAME / RESAMPLE_FREQ, SAMPLES_PER_FRAME / RESAMPLE_FREQ);
	reader.open(filename, fe, DEBUG);
	reader.resample(RESAMPLE_FREQ);
	
	while (1) {
		if (count % NUM_AVER == 0) {

			// take the average of NUM_AVER windows
			for (i = 0; i < aver_spec.size(); i++) {
				aver_spec.at(i) /= NUM_AVER;
			}
			pred_result.push_back(ada.do_prediction(aver_spec));			
			// reassign the average to 0
			aver_spec.assign(SAMPLES_PER_FRAME / 2 + 1, 0);
		}

		if (fe.get_spectrum(reader, spectrum, &db, DEBUG)) {
			data_db.push_back(db);
#ifdef DEBUG
			assert(aver_spec.size() == spectrum.size());
			assert(spectrum.size() == SAMPLES_PER_FRAME / 2 + 1);
#endif
			// taek the average of NUM_AVER windows
			for (i = 0; i < aver_spec.size(); i++) {
				aver_spec.at[i] += spectrum.at[i]; 
			}
			count++;
		} else {
			// handle the last frame, if any
			if (count % NUM_AVER != 0) {
				for (i = 0; i < aver_spec.size(); i++) {
					aver_spec.at(i) /= count % NUM_AVER;
				}
				pred_result.push_back(ada.do_prediction(aver_spec));
			}
			break;
		}
	}
}

void Music_Noise_Classifier::do_classify_no_smooth(vector<int> pred_result, vector<float> data_db, vector<int> &result_no_smooth) {
	vector<float> data_db_normalize;
	
	// normalize the db
	fe.db_normalize(data_db, data_db_normalize);



}
	
void Music_Noise_Classifier::do_hmm(vector<float> result_no_smooth, vector<float> &result_with_smooth) {
}
