#include <assert.h>
#include <algorithm>
#include "constant.h"
#include "feature_extractor.h"
#include "fft3/FFT3.h"
#include "math.h"
#include "rsc_utils.h"

#define DEBUG_LOG
#define FIXED_PARAS

int Feature_extractor::get_spectrum(Audio_reader &reader, vector<float> &data_spec, float *frame_db, bool verbose) {
	int i;
	int has_next = 0;
    float sample_rate = reader.get_sample_rate();
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
		for (i = 0; i < frame_size; i++) {
			data_spec.push_back(frame_data[i]);
			sq_sum += frame_data[i] * frame_data[i];
		}

		*frame_db = 10 * log10(sq_sum / frame_size);
    } 

    free(fft_dataI);
    free(fft_dataR);
    free(frame_data);
	free(hamming);

	return has_next;
}	

void Feature_extractor::db_normalize(vector<float> data_in, vector<float> &data_out) {
	vector<float> tmp = data_in;
	float db_stand;
	int i, size = tmp.size();

	sort(tmp.begin(), tmp.end());
	db_stand = tmp.at((int)(0.95 * size + 0.5));
#ifdef DEBUG
	printf("The standard db is %f\n", db_stand);
#endif
	for (i = 0; i < size; i++) {
		data_out.push_back(data_in.at(i) + (DB_CENTER - db_stand));
	}
}


int Feature_extractor::get_chroma_vector(Audio_reader &reader, vector<vector<float> > &data_spec) {
	return 0;
}

/*				GEN_MAGNITUDE
   given the real and imaginary portions of a complex FFT function, compute 
   the magnitude of the fft bin.

   NOTE: out should be length n
*/
void Feature_extractor::gen_Magnitude(float* inR, float* inI, int num_bin, float* out)
{
    int i;
    for (i = 0; i < num_bin; i++) {
      float magVal = sqrt(inR[i] * inR[i] + inI[i] * inI[i]);
      out[i]= magVal;
    }
}	

/*				GEN_HAMMING
    given data from reading in a section of a sound file
    applies the hamming function to each sample.
    n specifies the length of in and out.
*/
void Feature_extractor::gen_Hamming(float* h, int n)
{
    int k;
    for (k = 0; k < n; k++) {
      float cos_value = (float) cos(2.0 * M_PI * k * (1.0 / n));
        h[k] = 0.54F + (-0.46F * cos_value);
    }
}
