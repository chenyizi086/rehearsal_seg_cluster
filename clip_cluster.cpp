#include "clip_cluster.h"
#include "feature_extractor.h"
#include <string>
#include "rsc_utils.h"
#include "sys/types.h"
#include "dirent.h"
#include "assert.h"
#include <algorithm>
#include "audio_clip.h"
#include "audiofilereader.h"
#include "constant.h"
#include <sstream>

#ifdef DEBUG
#include <iostream>
#endif

Feature_extractor fe_clip;

bool CLUSTER_DEBUG_FLAG = true;

Clip_cluster::Clip_cluster() {
    load_all_temp_cens();
}

Clip_cluster::~Clip_cluster() {
    if (!all_temp_cens.empty()) {
        for (int i = 0; i < all_temp_cens.size(); i++) {
            for (int j = 0; j < all_temp_cens[0].size(); j++) {
                free(all_temp_cens[i][j]);
            }
        }
    }
    if (atc_write.is_open()) {
        atc_write.close();
    }
}

void Clip_cluster::do_cluster(vector<Audio_clip *> &clips) {
    int i;
    
#ifdef DEBUG
    cout << "=============== START CLUSTERING ===============" << endl;
    cout << "Number of clips: " << clips.size() << endl;
#endif
    
    //50% overlapping
    fe_clip.set_parameters(HOP_SIZE_CHROMA / RESAMPLE_FREQ, SAMPLES_PER_FRAME_CHROMA / RESAMPLE_FREQ);
    // Load all the templates CENS for cluster centroids if any 
    for (i = 0; i < clips.size(); i++) {
        do_clip_cluster(clips[i]);	
    }
}

void Clip_cluster::do_clip_cluster(Audio_clip *clip) {
    char *atc_readname;
    string atc_rsample_name;
    int start, end;
    long nframes;
    vector<float*> data_cens;
    vector<float> min_dist;
    
    Audio_file_reader reader_clip;
    
    atc_readname = (char*)clip->get_filename();
    atc_rsample_name = "resample_" + string(atc_readname);

    start = clip->get_start();
    end = clip->get_end();
    
    min_dist.assign(nclusters, 0.0f);
    
    reader_clip.open(atc_rsample_name.c_str(), fe_clip, start * SAMPLES_PER_FRAME * NUM_AVER, 0, CLUSTER_DEBUG_FLAG);
#ifdef DEBUG
    reader_clip.print_info();
#endif
    nframes = (end - start + 1) * NUM_AVER * (SAMPLES_PER_FRAME_CHROMA / HOP_SIZE_CHROMA);
    fe_clip.get_CENS(reader_clip, nframes, data_cens);
    
    if (all_temp_cens.size() != 0) {
        compare_and_cluster(clip, data_cens, min_dist);
    } else {
        if (nclusters == 0) {
            nclusters++;        
            write_to_atc(data_cens, nclusters-1);
            clip->is_centroid = true;
            clip->set_cluster_id(nclusters - 1);
            
            all_temp_cens.push_back(data_cens);
        } else {
            compare_and_cluster(clip, data_cens, min_dist);                    
        }
    }
}

void Clip_cluster::compare_and_cluster(Audio_clip *clip, vector<float*> &data_cens, vector<float> &min_dist) {
    int i, index;
    float m_dist, min_best;    
    vector<int> exchanges;
    
    for (i = 0; i < nclusters; i++) {
        vector<float> dists;
        int exchange = dist_CENS(data_cens, all_temp_cens[i], dists);

#ifdef DEBUG
        for (int j = 0; j < dists.size(); j++) {
            cout << dists[j] << " ";
        }
        cout << endl;
#endif
        
        exchanges.push_back(exchange);
        
        int dummy;
        m_dist = vector_min(dists, &dummy);
        min_dist[i] = m_dist;
    }
    
    min_best = vector_min(min_dist, &index);
#ifdef DEBUG
    cout << "Best match distance: " << min_best << " with cluster " << index << endl;
#endif
    if (min_best <= MATCHING_THRESHOLD) {
        // cluster them together
        if (exchanges[index]) {
            // replace the template with this new one
            write_to_atc(data_cens, index); 
            all_temp_cens[index] = data_cens;
            clip->is_centroid = true;
            // TODO:
            // remove the centroid label for the original centroid
            
            all_temp_cens[index] = data_cens;
        }  else {
            clip->is_centroid = false;
        }
        clip->set_cluster_id(index);
    } else {
        // make it a new cluster
        nclusters++;
        write_to_atc(data_cens, nclusters - 1);
        clip->set_cluster_id(nclusters - 1);
        clip->is_centroid = true;
    }
}    

