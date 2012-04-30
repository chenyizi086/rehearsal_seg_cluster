#include "music_noise_classifier.h"
#include "clip_cluster.h"
#include "audio_clip.h"

int main (int argc, char * const argv[]) {
    Music_noise_classifier classifier;
    Clip_cluster cluster;
    
    for (int i = 0; i < argc; i++) {
        vector<Audio_clip> clips;
        classifier.do_music_noise_classify(argv[i], clips);
        cluster.do_cluster(clips);
    }
    return 0;
}
