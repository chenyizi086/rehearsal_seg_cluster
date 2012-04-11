/* audiofilereader.cpp -- implements a class to read samples
 *
 * 14-Jun-08  RBD
 * 16-Jun-08  RBD revised to use libsndfile
 */
#include "assert.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "sndfile.h"
#include "samplerate.h"
#include "audioreader.h"
#include "audiofilereader.h"

#ifdef WIN32
#include <malloc.h>
#define bzero(addr, siz) memset(addr, 0, siz)
#define alloca _alloca
#endif

#define DEBUG

double Audio_file_reader::get_sample_rate()
{
    return sf_info.samplerate;
}

double Audio_file_reader::get_sample_rate_rs()
{
	if (sf_rs != NULL)
		return sf_info_rs.samplerate;
	else 
		return -1;
}

long Audio_file_reader::get_frames()
{
    return total_frames;
}

long Audio_file_reader::get_frames_rs()
{
	if (sf_rs != NULL)
		return total_frames_rs;
	else
		return -1;
} 

long Audio_file_reader::read(float *data, long n)
{
    // note that "samples_per_frame" is really "frames_per_window" in this
    // context, so we're computing bytes per window
    float *input_data = (float *) alloca(bytes_per_frame * samples_per_frame);
    assert(input_data != NULL) ;
	
    long frames_read = (long) sf_readf_float(sf, input_data, n);
    long chans = sf_info.channels;
    // now convert to mono and move to data
    for (int frame = 0; frame < frames_read; frame++) {
        float sum = 0;
        for (int chan = 0; chan < sf_info.channels; chan++) {
            // sum over channels within a frame
            sum += input_data[frame * chans + chan];
        }
        // write the frame sum to result array
        data[frame] = sum;
    }
    return frames_read;
}


bool Audio_file_reader::open(const char *filename, Feature_extractor &fe, bool verbose)
{
    bytes_per_frame = 0; // initialize now in case an error occurs
    name[0] = 0;
    bzero(&sf_info, sizeof(sf_info));
    sf = sf_open(filename, SFM_READ, &sf_info);
    if (!sf) 
		return false;
    strncpy(name, filename, MAX_NAME_LEN);
    name[MAX_NAME_LEN] = 0; // just in case
    total_frames = (long) sf_seek(sf, 0, SEEK_END);
    sf_seek(sf, 0, SEEK_SET);
    // we're going to read floats, but they might be multi-channel...
    bytes_per_frame = sf_info.channels * sizeof(float);
    calculate_parameters(fe, verbose);
    return true;
}

void Audio_file_reader::resample(int new_sample_rate) {
	sf_count_t	count;	
	double gain = 1.0;
	int samplerate = sf_info.samplerate;

    bzero(&sf_info_rs, sizeof(sf_info_rs));
	
	if (samplerate <= rsamplerate) {
		printf("WARNING: The original file has sampling frequency %d less than the resampled frequency %d\n", samplerate, new_sample_rate);
	}

	int	max_speed = SF_FALSE ;
	int converter = DEFAULT_CONVERTER ;

#ifdef DEBUG
	printf ("Input File    : %s\n", sf_info.name) ;
	printf ("Sample Rate   : %d\n", sf_info.samplerate) ;
	printf ("Input Frames  : %ld\n\n", (long) sf_info.frames) ;
#endif

	src_ratio = (1.0 * new_sample_rate) / samplerate ;
	sf_info_rs.samplerate = new_sample_rate ;

	if (fabs (src_ratio - 1.0) < 1e-20)
	{	
		printf ("Target samplerate and input samplerate are the same. Exiting.\n") ;
		return
	};


#ifdef DEBUG
	printf ("SRC Ratio     : %f\n", src_ratio) ;
	printf ("Converter     : %s\n\n", src_get_name (converter)) ;
#endif

	if (src_is_valid_ratio (src_ratio) == 0)
	{	
		printf ("Error : Sample rate change out of valid range.\n") ;
		return
	};

#ifdef DEBUG
	printf ("Output file   : %s\n", sf_info.name);
	printf ("Sample Rate   : %d\n", sf_info_rs.samplerate) ;
#endif

	do
	{	
		if (max_speed)
		{	/* This is mainly for the comparison program tests/src-evaluate.c */
			sf_command (sf_rs, SFC_SET_ADD_PEAK_CHUNK, NULL, SF_FALSE) ;
			}
		else
		{	/* Update the file header after every write. */
			sf_command (sf_rs, SFC_SET_UPDATE_HEADER_AUTO, NULL, SF_TRUE) ;
			} ;

		sf_command (sf_rs, SFC_SET_CLIPPING, NULL, SF_TRUE) ;

		count = sample_rate_convert (sf, sf_rs, converter, src_ratio, sf_info.channels, &gain) ;
		}
	while (count < 0) ;

#ifdef DEBUG
	printf ("Output Frames : %ld\n\n", (long) count) ;
#endif

	return
} /* main */

