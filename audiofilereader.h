#ifndef AUDIO_FILE_READER_H
#define AUDIO_FILE_READER_H

#define MAX_NAME_LEN 255
#include "sndfile.h"
#include "audioreader.h"

class Feature_extractor;

class Audio_file_reader: public Audio_reader {
public:
    Audio_file_reader(){};
    ~Audio_file_reader(){free(temp_data);};
    virtual long read(float *data, long n);
    SNDFILE *sf;
	SNDFILE *sf_rs;
    SF_INFO sf_info;
	SF_INFO sf_info_rs;
    char name[MAX_NAME_LEN + 1];
    int bytes_per_frame;
    long total_frames;
	long total_frames_rs;
    bool open(const char *filename, Feature_extractor &fe, bool verbose);
	bool open(const char *filename, Feature_extractor &fe, int start_frame, bool verbose);
    void close();
    double get_sample_rate();
	double get_sample_rate_rs();
	long get_frames_rs();
    long get_frames();
    void print_info();
	void resample(int rsamplerate);
};

#endif
