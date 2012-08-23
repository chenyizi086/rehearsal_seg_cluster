#include "music_noise_classifier.h"
#include "clip_cluster.h"
#include "audio_clip.h"
#include "audio_player.h"
#include "constant.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include <ctime>

Feature_extractor fe;

int main (int argc, char * const argv[]) {
    void print_usage();
    if (argc <= 1) {
        cout << "Not enough arguments." << endl;
        print_usage();
        exit(1);
    }
    
#ifdef TIME_MEAS
    clock_t start, end; 
#endif
    Music_noise_classifier classifier("./params/music_noise_classifier_energynorm_30.txt", "./params/eigen_music_inv_energynorm.txt");
    Clip_cluster cluster;
   
    for (int i = 1; i < argc; i++) { 
        cout << argv[i] << endl;
        vector<Audio_clip *> clips;
#ifdef TIME_MEAS
        start = clock();
#endif
        classifier.do_music_noise_classify(argv[i], clips);
#ifdef TIME_MEAS
        end = clock();
        cout << "Running Time for segmentation : " << (double) (end - start) / CLOCKS_PER_SEC << endl;
        start = clock();
#endif
        cluster.do_cluster(clips);
#ifdef TIME_MEAS
        end = clock();
        cout << "Running Time for clustering : " << (double) (end - start) / CLOCKS_PER_SEC << endl;
#endif
    }
    return 0;
}

void print_usage() {
    cout << "Usage:" << endl;
    cout << "\trehearsal-seg-cluster audio_clip1 audio_clip2..." << endl;
}
    