void Clip_cluster::write_to_atc(vector<float*> &data_cens, int cluster_id) {
    ofstream atc_write;
    stringstream ss;
    string path = "./CENS_cent/" + int2str(cluster_id);
    atc_write.open(path.c_str(), ios_base::trunc);
    
    long pos = ss.tellp();
    ss.seekp(pos);
    
    for (int i = 0; i < data_cens.size(); i++) {
        for (int j = 0; j < CHROMA_BIN_COUNT; j++) {
            
            ss << data_cens[i][j];
            if (j != CHROMA_BIN_COUNT - 1) {
                ss << " ";
            }
        }
        ss << "\n";
    }
    
    string output = ss.str();
    atc_write.write(output.c_str(), output.size());
    atc_write.close();
    
}


/*
 * return 0 indicates that query is longer than the template
 */
int Clip_cluster::dist_CENS(vector<float*>  &query_cens, vector<float*> &template_cens, vector<float> &dists) {
    if (query_cens.size() > template_cens.size()) {
        dist_CENS(template_cens, query_cens, dists);
        return 1;
    }
    
    int i, j, k, len_q = query_cens.size(), len_t = template_cens.size();
    float dot_prod, sum;
    for (i = 0; i < len_t - len_q + 1; i++) {
        sum = 0;
	    for (j = 0; j < CHROMA_BIN_COUNT; j++) {
            dot_prod = 0;
	        for (k = 0; k < len_q; k++) {
                dot_prod += query_cens[k][j] * template_cens[k + i][j];
            }
            sum += dot_prod;
	    }
        dists.push_back(1 - sum/len_q);
    }
    return 0;
}

int Clip_cluster::load_all_temp_cens() {
    // the number of clusters is stored in a separate atc_read
    vector<string> files;
    nclusters = get_nclusters(files);
    if (nclusters == -1) {
        return -1;
    }
    
    if (nclusters == 0) {
        return -1;
    }
    
    for (int i = 0; i < nclusters; i++) {
        all_temp_cens.push_back(read_temp(files[i]));
    }  
    return 0;
}

/*
 * Get the number of clusters by counting the number of files in the CENS centroid folder (one file represents one centroid), and also get the filenames at the same time
 */
int Clip_cluster::get_nclusters(vector<string> &files) {
    DIR *dp;
    string name;
    struct dirent *dirp;
    int count = 0;
    if((dp  = opendir("./CENS_cent")) == NULL) {
        cout << "Error opening CENS centroid directory" << endl;
        return -1;
    }
    
    while ((dirp = readdir(dp)) != NULL) {
        name = string(dirp->d_name);
        // ignore the hidden files
        if (name.size() <= 0 || name[0] == '.') {
            continue;
        }
        files.push_back(name);
        count++;
    }
    closedir(dp);
    return count;
}

vector<float*> Clip_cluster::read_temp(string filename) {
    vector<float*> cens;    
    string path = "./CENS_cent/" + filename;
    vector<vector<float> > raw = read_matfile(path.c_str());
    int count = raw.size();
    for (int i = 0; i < count; i++) {
        float* tmp = ALLOC(float, CHROMA_BIN_COUNT);
        for (int j = 0; j < CHROMA_BIN_COUNT; j++) {
            tmp[j] = raw[i][j];
        }
        cens.push_back(tmp);
    }
    return cens;
}
    
    
    
    
    
    
    
    
