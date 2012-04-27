#ifndef CLIP_CLUSTER_H
#define CLIP_CLUSTER_H

#include <vector>

class Clip_cluster {
public:
	Clip_cluster(){};
	void do_cluster(vector<Audio_clip> &clips);
private:
	void do_clip_cluster(Audio_clip clip);
	float dist_CENS(vector<vector<float> > query_cens, vector<vector<float> > template_cens);

	void load_all_temp_cens();
	vector<vector<float> > all_temp_cens[];

	void load_database();

};
#endif

