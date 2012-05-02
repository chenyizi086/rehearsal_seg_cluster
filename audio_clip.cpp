#include "audio_clip.h"

Audio_clip::Audio_clip(string _filename) {
		filename = _filename; 
		// by default, the clip is not cluster centroid
		is_centroid = false;
}

//void Audio_clip::set_beat_map(Beat_map bmap) {
//}




