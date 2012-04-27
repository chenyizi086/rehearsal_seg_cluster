#include "clip_cluster.h"
#include "feature_extractor.h"
#include "gen_chroma.h"

Feature_extractor fe;
Audio_file_reader reader;

void Clip_cluster::do_cluster(vector<Audio_clip> &clips) {
	int i;
	//50% overlapping
	fe.set_parameters(SAMPLES_PER_FRAME_CHROMA / RESAMPLE_FREQ, HOP_SIZE_CHROMA / RESAMPLE_FREQ);
	for (i = 0; i < clips.size(); i++) {
		cluster(clips[i]);	
	}
}

void Clip_cluster::do_clip_cluster(Audio_clip clip) {
	char *filename;
	int start, end;
	long nframes;
	vector<vector<float> > data_cens;
	Audio_clip clip;

	clip = clips[i];
	filename = clip.get_filename();
	start = clip.get_start();
	end = clip.get_end();
	
	reader.open(filename, fe, start * SAMPLES_PER_FRAME * NUM_AVER, DEBUG_FLAG);
	nframes = (end - start + 1) * NUM_AVER;
	fe.get_CENS(reader, nframes, data_cens);


}

/*
 * return 0 indicates that query is longer than the template
 */
int Clip_cluster::dist_CENS(vector<vector<float> > query_cens, vector<vector<float> > template_cens, vector<float> dists) {
	if (query_cens.size() > template_cens.size()) {
		dist_CENS(template_cens, query_cens, dists);
		return 0;
	}

	int i, j, k, len_q = query_cens.size(), len_t = template_cens.size();
	float dot_prod, sum;
	for (i = 0; i < len_t - len_q + 1; i++) {
		sum = 0;
		for (j = 0; j < CHROMA_BIN_COUNT + 1 ; j++) {
			dot_prod = 0;
			for (k = 0; k < len_q; k++) {
				dot_prod += query_cens[k][j] * template_cens[k + i][j];
			}
			sum += dot_prod;
		}
		dists.push_back(1 - sum/len_q);
	}
	return 1;

}

int load_all_temp_cens() {
	FILE *atc = NULL;
	atc = fopen(atc_filename, "w+");
}
	
	

		

