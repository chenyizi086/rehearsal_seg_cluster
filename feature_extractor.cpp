#ifdef _WIN32
#include "malloc.h"
#endif

#include "stdlib.h" // for OSX compatibility, malloc.h -> stdlib.h
#include <assert.h>
#include <algorithm>
#include "constant.h"
#include "audioreader.h"
#include "feature_extractor.h"
#include "fft3/FFT3.h"
#include "math.h"
//#include <cmath>
#include "rsc_utils.h"
#include "stdio.h"
#include <string>
#include <fstream>

#ifdef DEBUG
#include <iostream> // cout
#endif
using namespace std;


bool FE_DEBUG_FLAG = true;


Feature_extractor::Feature_extractor() {
#ifdef DEBUG_LOG
	dbf = fopen("feature_extractor_debug.txt", "w");
	assert(dbf);
#endif
}

Feature_extractor::~Feature_extractor() {
#ifdef DEBUG_LOG
	fclose(dbf);
#endif
    
}

int Feature_extractor::get_spectrum(Audio_reader &reader, vector<float> &data_spec, bool verbose) {
	int i;
	int has_next = 0;
    //float sample_rate = reader.get_sample_rate();
	// output data should be empty
	if (!data_spec.empty()) {
		data_spec.clear();
	}
    
    if (verbose) {
        printf ("==============FILE ====================\n");
        reader.print_info();
    }
#ifdef DEBUG
    printf("******** BEGIN SPECTRUM COMPUTATION *********\n");
#endif
    
	// since our parameters for classifier are fixed, so frame_size should actually be the same with the size of parameters, as in ths case, samples_per_frame should be eual to frame_size
    int frame_size = nextPowerOf2(reader.samples_per_frame);
#ifdef FIXED_PARAS
	assert(frame_size == reader.samples_per_frame);
#endif
	if (verbose) {
        printf("   samples per frame is %ld \n", reader.samples_per_frame);
        printf("   total frames %ld\n", reader.frame_count); 
        // printf("   Window size  %g second \n", reader.window_size);
        printf("   hopsize in samples %ld \n", reader.hop_samples);
        printf("   fft size %d\n", frame_size);
    }
    
    float *frame_data = ALLOC(float, frame_size);
    float *fft_dataR = ALLOC(float, frame_size);
    float *fft_dataI = ALLOC(float, frame_size);	
    //set to zero
    memset(frame_data, 0, frame_size * sizeof(float));
    memset(fft_dataR, 0, frame_size * sizeof(float));	
    memset(fft_dataI, 0, frame_size * sizeof(float));
    //check to see if memory has been allocated
    assert(frame_data != NULL);
    assert(fft_dataR != NULL);
    assert(fft_dataI != NULL);
    
	// create Hamming window data
    float *hamming = ALLOC(float, reader.samples_per_frame);
    gen_Hamming(hamming, reader.samples_per_frame);
    
	if (reader.read_window(frame_data)) {
        //fill out array with 0's till next power of 2
#ifdef DEBUG
        printf("samples_per_frame %ld sample %g\n", 
               reader.samples_per_frame, frame_data[0]);
#endif
        for (i = reader.samples_per_frame; i < frame_size; i++) 
            frame_data[i] = 0;
        
#ifdef DEBUG_LOG
        printf("preFFT: frame_data[1000] %g\n", frame_data[1000]);
#endif
		has_next = 1;
        
		for (i = 0; i < reader.samples_per_frame; i++) {
            float x = frame_data[i];
            frame_data[i] = x * hamming[i];
        }
        
		FFT3(frame_size, 0, frame_data, NULL, fft_dataR, fft_dataI); //fft3
        
		//given the fft, compute the energy of each point
		gen_Magnitude(fft_dataR, fft_dataI, reader.samples_per_frame, frame_data);
        
		float sq_sum = 0.0;
		for (i = 0; i < frame_size / 2 + 1; i++) {
			sq_sum += frame_data[i] * frame_data[i];
		}
        
		float norm = sqrtf(sq_sum);
        
        if (norm < 0.000001) {
            norm = 1.0;   // avoid 0/0
        }
        
		for (i = 0; i < frame_size / 2 + 1; i++) {
			data_spec.push_back(frame_data[i] / norm);
		}
    } 
    
#ifdef DEBUG
    float sq_sum = 0;
    for (i = 0; i < data_spec.size(); i++) {
        sq_sum += data_spec[i] * data_spec[i];
    }
    cout << sq_sum << endl;
    if (sq_sum > 0.0001) {
        assert(sq_sum - 1 > -0.0001);
        assert(sq_sum - 1 < 0.0001);
    }
#endif
    
    free(fft_dataI);
    free(fft_dataR);
    free(frame_data);
	free(hamming);
    
	return has_next;
}	

