#ifndef AUDIO_FILE_READER_H
#define AUDIO_FILE_READER_H

#define MAX_NAME_LEN 255
#include "sndfile.h"
#include "audioreader.h"

using namespace std;

class Feature_extractor;

class Audio_file_reader: public Audio_reader {
public:
    Audio_file_reader(){};
    ~Audio_file_reader(){};
    virtual long read(float *data, long n);
    SNDFILE *sf;
    SF_INFO sf_info;
    char name[MAX_NAME_LEN + 1];
    int bytes_per_frame;
    long total_frames;
    bool open(const char *filename, Feature_extractor &fe, int rsamplerate, bool verbose);
	bool open(const char *filename, Feature_extractor &fe, int start_frame, int rsamplerate, bool verbose);
    void close();
    double get_sample_rate();
    long get_frames();
    void print_info();
	string resample(int rsamplerate);
private:
    sf_count_t sample_rate_convert(SNDFILE *sf_rs, int converter, double src_ratio, int channels, double *gain);	
};

#endif
