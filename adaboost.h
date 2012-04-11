#define AD_DFT_FRAME_PERIOD 1.25
#define AD_DFT_FRAME_PERIOD_TEXT wxT("1.25 secs")

#include<vector>
using namespace std;

class Adaboost: public Machine_Learner {
public:
	Adaboost();
	~Adaboost();
    
    void load_classifier(const char *f_ada);
    void load_eigenmusic(const char *f_em);
	int do_prediction(vector<float> data);

	void 
private:
    vector<vector<float> > parameters;
    vector<vector<float> > eigen_music;
};
