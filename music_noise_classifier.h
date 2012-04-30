#ifndef MUSIC_NOISE_CLASSIFIER_H
#define MUSIC_NOISE_CLASSIFIER_H

#include <vector>
using namespace std;

class Audio_clip;

class Music_noise_classifier {
public:
    Music_noise_classifier(){};
	void do_music_noise_classify(const char* filename, vector<Audio_clip> &clips);
private:
	void do_adaboost(const char* filename, vector<int> &pred_result, vector<float> &ada_raw_result, vector<float> &data_db);	
	void load_adaboost_paras(const char* f_ada, const char* f_em);
	void do_gen_clips(const char* filename, vector<int> result_with_smooth, vector<Audio_clip> &clips);
};

#endif

