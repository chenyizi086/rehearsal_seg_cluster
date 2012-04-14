#ifndef ADUIO_CLIP_H
#define AUDIO_READER_H

/*
 * A class to represent segments clip
 */
class Audio_clip {
public:
	Audio_clip(const char* _filename);
	void set_cluster_id(int _cluster_id) {cluster_id = _cluster_id;};
	const char* get_filename() {return filename;};
	void set_start(int _start) {start = _start;};
	void set_end(int _end) {end = _end;};
	int get_start() {return start;};
	int get_end() {return end;};
	int get_cluster_id() {return cluster_id;};
	bool is_centroid;
	bool is_music;
private:
	const char* filename;
	int start;
	int end;
	int cluster_id;
	Beat_map bmap;
};


#endif
