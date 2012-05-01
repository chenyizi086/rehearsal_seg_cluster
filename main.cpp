#include "music_noise_classifier.h"
#include "clip_cluster.h"
#include "audio_clip.h"
#include <iostream>

Feature_extractor fe;

int main (int argc, char * const argv[]) {
    Music_noise_classifier classifier;
    Clip_cluster cluster;
    
    
     if (argc <= 1) {
        cout << "Not enough arguments" << endl;
        exit(1);
    }
     
    
    for (int i = 1; i < argc; i++) { 
        cout << argv[i] << endl;
        vector<Audio_clip> clips;
        classifier.do_music_noise_classify(argv[i], "./params/music_noise_classifier_energynorm_30.txt", "./params/eigen_music_inv_energynorm.txt", clips);
        cluster.do_cluster(clips);
    }
    return 0;
}
