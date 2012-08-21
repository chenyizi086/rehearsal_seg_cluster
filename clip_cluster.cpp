#include "audio_clip.h"
#include "audiofilereader.h"
#include "clip_cluster.h"
#include "constant.h"
#include "feature_extractor.h"
#include "rsc_utils.h"

#include "assert.h"
#include "dirent.h"
#include "sys/types.h"

#include <algorithm>
#include <sstream>
#include <string>


#ifdef DEBUG
#include <iostream>
#endif

Feature_extractor fe_clip;

bool CLUSTER_DEBUG_FLAG = true;

Clip_cluster::Clip_cluster() {
    load_all_temp_cens();
    load_database();
}

Clip_cluster::~Clip_cluster() {
    int i, j;
    if (!all_temp_cens.empty()) {
        for (i = 0; i < all_temp_cens.size(); i++) {
            for (j = 0; j < all_temp_cens[i].size(); j++) {
                free(all_temp_cens[i][j]);
            }
        }
    }
    
    if (atc_write.is_open()) {
        atc_write.close();
    }
    if (db_write.is_open()) {
        db_write.close();
    }
}

void Clip_cluster::load_database() {
    string line;
    ifstream db_read("databases.txt");
#ifdef DEBUG    
    int count = 0;
#endif
    // Load table from file
    while (getline(db_read, line))
    {
        string filename; 
        int start, end, cluster_id;
        istringstream is(line);
        is >> filename;
        is >> start;
        is >> end;
        is >> cluster_id;
        
        cout << filename << endl;
        cout << start << endl;
        cout << end << endl;
        cout << cluster_id << endl;
        
        Audio_clip *clip = new Audio_clip(filename);
        clip->set_start(start);
        clip->set_end(end);
        clip->is_music = true;
        clip->set_cluster_id(cluster_id);
        
        vector<Audio_clip *> db_clips = db[cluster_id];
        db_clips.push_back(clip);
        db[cluster_id] = db_clips;
#ifdef DEBUG
        count++;
        cout << "db for cluster " << cluster_id << endl;
        for (int i = 0; i < db[cluster_id].size(); i++) {
            cout << db[cluster_id][i]->get_filename() << " " << db[cluster_id][i]->get_start() << " " << db[cluster_id][i]->get_cluster_id() << endl;
        }
#endif
    }
#ifdef DEBUG
    cout << count << " clips have been read" << endl;
#endif
    db_read.close();
}

/*
string Clip_cluster::get_cluster_clips(string clip) {
    ifstream cens_info_read;
    string line, result = "";
    stringstream ss;
    for (int i = 0; i < nclusters; i++) {
        string path = "./CENS_centinfo/" + int2str(i);
        cens_info_read.open(path.c_str());
        getline(cens_info_read, line);
        if (line == clip) {
            for (int j = 0; j < db[i].size(); j++) {
                Audio_clip *tmp = db[i][j];
                ss >> *tmp >> endl;
            }
            result = ss.str();
            break;
        }
    }
    return result;
}
 */

void Clip_cluster::do_cluster(vector<Audio_clip *> &clips) {
    int i, j;
    map<int, vector<Audio_clip *> >::iterator it;
    vector<Audio_clip *> db_clips;
    Audio_clip *clip;
    
#ifdef DEBUG
    cout << "=============== START CLUSTERING ===============" << endl;
    cout << "Number of clips: " << clips.size() << endl;
#endif
    
    //50% overlapping
    fe_clip.set_parameters(HOP_SIZE_CHROMA / RESAMPLE_FREQ, SAMPLES_PER_FRAME_CHROMA / RESAMPLE_FREQ);
    // Load all the templates CENS for cluster centroids if any 
    for (i = 0; i < clips.size(); i++) {
#ifdef DEBUG
        cout << endl;
        cout << "******************** TO CLUSTER CLIP " << i << "***********************" << endl;
#endif
        clip = clips[i];
        do_clip_cluster(clip);
        
        bool exist = false;
        for (it = db.begin();it != db.end(); it++) { 
            db_clips = (*it).second;
            for (j = 0; j < db_clips.size(); j++) {
                if (*clip == *db_clips[j]) {
#ifdef DEBUG
                    printf("The cilp in file %s, starting from %d to %d is in the database already\n", (clip->get_filename()).c_str(), clip->get_start(), clip->get_end());
#endif
                    exist = true;
                    break;
                }
            }
        }
        if (!exist) {
            write_to_db(clip);
        }
    }
#ifdef DEBUG
    cout << "********************** Processing Summary **************************" << endl;
    for (i = 0; i < clips.size(); i++) {
        cout << *clips[i] << endl;
    }
    cout << endl;
#endif
}


