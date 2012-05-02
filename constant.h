#ifndef CONSTANT_H
#define CONSTNAT_H

// to turn on debug flag
#define DEBUG
#define DEBUG_LOG
#define FIXED_PARAS

/*
 * Constant numbers for adaboost audio processing
 */
const int SAMPLES_PER_FRAME = 4096;
const float RESAMPLE_FREQ = 16000;	
const int NUM_AVER = 5;		// the spectrums will be averaged every 5 frames

const int DB_CENTER = 20;
const int DB_THRESHOLD = -10;  // filter out the frames less than this


/*
 * Constant numbers for CENS
 */
const int SAMPLES_PER_FRAME_CHROMA = 4096;	// 256 ms with RESAMPLE_FREQ
const int HOP_SIZE_CHROMA = 2048;           // 128 ms with RESAMPLE_FREQ
const int LONG_WINDOW = 32;					// long window size, 4096 ms with RESAMPLE_FREQ 
const int HOP_SIZE_LONG = 8;					// long window hop size, 1024 ms with RESAMPLE_FREQ

const int HIGH_CUTOFF = 2000;
const int LOW_CUTOFF = 40;

const int QUANT_SIZE = 4;
const float VEC_ENERGY[4] = {0.4, 0.2, 0.1, 0.05};
const float VEC_WEIGHT[4] = {0.25, 0.25, 0.25, 0.25};

const float DEFAULT_SILENCT_THRESHOLD = 0.1;

/*************************** Index for Adaboost parameter ************************/
const int FEATURE_INDEX = 0;
const int SIGN_INDEX = 1;
const int CLASSIFIER_INDEX = 2;
const int ALPHA_INDEX = 3;


/******************************* Parameters for HMM ******************************/
/*
 * prior for HMM
 */
const float p_m = 0.5;
const float p_n = 0.5;

/* 
 * transition prob for HMM
 */
const float pm_m = 0.99999;	// p(music|music)
const float pn_m = 0.00001;	// p(noise|music)
const float pm_n = 0.00001;	// p(music|noise)
const float pn_n = 0.99999;	// p(noise|noise)

/*
 * init prob for HMM
 */
const float p_init_m = 0.1;
const float p_init_n = 0.9;

const float alpha = 2.3;

const float MIN_LENGTH_SECS = 5.0; //secs


const float MATCHING_THRESHOLD = 0.15;
const float SEGMENT_LENGTH = 40;

#endif

