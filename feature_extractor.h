#ifndef FEATURE_EXTRACTOR_H
#define FEATURE_EXTRACTOR_H

#include<vector>
//#include<fstream>
#include "audioreader.h"
using namespace std;

FILE *dbf = NULL;

class Feature_extractor {
	public:
		Feature_extractor();
		~Feature_extractor();
		/*	
		 * given an audio reader, get the normalized spectrum of the next frame
		 * return 1 if this is not the last frame, return 0 otherwise
		 * the spectrum is stored in a vector of float number
		 */
		int get_spectrum(Audio_reader &reader, vector<float> &data_spec, float *frame_fb, bool verbose);
		void db_normalize(vector<float> &data_db);
		int get_CENS(Audio_reader &reader, long nframes, vector<vector<float> > &data_out);
		void set_parameters(double _frame_period, double _window_size){
			frame_period = _frame_period;
			window_size = _window_size;
		};
	private:
		int gen_chroma(Audio_reader &reader, int hcutoff, int lcutoff, long nframes, float **chrom_energy, double *actual_frame_period);
		void calculate_conv(float *chroma, int len1, float *window, int len2, float *out);
		void gen_Hamming(float *h, int n);
		void gen_Hanning(float *h, int n);
		void gen_Magnitude(float* inR, float* inI, int num_bin, float* out);
		
		double frame_period;		// frame size in seconds
		double window_size;		// window size in seconds
};

#endif 
