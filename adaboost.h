#define AD_DFT_FRAME_PERIOD 1.25
#define AD_DFT_FRAME_PERIOD_TEXT wxT("1.25 secs")

#include<vector>
using namespace std;

class Adaboost: public Machine_Learner {
public:
	Adaboost();
	~Adaboost();
    
    void load_classifier(const char *f_ada);
    void load_eigenmusic_inv(const char *f_em);
	int do_prediction(vector<float> data, float* ada_raw);

private:
    vector<vector<float> > parameters;		//    30 * 4
    vector<vector<float> > eigen_music_inv;		// 10 * fft_bin/2+1
};
