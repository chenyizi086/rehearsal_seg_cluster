#ifndef CLIP_CLUSTER_H
#define CLIP_CLUSTER_H

#include <vector>
#include <fstream>
#include <map>

using namespace std;

class Audio_clip;

class Clip_cluster {
public:
    Clip_cluster();
    ~Clip_cluster();
    void do_cluster(vector<Audio_clip *> &clips);
private:
    void do_clip_cluster(Audio_clip *clip);
    void compare_and_cluster(Audio_clip *clip, vector<float*> &data_cens, vector<float> &min_dist);
    int dist_CENS(vector<float*>  &query_cens, vector<float*> &template_cens, vector<float> &dists);
    void write_to_atc(vector<float*> &data_cens, int cluster_id);
    void write_to_db(Audio_clip *clip);
    void write_to_cent_info(Audio_clip *clip);
    int get_nclusters(vector<string> &files);
    int load_all_temp_cens();
    vector<vector<float*> > all_temp_cens;
    void load_database();
    vector<float*> read_temp(string filename);
    map<int, vector<Audio_clip *> > db;
    ofstream atc_write;
    ofstream db_write;
    int nclusters;
};
#endif

