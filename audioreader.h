#ifndef AUDIO_READER_H
#define AUDIO_READER_H

class Feature_extractor;

class Audio_reader {
public:
    long samples_per_frame;
    long hop_samples;
    double actual_frame_period;
    long frame_count; // number of analysis windows
    virtual void print_info() = 0;
    long read_window(float *data);
    virtual long read(float *data, long n) = 0;
    virtual double get_sample_rate() = 0;
    virtual long get_frames() = 0; // returns frames of input audio 
    // i.e. (samples/channels)
    void calculate_parameters(Feature_extractor &fe, bool verbose);
    Audio_reader(){};
    ~Audio_reader(){};
protected:
    bool reading_first_window;
    bool reading_last_window;
    float *temp_data;
};

#endif
