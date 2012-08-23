#ifndef ADUIO_CLIP_H
#define AUDIO_CLIP_H

#include "stdlib.h"
#include <iostream>
#include <string>
using namespace std;

/*
 * A class to represent segments
 */
class Audio_clip {
public:
	Audio_clip(string _filename);
    ~Audio_clip(){};
	void set_cluster_id(int _cluster_id) {cluster_id = _cluster_id;};
	string get_filename() {return filename;};
	void set_start(int _start) {start = _start;};
	void set_end(int _end) {end = _end;};
	int get_start() {return start;};
	int get_end() {return end;};
	int get_cluster_id() {return cluster_id;};
	bool is_centroid;
	bool is_music;
	friend ostream &operator << (ostream &os, Audio_clip &clip ) {
		os << clip.get_filename() << "\t" << clip.get_start() << "\t" << clip.get_end() << "\t" << int(clip.cluster_id);
        return os;
	};
    friend bool operator== (Audio_clip &one, Audio_clip &another) {
        if (one.filename == another.get_filename()) {
            if (one.start == another.get_start()) {
                if (one.end == another.get_end()) {
                    if (one.cluster_id == another.get_cluster_id()) {
                        return true;
                    }
                }
            }
        }
        return false;
    };

private:
	string filename;
	int start;
	int end;
	int cluster_id;
	//Beat_map bmap;
};

#endif
