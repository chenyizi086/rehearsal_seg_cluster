#ifndef FEATURE_EXTRACTOR_H
#define FEATURE_EXTRACTOR_H

#include<vector>
#include "audioreader.h"
using namespace std;

class Audio_reader;

class Feature_extractor {
	public:
		/*	
		 * given an audio reader, get the spectrum of the next frame
		 * return 1 if this is not the last frame, return 0 otherwise
		 * the spectrum is stored in a vector of float number
		 */
		int get_spectrum(Audio_reader &reader, vector<float> &data_spec, float *frame_fb, bool verbose);
		void db_normalize(vector<float> data_in, vector<float> &data_out);
		int get_chroma_vector(Audio_reader &reader, vector<vector<float> > &data_out);
		void set_parameters(double _frame_period, double _window_size){
			frame_period = _frame_period;
			window_size = _window_size;
		};
	private:
		void gen_Hamming(float *h, int n);
		void gen_Magnitude(float* inR, float* inI, int num_bin, float* out);
		
		double frame_period;		// frame size in seconds
		double window_size;		// window size in seconds
};

#endif 
