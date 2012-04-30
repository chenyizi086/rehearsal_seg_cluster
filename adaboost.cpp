#include "adaboost.h"
#include "constant.h"
#include "rsc_utils.h"

void Adaboost::load_classifier(const char *f_ada) {
    parameters = read_matfile(f_ada); 
}

void Adaboost::load_eigenmusic_inv(const char *f_em) {
    eigen_music_inv = read_matfile(f_em);
}

int Adaboost::do_prediction(vector<float> data, float* ada_raw) {
	vector<float> data_eigen;
	
	int i,j;
	float feature, sign, classifier;
	float tmp = 0;

	*ada_raw = 0;

	for (i = 0; i < eigen_music_inv.size(); i++) {
		for (j = 0; j < eigen_music_inv[0].size(); j++) {
			tmp += eigen_music_inv[i][j] * data[j];
		}
		data_eigen.push_back(tmp);
		tmp = 0;
	}

	for (i = 0; i < parameters.size(); i++) {
		feature = parameters[i][FEATURE_INDEX];
		sign = parameters[i][SIGN_INDEX];
		classifier = parameters[i][CLASSIFIER_INDEX];
	
		if (sign == 1) {
			*ada_raw += ((data_eigen[feature - 1] > classifier ? 1:-1) * parameters[i][ALPHA_INDEX]);
			//result_weak.push_back(data_eigen[feature - 1] > classifier ? 1:-1);
		} else {
			*ada_raw += ((data_eigen[feature - 1] < classifier ? 1:-1) * parameters[i][ALPHA_INDEX]);
			//result_weak.push_back(data_eigen[feature - 1] < classifier ? 1:-1);
		}
	}

	//for (i = 0; i < parameters.size(); i++) {
	//	*ada_raw += result_weak[i] * parameters[i][3];
	//}
	return *ada_raw > 0 ? 1:-1;
}


		
	