/*==============================================================================
*/

sf_count_t sample_rate_convert (SNDFILE *infile, SNDFILE *outfile, int converter, double src_ratio, int channels, double * gain)
{	static float input [BUFFER_LEN] ;
	static float output [BUFFER_LEN] ;

	SRC_STATE	*src_state ;
	SRC_DATA	src_data ;
	int			error ;
	double		max = 0.0 ;
	sf_count_t	output_count = 0 ;

	sf_seek (infile, 0, SEEK_SET) ;
	sf_seek (outfile, 0, SEEK_SET) ;

	/* Initialize the sample rate converter. */
	if ((src_state = src_new (converter, channels, &error)) == NULL)
	{	printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 0 ; /* Set this later. */

	/* Start with zero to force load in while loop. */
	src_data.input_frames = 0 ;
	src_data.data_in = input ;

	src_data.src_ratio = src_ratio ;

	src_data.data_out = output ;
	src_data.output_frames = BUFFER_LEN /channels ;

	while (1)
	{
		/* If the input buffer is empty, refill it. */
		if (src_data.input_frames == 0)
		{	
			src_data.input_frames = sf_readf_float (infile, input, BUFFER_LEN / channels) ;
			src_data.data_in = input ;

			/* The last read will not be a full buffer, so snd_of_input. */
			if (src_data.input_frames < BUFFER_LEN / channels)
				src_data.end_of_input = SF_TRUE ;
		};

		if ((error = src_process (src_state, &src_data)))
		{	
			printf ("\nError : %s\n", src_strerror (error)) ;
			exit (1) ;
		} ;

		/* Terminate if done. */
		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break ;

		max = apply_gain (src_data.data_out, src_data.output_frames_gen, channels, max, *gain) ;

		/* Write output. */
		sf_writef_float (outfile, output, src_data.output_frames_gen) ;
		output_count += src_data.output_frames_gen ;

		src_data.data_in += src_data.input_frames_used * channels ;
		src_data.input_frames -= src_data.input_frames_used ;
		} ;

	src_state = src_delete (src_state) ;

	if (max > 1.0)
	{	
		*gain = 1.0 / max ;
		printf ("\nOutput has clipped. Restarting conversion to prevent clipping.\n\n") ;
		return -1 ;
		} ;

	return output_count ;
} /* sample_rate_convert */

double apply_gain (float * data, long frames, int channels, double max, double gain)
{
	long k ;

	for (k = 0 ; k < frames * channels ; k++)
	{	data [k] *= gain ;

		if (fabs (data [k]) > max)
			max = fabs (data [k]) ;
		} ;

	return max ;
} /* apply_gain */

void Audio_file_reader::close()
{
    sf_close(sf);
	sf_close(sf_rs);
}

void Audio_file_reader::print_info()
{
	printf("***original file***\n");
    printf("   file name = %s\n", name);
    double sample_rate = sf_info.samplerate;
    printf("   sample rate = %g\n", sample_rate);
    printf("   channels = %d\n", sf_info.channels);
    /*=============================================================*/
    printf("   total frames number is = %ld\n", total_frames);
    printf("   audio duration = %g seconds\n", total_frames / sample_rate);
    /*=============================================================*/

	if (sf_rs != NULL) {
		printf("***resampled file***\n");
		double sample_rate_rs = sf_info_rs.samplerate;
		printf("   sample rate = %g\n", sample_rate_rs);
		printf("   channels = %d\n", sf_info_rs.channels);
	    /*=============================================================*/
	    printf("   total frames number is = %ld\n", total_frames_rs);
	    printf("   audio duration = %g seconds\n", total_frames_rs / sample_rate_rs);
	}
}

