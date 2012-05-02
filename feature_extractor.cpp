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
#include <cmath>
#include "rsc_utils.h"
#include "stdio.h"
#include <string>
#include <fstream>

#ifdef DEBUG
#include <iostream> // cout
#endif
using namespace std;

//if 1, causes printing internally
#define PRINT_BIN_ENERGY 1

#define p1 0.0577622650466621
#define p2 2.1011784386926213

bool FE_DEBUG_FLAG = true;

FILE *dbf = NULL;


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

int Feature_extractor::get_CENS(Audio_reader &reader, long nframes, vector<float*> &data_cens) {
	int actual_nframes, i, j, k;
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
            float tmp = 0;
            /*
#ifdef DEBUG
            printf("%f (", AREF2(chroma, i, j));
#endif
             */
			for (k = QUANT_SIZE - 1; k >= 0; k--) {
                if (AREF2(chroma, i, j) < VEC_ENERGY[k]) {
                    break;
                } else {
                    tmp += VEC_WEIGHT[k];
                    if (tmp > 1) {
                        printf("stop\n");
                    }
				}
            }
            AREF2(chroma, i, j) = tmp;
/*
#ifdef DEBUG
            cout << AREF2(chroma, i, j) << ") ";
#endif
 */
		}
        /*
#ifdef DEBUG  
        cout << AREF2(chroma, i, CHROMA_BIN_COUNT) << endl;
#endif
         */
        
	}
    
	// claculate CENS, which is the convoltion of chroma stat help
	// and hanning window, then downsampling by factor of HOP_SIZE_LONG
	//for (i = 0; i < nframes; i += HOP_SIZE_LONG) {
    for (i = 0; i < CHROMA_BIN_COUNT; i++) {
        chroma_bin = (AREF1(chroma, 0) + i);
        int conv_len = nframes + LONG_WINDOW - 1;
        chroma_conv = ALLOC(float, conv_len);
        calculate_conv(chroma_bin, nframes, hanning, (int)LONG_WINDOW, chroma_conv);
        for (j = 0; j < conv_len; j += HOP_SIZE_LONG) {
            if (i == 0) {
                float* bin_cens;
                bin_cens = ALLOC(float, CHROMA_BIN_COUNT);
                bin_cens[i] = chroma_conv[j];
                data_cens.push_back(bin_cens);
            } else {
                data_cens[j / HOP_SIZE_LONG][i] = chroma_conv[j];
            }
        }
        free(chroma_conv);
    }
    
    // normalize the CENS
    int count = data_cens.size(); 
    for (i = 0; i < count; i++) {
        norm_l2(data_cens[i]);
    }

/*
#ifdef DEBUG
    for (i = 0; i < count; i++) {
        for (j = 0; j < CHROMA_BIN_COUNT; j++) {
            cout << data_cens[i][j] << " ";
        }
        cout << endl;
    }
#endif
*/    
    free(chroma);	
	return 0;
}

void Feature_extractor::norm_l2(float *chroma_vec) {
    int i;
    float norm = 0.0f;
    for (i = 0; i < CHROMA_BIN_COUNT; i++) {
        norm += chroma_vec[i] * chroma_vec[i];
    }
    norm = sqrt(norm);
    float def_CENS = sqrt(1.0/(CHROMA_BIN_COUNT));
    for (i = 0; i < CHROMA_BIN_COUNT; i++) {
        if (norm != 0) {
            chroma_vec[i] /= norm;
        } else {
            chroma_vec[i] = def_CENS;
        }
    }
}
    


/*
 *  Compute the convolution given the start position in one bin of chroma, and the window. 
 Note the with row-wise chroma vector, the access for chroma of one pitch class is done within one column.
 Currently a n^2 imlementation, nlog(n) can be achieved with FFT (not now, FFT now doesn't take 
 non-powerof2 length array
 */
