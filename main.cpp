#include "music_noise_classifier.h"
#include "clip_cluster.h"
#include "audio_clip.h"
#include "audio_player.h"
#include "constant.h"
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include "portaudio.h"
#include <ctime>

Feature_extractor fe;

int MODE;

int SONG;

int START[2] = {0, 0};
int END[2] = {218, 157};

string FILENAMES[2] = {"chinese_pop1.wav", "flute3.wav"};

int main (int argc, char * const argv[]) {
    MODE = atoi(argv[1]);
    cout << "Mode " << MODE << endl;
    if (MODE == 0) {
        clock_t start, end; 
        Music_noise_classifier classifier("./params/music_noise_classifier_energynorm_30.txt", "./params/eigen_music_inv_energynorm.txt");
        Clip_cluster cluster;
        if (argc <= 1) {
            cout << "Not enough arguments" << endl;
            exit(1);
        }
        for (int i = 2; i < argc; i++) { 
            cout << argv[2] << endl;
            vector<Audio_clip *> clips;
            start = clock();
            classifier.do_music_noise_classify(argv[i], clips);
            end = clock();
            cout << "Running Time for segmentation : " << (double) (end - start) / CLOCKS_PER_SEC << endl;
            start = clock();
            cluster.do_cluster(clips);
            end = clock();
            cout << "Running Time for clustering : " << (double) (end - start) / CLOCKS_PER_SEC << endl;

        }
        return 0;
    } else if (MODE == 1) {
        Clip_cluster cluster;
        SONG = atoi(argv[2]);
        
        //  Prepare our context and socket
        zmq::context_t context (1);
        zmq::socket_t socket (context, ZMQ_REP);
        socket.bind ("tcp://*:5555");
        
        while (true) {
            zmq::message_t request;
            
            //  Wait for next request from client
            socket.recv (&request);
            std::cout << "Received something" << std::endl;
            
            //  Send reply back to client and start playing
            zmq::message_t reply (5);
            memcpy ((void *) reply.data (), "Start", 5);
            socket.send (reply);
            break;
        }
        
        float start = START[SONG] * SAMPLES_PER_FRAME / RESAMPLE_FREQ * NUM_AVER;
        float end = END[SONG] * SAMPLES_PER_FRAME / RESAMPLE_FREQ * NUM_AVER;
        cout << start << endl;
        cout << end << endl;
        tapper_set_music(FILENAMES[SONG].c_str(), start, end);
        tapper_init_portaudio(256);
        tapper_play();
        while (true) {
            Pa_Sleep(1000);
            printf("%d\n", tapper_get_pointer());
            if (tapper_get_pointer() == -1) {
                break;
            }
        }   
        return 0;
    } else {
        cout << "Wrong mode number, either 0 or 1" << endl;
        exit(1);
    }    
}
