#ifndef HMM_SMOOTHER_H
#define HMM_SMOOTHER_H

class HMM_Smoother {
public:
    HMM_Smoother();
    void do_smooth(vector<float> ada_raw_result, vector<int> &result_with_smooth);
private:
	int ind2label(int ind);
};

#endif