void Feature_extractor::calculate_conv(float *chroma_bin, const int len1, float *window, const int len2, float *out) {
	int i, j, k, len_conv = len1 + len2 - 1;
	float chroma_at; 
	for (i = 0; i < len_conv; i++) {
        out[i] = 0;
		for (j = 0, k = i; j < len1 && k >=0; j++, k--) {
			if (k < len2) {
				chroma_at = *(chroma_bin + j * CHROMA_BIN_COUNT);
				out[i] += window[k] * chroma_at;
			}
		}
	}
 
    /*
#ifdef DEBUG
    for (i = 0; i < len_conv; i++) {
        printf("%f ", out[i]);
    }
    printf("\n\n");
#endif
     */
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

// each row is one chroma vector, 
// data is stored as an array of chroma vectors:
// vector 1, vector 2, ...

float hz_to_step(float hz)
{
    return float((log(hz) - p2) / p1);
}


/*				PRINT_BINS
 This function is intended for debugging purposes.
 pass in an array representing the "mid point"
 of each bin, and the number of bins.  The
 function will print out:
 i value
 index falue
 low range of the bin
 middle of the bin
 high range of the bin
 */
void print_Bins(float* bins, int numBins){
    printf("BINS: \n");
    int i;
    for (i=0; i<numBins; i++) {
        int index = i % numBins;
        int indexNext = (index + 1) % numBins;
        int indexPrev = (index - 1) % numBins;
        
        float maxValue =(bins[index]+bins[indexNext])/2;
        float minValue=(bins[index]+bins[indexPrev])/2;
        
        if(index == 1)
            maxValue =bins[index]+(bins[index]-((bins[index]+bins[indexPrev])/2));
        if(index == 2)
            minValue =bins[index]-(((bins[index]+bins[indexNext])/2)-bins[index]);
        
        printf("%d (%d) %g||%g||%g\n",i,index,minValue,bins[i],maxValue);
    }		
}

/*				MIN_BIN_NUM
 Returns the index in the array of bins
 of the "smallest" bin.  aka, the bin
 whose midpoint is the smallest.
 */
int min_Bin_Num(float* bins, int numBins){
    
    int i;
    int minIndex=0;
    float minValue=bins[0];
    for (i = 0; i < numBins; i++) {   
        if (minValue > bins[i]) {
            minValue = bins[i];
            minIndex = i;
        }
    }
    return minIndex;
}

void gen_Magnitude_range(float* inR, float* inI, int low, int hi, float* out)
{
    int i;
    
    for (i = low; i < hi; i++) {
        float magVal = sqrt(inR[i] * inR[i] + inI[i] * inI[i]);
        //printf("   %d: sqrt(%g^2+%g^2)=%g\n",i,inR[i],inI[i+1],magVal);
        out[i]= magVal;
#ifdef DEBUG_LOG
        //if (i == 1000) fprintf(dbf, "gen_Magnitude: %d %g\n", i, magVal);
#endif
    }
}


// normalize a chroma vector to have
// mean of 0 and std. dev. of 1
//
static void normalize(float *cv)
{
    float sum = 0;
    
    for (int i = 0; i < CHROMA_BIN_COUNT; i++) {
        sum += cv[i];
    }
    
    if (sum == 0) {
        // avoid 0/0
        sum = 1;
    }
    /* Normalize this frame to avg. 0 */
    for (int i = 0; i < CHROMA_BIN_COUNT; i++) {
        cv[i] /= sum;
    }
    
#ifdef DEBUG_LOG
    sum = 0;
    for (int i = 0; i < CHROMA_BIN_COUNT; i++) {
        sum += cv[i];
        fprintf(dbf, "%f ", cv[i]);
        
    }
    fprintf (dbf, "\n");
    if (sum != 0){
        assert(abs(sum - 1) < 0.001);
    }
#endif
    
    /*
     No need to do these as in Muller's paper and the code they released
     */
    
    ///* Calculate std. dev. for this frame */
    //float sum = 0;
    //for (int i = 0; i < CHROMA_BIN_COUNT; i++) {
    //    float x = cv[i];
    //    sum += x * x;
    //}
    //float dev = sqrt(sum / CHROMA_BIN_COUNT);
    //if (dev == 0.0) dev = 1.0F; /* don't divide by zero */
    
    ///* Normalize this frame to std. dev. 1*/
    //for (int i = 0; i < CHROMA_BIN_COUNT; i++) cv[i] /= dev;
}


/* GEN_CHROMA_AUDIO -- compute chroma for an audio file 
 */
/*
 generates the chroma energy for a given sequence
 with a low cutoff and high cutoff.  
 The chroma energy is placed in the float *chrom_energy.
 this 2D is an array of pointers.
 The function returns the number of frames 
 (aka the length of the 1st dimention of chrom_energy)
 */
int Feature_extractor::gen_chroma(Audio_reader &reader, int hcutoff, 
                                  int lcutoff, long nframes, float **chrom_energy, double *actual_frame_period)
{
    int i;
    double sample_rate = reader.get_sample_rate();
    
    if (FE_DEBUG_FLAG) {
        printf ("==============FILE====================\n");
        reader.print_info();
    }
#ifdef DEBUG_LOG
    //fprintf(dbf, "******** BEGIN AUDIO CHROMA COMPUTATION *********\n");
#endif
    // this seems like a poor way to set actual_frame_period_0 or _1 in 
    // the Scorealign object, but I'm not sure what would be better:
    *actual_frame_period = float(reader.actual_frame_period);
    
    /*=============================================================*/
    
    // allocate some buffers for use in the loop
    int full_data_size = nextPowerOf2(reader.samples_per_frame);
    if (FE_DEBUG_FLAG) {
        printf("   samples per frame is %ld \n", reader.samples_per_frame);
        printf("   total chroma frames %ld\n", reader.frame_count); 
		printf("   the number of chroma frames to compute %ld\n", nframes);
        // printf("   Window size  %g second \n", reader.window_size);
        printf("   hopsize in samples %ld \n", reader.hop_samples);
        printf("   fft size %d\n", full_data_size);
    }
    
    float *full_data = ALLOC(float, full_data_size);
    float *fft_dataR = ALLOC(float, full_data_size);
    float *fft_dataI = ALLOC(float, full_data_size);	
    //set to zero
    memset(full_data, 0, full_data_size * sizeof(float));
    memset(fft_dataR, 0, full_data_size * sizeof(float));	
    memset(fft_dataI, 0, full_data_size * sizeof(float));
    //check to see if memory has been allocated
    assert(full_data != NULL);
    assert(fft_dataR != NULL);
    assert(fft_dataI != NULL);
    
    int *bin_map = ALLOC(int, full_data_size);
	
    //set up the chrom_energy array;
    *chrom_energy = ALLOC(float, nframes * CHROMA_BIN_COUNT);
    int cv_index = 0;
    
    // set up mapping from spectral bins to chroma bins
    // ordinarily, we would add 0.5 to round to nearest bin, but we also
    // want to subtract 0.5 because the bin has a width of +/- 0.5. These
    // two cancel out, so we can just round down and get the right answer.
    int num_bins_to_use = (int) (hcutoff * full_data_size / sample_rate);
    // But then we want to add 1 because the loops will only go to 
    // high_bin - 1:
    int high_bin = min(num_bins_to_use + 1, full_data_size);
    //printf("center freq of high bin is %g\n", (high_bin - 1) * sample_rate / 
    //    full_data_size);
    //printf("high freq of high bin is %g\n", 
    //     (high_bin - 1 + 0.5) * sample_rate / full_data_size);
    // If we add 0.5, we'll round to nearest bin center frequency, but
    // bin covers a frequency range that goes 0.5 bin width lower, so we
    // add 1 before rounding.
    int low_bin = (int) (lcutoff * full_data_size / sample_rate);
    //printf("center freq of low bin is %g\n", low_bin * sample_rate / 
    //    full_data_size);
    //printf("low freq of low bin is %g\n", (low_bin - 0.5) * sample_rate / 
    //    full_data_size);
    //printf("frequency spacing of bins is %g\n", 
    //     sample_rate / full_data_size);
    double freq = low_bin * sample_rate / full_data_size;
    for (i = low_bin; i < high_bin; i++) {
        float raw_bin = hz_to_step(float(freq));
        int round_bin = (int) (raw_bin + 0.5F);
        int mod_bin = round_bin % 12;
        bin_map[i] = mod_bin;
        freq += sample_rate / full_data_size;
    }
    // printf("BIN_COUNT is !!!!!!!!!!!!!   %d\n",CHROMA_BIN_COUNT);
    
    // create Hamming window data
    float *hamming = ALLOC(float, reader.samples_per_frame);
    gen_Hamming(hamming, reader.samples_per_frame);
    
    for (int count = 0; count < nframes; count++) {
        reader.read_window(full_data);
        //fill out array with 0's till next power of 2
#ifdef DEBUG_LOG
        //fprintf(dbf, "samples_per_frame %d sample %g\n", reader.samples_per_frame, full_data[0]);
#endif
        for (i = reader.samples_per_frame; i < full_data_size; i++) 
            full_data[i] = 0;
        
#ifdef DEBUG_LOG
        //fprintf(dbf, "preFFT: full_data[1000] %g\n", full_data[1000]);
#endif
        
        // compute the RMS, then apply the Hamming window to the data
        float rms = 0.0f;
        for (i = 0; i < reader.samples_per_frame; i++) {
            float x = full_data[i];
            rms += x * x;
            full_data[i] = x * hamming[i];
        }
        rms = sqrt(rms / reader.samples_per_frame);
        
#ifdef DEBUG_LOG   
        //fprintf(dbf, "preFFT: hammingData[1000] %g\n", full_data[1000]);
#endif
        FFT3(full_data_size, 0, full_data, NULL, fft_dataR, fft_dataI); //fft3
        
        //given the fft, compute the energy of each point
        gen_Magnitude_range(fft_dataR, fft_dataI, low_bin, high_bin, full_data);
        
        /*-------------------------------------
         GENERATE BINS AND PUT
         THE CORRECT ENERGY IN
         EACH BIN, CORRESPONDING
         TO THE CORRECT PITCH
         -------------------------------------*/
        
        float binEnergy[CHROMA_BIN_COUNT];
        int binCount[CHROMA_BIN_COUNT];
        
        for (i = 0; i < CHROMA_BIN_COUNT; i++) {
            binCount[i] = 0; 
            binEnergy[i] = 0.0;
        }
        
        for (i = low_bin; i < high_bin; i++) {
            int mod_bin = bin_map[i];
            binEnergy[mod_bin] += full_data[i];
            binCount[mod_bin]++;
        }
        
#ifdef DEBUG_LOG
        //fprintf(dbf, "cv_index %d\n", cv_index);
#endif
        assert(cv_index < nframes);
        float *cv = AREF1(*chrom_energy, cv_index);
        for (i = 0;  i < CHROMA_BIN_COUNT; i++) {
            cv[i] = binEnergy[i] / binCount[i];
        }
        //normalize 
        normalize(cv);
#ifdef DEBUG_LOG
        //fprintf(dbf, "%d@%g) ", cv_index, cv_index * reader.actual_frame_period);
        //for (int i = 0; i < CHROMA_BIN_COUNT; i++) {
        //    fprintf(dbf, "%d:%g ", i, cv[i]);
        //}
        //fprintf(dbf, " sil?:%g\n\n", cv[CHROMA_BIN_COUNT]);
#endif
        cv_index++;
    } // end of while ((readcount = read_mono_floats...
    
    free(hamming);
    free(fft_dataI);
    free(fft_dataR);
    free(full_data);
    if (FE_DEBUG_FLAG)
        printf("\nGenerated Chroma.\n");
    return cv_index;
}