void Clip_cluster::write_to_db(Audio_clip *clip) {
    stringstream ss;
    db_write.open("databases.txt", ios_base::app);
    ss << *clip << endl;
    string out = ss.str();
    db_write.write(out.c_str(), out.size());
    db_write.close();
    
    vector<Audio_clip *> db_clips = db[clip->get_cluster_id()];
    db_clips.push_back(clip);
    db[clip->get_cluster_id()] = db_clips;
}


void Clip_cluster::do_clip_cluster(Audio_clip *clip) {
    string atc_readname;
    string atc_rsample_name;
    int start, end;
    long nframes;
    vector<float*> data_cens;
    vector<float> min_dist;
    
    clock_t start_t, end_t;
    
    Audio_file_reader reader_clip;
    
    atc_readname = clip->get_filename();
    atc_rsample_name = "resample_" + string(atc_readname);
    
    start = clip->get_start();
    end = clip->get_end();
    
    min_dist.assign(nclusters, 0.0f);
    
    reader_clip.open(atc_rsample_name.c_str(), fe_clip, start * SAMPLES_PER_FRAME * NUM_AVER, 0, CLUSTER_DEBUG_FLAG);
    
    nframes = (end - start + 1) * NUM_AVER * (SAMPLES_PER_FRAME_CHROMA / HOP_SIZE_CHROMA);
    
    start_t = clock();
    fe_clip.get_CENS(reader_clip, nframes, data_cens);
    end_t = clock();
    cout << "Running Time for CENS : " << (double) (end_t - start_t) / CLOCKS_PER_SEC << endl;

    start_t = clock();
    
    if (all_temp_cens.size() != 0) {
        compare_and_cluster(clip, data_cens, min_dist);
    } else {
        if (nclusters == 0) {
            nclusters++;        
            write_to_atc(data_cens, nclusters-1);
            clip->is_centroid = true;
            clip->set_cluster_id(nclusters - 1);
            write_to_cent_info(clip);
            
            all_temp_cens.push_back(data_cens);
        } else {
            compare_and_cluster(clip, data_cens, min_dist);                    
        }
    }
    
    end_t = clock();
    cout << "Running Time for matching and clustering : " << (double) (end_t - start_t) / CLOCKS_PER_SEC << endl;

}

void Clip_cluster::compare_and_cluster(Audio_clip *clip, vector<float*> &data_cens, vector<float> &min_dist) {
    int i, index;
    float m_dist, min_best;    
    vector<int> exchanges;
    
    for (i = 0; i < nclusters; i++) {
        vector<float> dists;
        int exchange = dist_CENS(data_cens, all_temp_cens[i], dists);
        
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
            clip->set_cluster_id(index);
            write_to_cent_info(clip);
            all_temp_cens[index] = data_cens;
        }  else {
            clip->is_centroid = false;
            clip->set_cluster_id(index);
        }
    } else {
        // make it a new cluster
        nclusters++;
        write_to_atc(data_cens, nclusters - 1);
        clip->set_cluster_id(nclusters - 1);
        clip->is_centroid = true;
        
        all_temp_cens.push_back(data_cens); 
        write_to_cent_info(clip);
    }
}    

void Clip_cluster::write_to_atc(vector<float*> &data_cens, int cluster_id) {
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
    
    int i, j, k, begin, end, length, len_q, len_t = template_cens.size();
    float dot_prod, sum;
    
    // only the middle SEGMENT_LENGTH long frames (1.024 sec) are used for compare
    length = query_cens.size();
    if (length <= SEGMENT_LENGTH) {
#ifdef DEBUG
        cout << "Length less than " << SEGMENT_LENGTH << " frames, use original length" << endl;
#endif
        begin = 0;
        end = length;
    } else {
        begin = (length - SEGMENT_LENGTH) / 2;
        end = begin + SEGMENT_LENGTH;
#ifdef DEBUG
        printf("Larger than %d frames, use middle parts from %d to %d\n", SEGMENT_LENGTH, begin, end);
#endif
    }
    len_q = end - begin;
    
    for (i = 0; i < len_t - len_q + 1; i++) {
        sum = 0;
	    for (j = 0; j < CHROMA_BIN_COUNT; j++) {
            dot_prod = 0;
	        for (k = 0; k < len_q; k++) {
                dot_prod += query_cens[k + begin][j] * template_cens[k + i][j];
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

void Clip_cluster::write_to_cent_info(Audio_clip *clip) {
    ofstream centinfo_write;
    string path;
    stringstream ss;

    path = "./CENS_centinfo/" +int2str(clip->get_cluster_id());
    centinfo_write.open(path.c_str(), ios_base::trunc);
    ss << *clip << endl;
    string out = ss.str();
    centinfo_write.write(out.c_str(), out.size());
    centinfo_write.close();
}
    
    
    








