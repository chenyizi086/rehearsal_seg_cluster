#include "audio_clip.h"

Audio_clip::Audio_clip(const char* _filename, int _start, int _end) {
		filename = _filename; 
		start = _start; 
		end = _end;
		// by default, the clip is not cluster centroid
		is_centroid = false;
}

void Audio_clip::set_beat_map(Beat_map bmap) {
}
