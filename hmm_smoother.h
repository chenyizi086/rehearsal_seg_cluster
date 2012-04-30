#ifndef HMM_SMOOTHER_H
#define HMM_SMOOTHER_H

#include <vector>
using namespace std;

class HMM_smoother {
public:
    HMM_smoother(){};
    ~HMM_smoother(){};
    void do_smooth(vector<float> ada_raw_result, vector<int> &result_with_smooth);
private:
    void compute_viterbi(vector<float> &viterbi_m, vector<float> &viterbi_n, vector<int> &path_m, vector<int> &path_n, int n_lframe, vector<float> ada_raw_result);
	void viterbi_decode(vector<int> &result_with_smooth, vector<int> path_m, vector<int> path_n, int ind, int n_lframe);
	int ind2label(int ind);
	float logistic(float ada_raw, int is_music);
};

#endif
