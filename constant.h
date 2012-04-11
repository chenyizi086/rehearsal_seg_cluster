#ifndef CONSTANT_H
#define CONSTNAT_H

// to turn on debug flag
bool DEBUG = true;
#define DEBUG

/*
 * Constant numbers for adaboost audio processing
 */
const int SAMPLES_PER_FRAME = 4096;
const int RESAMPLE_FREQ = 16000;	
const int NUM_AVER = 5;		// the spectrums will be averaged every 5 frames

const int DB_CENTER = 20;
const int DB_THRESHOLD = -10;

class Audio_clip {
public:
	Audio_clip(const char* _filename, int _start, int _end) {filename = _filename; start = _start; end = _end;};
	void set_cluster_id(int _cluster_id) {cluster_id = _cluster_id;};
	const char* get_filename() {return filename;};
	int get_start() {return start;};
	int get_end() {return end;};
	int get_cluster_id() {return cluster_id;};
private:
	const char* filename;
	int start;
	int end;
	int cluster_id;
	//Beat_map bmap;
	bool is_centroid;
};


#endif

