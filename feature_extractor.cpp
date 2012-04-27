#include <assert.h>
#include <algorithm>
#include "constant.h"
#include "feature_extractor.h"
#include "fft3/FFT3.h"
#include "math.h"
#include "rsc_utils.h"
#include "gen_chroma.h"


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
			sq_sum += frame_data[i] * frame_data[i];
		}

		*frame_db = 10 * log10(sq_sum / frame_size);

		float norm = sqrt(sq_sum);
		for (i = 0; i < frame_size; i++) {
			data_spec.push_back(frame_data[i] / norm);
		}
    } 

    free(fft_dataI);
    free(fft_dataR);
    free(frame_data);
	free(hamming);

	return has_next;
}	

void Feature_extractor::db_normalize(vector<float> &data_db) {
	vector<float> tmp = data_db;
	float db_stand;
	int i, size = tmp.size();

	sort(tmp.begin(), tmp.end());
	db_stand = tmp[(int)(0.95 * size + 0.5)];
#ifdef DEBUG
	printf("The standard db is %f\n", db_stand);
#endif
	for (i = 0; i < size; i++) {
		//data_out.push_back(data_in.at(i) + (DB_CENTER - db_stand));
		data_db[i] += (DB_CENTER - db_stand);
	}
}


int Feature_extractor::get_CENS(Audio_reader &reader, long nframes, vector<vector<float> > &data_cens) {
	int actual_nframes, i, j, k, l;
	float *chroma, *chroma_conv, *hanning, *chroma_bin;
	double actual_frame_period; 
	actual_nframes = gen_chroma(reader, HIGH_CUTOFF, LOW_CUTOFF, nframes, &chroma, &actual_frame_period);
	assert(actual_nframes == nframes);

	// generate the hanning window for long time statistics
	hanning = ALLOC(float, LONG_WINDOW);
	gen_Hanning(hanning, LONG_WINDOW);
	float sum = 0.0;
	// normalize the window
	for (i = 0; i < LONG_WINDOW; i++) {
		sum += hanning[i];
	}
	for (i = 0; i < LONG_WINDOW; i++) {
		hanning[i] /= sum;
	}

	// calcualte the chroma stat help, which is a quantization statistics of chroma 
	for (i = 0; i < nframes; i++) {
		for (j = 0; j < CHROMA_BIN_COUNT; j++) {
			for (k = 0; k < QUANT_SIZE; k++) {
				if (AREF2(chroma, i, j) > VEC_ENERGY[k]) {
					for (l = k; l < QUANT_SIZE; l++) {
						AREF2(chroma, i, j) += VEC_WEIGHT[l];
					}
				} else if (k == QUANT_SIZE - 1) {
					AREF2(chroma, i, j) = 0;
				}
			}
		}
	}

	// claculate CENS, which is the convoltion of chroma stat help
	// and hanning window, then downsampling by factor of HOP_SIZE_LONG
	for (i = 0; i < nframes; i += HOP_SIZE_LONG) {
		for (j = 0; j < CHROMA_BIN_COUNT; j++) {
			float *chroma_bin = (AREF1(chroma, i) + j);
			calculate_conv(chroma_bin, (int)LONG_WINDOW, hanning, (int)LONG_WINDOW, chroma_conv);
			for (k = 0; k < 2 * LONG_WINDOW - 1; k += HOP_SIZE_LONG) {
				int offset = (i / HOP_SIZE_LONG) *((2 * LONG_WINDOW - 1) / HOP_SIZE_LONG);
				if (j == 0) {
					vector<float> bin_cens;
					bin_cens.push_back(chroma_conv[k]);
					data_cens.push_back(bin_cens);
				} else {
					data_cens[(offset + k / HOP_SIZE_LONG)/HOP_SIZE_LONG].push_back(chroma_conv[k]);
				}
			}
		}
	}
	return 0;
}


/*
 *  Compute the convolution given the start position in one bin of chroma, and the window. Note the with row-wise chroma vector, the access for chroma of one pitch class is done within one column
 
   Ccurrently a n^2 imlementation, nlog(n) can be achieved with FFT (not now, FFT now doesn't take non-powerof2 length array
*/
void Feature_extractor::calculate_conv(float *chroma_bin, const int len1, float *window, const int len2, float *out) {
	int i, j, k, len_conv = len1 + len2 - 1;
	float chroma_at; 
	out = ALLOC(float, len_conv);
	for (i = 0; i < len_conv; i++) {
		for (j = 0, k = i; j < len1 && k >=0; j++, k--) {
			if (k < len2) {
				chroma_at = *(chroma + j * (CHROMA_BIN_COUNT + 1));
				out[i] += window[k] * chroma_at;
			}
		}
	}
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

void Feature_extractor::gen_Hanning(float *h, int n)
{
	int k;
	for (k = 0; k < n; k++) {
		float cos_value = (float) cos(2.0 * M_PI * k * (1.0 / (n - 1)));
		h[k] = 0.5 * (1 - cos_value);
	}
}
