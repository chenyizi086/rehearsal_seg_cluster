#ifndef CONSTANT_H
#define CONSTNAT_H

// to turn on debug flag
#define DEBUG

bool DEBUG_FLAG = true;

/*
 * Constant numbers for adaboost audio processing
 */
const int SAMPLES_PER_FRAME = 4096;
const int RESAMPLE_FREQ = 16000;	
const int NUM_AVER = 5;		// the spectrums will be averaged every 5 frames

const int DB_CENTER = 20;
const int DB_THRESHOLD = -10;  // filter out the frames less than this

/*************************** Index for Adaboost parameter ************************/
const int FEATURE_INDEX = 0;
const int SIGN_INDEX = 1;
const int CLASSIFIER_INDEX = 2;
const int ALPHA_INDEX = 3;


/******************************* Parameters for HMM ******************************/
/*
 * prior for HMM
 */
const float p_m = 0.5
const float p_n = 0.5 

/* 
 * transition prob for HMM
 */
const float pm_m = 0.95		// p(music|music)
const float pn_m = 0.05		// p(noise|music)
const float pm_n = 0.05		// p(music|noise)
const float pn_n = 0.95		// p(noise|noise)

/*
 * init prob for HMM
 */
const p_init_m = 0.1
const p_init_n = 0.9

const alpha = 2.3;


const float MIN_LENGTH_SECS = 5.0 //secs

#endif

