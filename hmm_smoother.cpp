#include "hmm_smoother.h"
#include "math.h"
#include "constant.h"
#include <vector>
#include <algorithm>

#define MAX(x, y) x > y ? x:y

void HMM_smoother::do_smooth(vector<float> ada_raw_result, vector<int> &result_with_smooth) {
	vector<float> viterbi_m, viterbi_n;
	vector<int> path_m, path_n;
	int ind, n_lframe = ada_raw_result.size();
	
	// put the init values for viterbi
	viterbi_m.push_back(log(p_init_m) + logistic(ada_raw_result[0], 1));
	viterbi_n.push_back(log(p_init_n) + logistic(ada_raw_result[0], 0));

	compute_viterbi(viterbi_m, viterbi_n, path_m, path_n, n_lframe, ada_raw_result);
		
	// decode from the last
	ind = viterbi_m[n_lframe - 1] > viterbi_n[n_lframe - 1] ? 0:1; 
	viterbi_decode(result_with_smooth, path_m, path_n, ind, n_lframe);
}

void HMM_smoother::compute_viterbi(vector<float> &viterbi_m, vector<float> &viterbi_n, vector<int> &path_m, vector<int> &path_n, int n_lframe, vector<float> ada_raw_result) {
    float max_m, max_n;
	int ind_m, ind_n;

	for (int i = 1 ; i < n_lframe; i++) {

		// computer viterbi
		max_m = MAX(viterbi_m[i-1] + log(pm_m), viterbi_m[i-1] + log(pm_n));
		ind_m = viterbi_m[i-1] + log(pm_m) > viterbi_m[i-1] + log(pm_n) ? 0:1; 
		
		max_n = MAX(viterbi_n[i-1] + log(pn_m), viterbi_n[i-1] + log(pn_n));
		ind_n = viterbi_n[i-1] + log(pn_m) > viterbi_n[i-1] + log(pn_n) ? 0:1;

		viterbi_m[i-1] = logistic(ada_raw_result[i], 1) + max_m;
		viterbi_n[i-1] = logistic(ada_raw_result[i], 0) + max_n;

		path_m.push_back(ind_m);
		path_n.push_back(ind_n);

	}
}

void HMM_smoother::viterbi_decode(vector<int> &result_with_smooth, vector<int> path_m, vector<int> path_n, int ind, int n_lframe) {
    // trace back
	result_with_smooth.push_back(ind2label(ind));
	for (int i = n_lframe - 1; i >= 0; i--) {
		if (ind == 0) {
			result_with_smooth.push_back(ind2label(path_m[i]));
			ind = path_m[i];
		} else {
			result_with_smooth.push_back(ind2label(path_n[i]));
			ind = path_n[i];
		}
	}
	// add the first long_frame the same as the second  long_frame and 
	// then reverse it to the actual order
	result_with_smooth.push_back(ind2label(ind));
	reverse(result_with_smooth.begin(), result_with_smooth.end());
}

/*
 * given index of viterbi path (0 -> music, 1 -> music)
 * return label (1 -> music, -1 -> noise)
 */
int HMM_smoother::ind2label(int ind) {
	return -2 * ind + 1;
}


float HMM_smoother::logistic(float ada_raw, int is_music) {
	if (is_music) {
		return log(1.0 / (1 + exp(- ada_raw * alpha)) / p_m);
	} else {
		return log((1 - 1.0 / (1 + exp(- ada_raw * alpha))) / p_n);
	}
}
