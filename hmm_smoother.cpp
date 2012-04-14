#include "hmm_smoother.h"
#include "math"
#include <vector>
#include <algorithm>

#define MAX(x, y) x > y ? x:y

void HMM_Smoother::do_smooth(vector<float> ada_raw_result, vector<int> &result_with_smooth) {
	vector<float> viterbi_m, viterbi_n;
	vector<float> p_os_m;		// p(observation | state = music)
	vector<float> p_os_n;		// p(observation | state = noise)
	vector<int> path_m, path_n;

	int i, ind, n_lframe = ada_raw_result.size();
	
	viterbi_m.push_back(log(p_init_m) + log(p_os_m[0]));
	viterbi_n.push_back(log(p_init_n) + log(p_os_n[0]));

	for (i = 1 ; i < n_lframe; i++) {

		// computer viterbi
		max_m = MAX(viterbi_m[i-1] + log(pm_m), viterbi_m + log(pm_n));
		ind_m = viterbi_m[i-1] + log(pm_m) > viterbi_m + log(pm_n) ? 0:1; 
		
		max_n = MAX(viterbi_n[i-1] + log(pn_m), viterbi_n + log(pn_n));
		ind_n = viterbi_n[i-1] + log(pn_m) > viterbi_n + log(pn_n) ? 0:1;

		viterbi_m = log(1.0 / (1 + exp(- ada_raw_result[i] * alpha)) / p_m) + max_m;
		viterbi_n = log((1 - 1.0 / (1 + exp(- ada_raw_result[i] * alpha))) / p_n) + max_n;

		path_m.push_back(ind_m);
		path_n.push_back(ind_n);

	}
	
	// trace back
	ind = viterbi_m[n_lframe - 1] > viterbi_n[n_lframe - 1] ? 0:1; 
	result_with_smooth.push_back(ind2label(ind));
	for (i = n_lframe - 1; i >= 0; i--) {
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
	reverse(result_no_smooth.begin(), result_no_smooth.end());
}

/*
 * given index of viterbi path (0 -> music, 1 -> music)
 * return label (1 -> music, -1 -> noise)
 */
int HMM_Smoother::ind2label(int ind) {
	return -2 * ind + 1;
}
