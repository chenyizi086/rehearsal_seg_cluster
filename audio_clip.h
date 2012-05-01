#ifndef ADUIO_CLIP_H
#define AUDIO_CLIP_H

#include "stdlib.h"
#include <iostream>
using namespace std;
/*
 * A class to represent segments clip
 */
class Audio_clip {
public:
	Audio_clip(const char* _filename);
    ~Audio_clip(){};
	void set_cluster_id(int _cluster_id) {cluster_id = _cluster_id;};
	const char* get_filename() {return filename;};
	void set_start(int _start) {start = _start;};
	void set_end(int _end) {end = _end;};
	int get_start() {return start;};
	int get_end() {return end;};
	int get_cluster_id() {return cluster_id;};
	bool is_centroid;
	bool is_music;
	friend ostream &operator << (ostream &os, Audio_clip &clip )
    {
		os << clip.get_filename() << "\t[" << clip.get_start() << ", " << clip.get_end() << "]\t" << int(clip.is_music);
        return os;
	}

	//friend istream &operator >> (istream &is, Audio_clip &clip) {
	//	is >> clip.filename >> "\t[" >> clip.start >> ", " >> clip.end << "]\t" << clip.is_music;
	//	return is;
	//}
private:
	const char* filename;
	int start;
	int end;
	int cluster_id;
	//Beat_map bmap;
};

#endif
