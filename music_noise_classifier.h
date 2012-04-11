/*
 *  music_noise_classifier.h
 *  rehearsal-seg-cluster
 *
 *  Created by Dawen Liang on 1/26/12.
 *  Copyright 2012 Carnegie Mellon University. All rights reserved.
 *
 */
#include<vector>
#include "constant.h"
using namespace std;

class Audio_clip;

class Music_Noise_Classifier {
public:
    Music_Noise_Classifier(){};
	void do_music_noise_classify(const char* filename, vector<Audio_clip> &clips);
private:
	void do_adaboost(const char* filename, vector<int> &pred_result, vector<float> &data_db);	
	void do_classify_no_smooth(vector<int> pred_result, vector<float> data_db, vector<int> &result_no_smooth); 
	void load_adaboost_paras(const char* f_ada, const char* f_em);
    void do_hmm(vector<int> &result_no_smooth, vector<float> &result_with_smooth);
};

