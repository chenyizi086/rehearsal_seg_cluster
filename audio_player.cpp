#include <iostream>
#include <stdio.h>
#include <sndfile.h>
#include <portaudio.h>
#include <vector>
#include <fstream> 
#include <string.h> 
#include "audio_player.h"
using namespace std;

SNDFILE * snd;
SF_INFO info;
std::vector<float> audio;
int pointer = 0;
int mode = 0;
int buffer_size = 256;
PaStream *stream;

#define MAX_BUFFER_LEN 8192
static double data [MAX_BUFFER_LEN];



static int patestCallback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    /* Cast data passed through stream to our structure. */
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    //printf("%lld %g\n", d, data[30] * 100);
    for( i=0; i< buffer_size; i++)
    {
        *out++ = audio[pointer];  /* left */
        if (info.channels == 2) {
            *out++ = audio[pointer];  /* right */
        }
        pointer += info.channels;
        if (pointer >= audio.size()){
            //tapper_stop();
            tapper_restart();
            break;
        }
    }
    return 0;
}



bool tapper_set_music(const char *fn, float start, float end){
    
    // load music
    snd = sf_open(fn, SFM_READ, &info);
    if (snd == NULL) {
        printf("Error: no file\n");
        return false;
    }
    sf_seek(snd, (int)(start * info.samplerate), SEEK_SET);
    audio.clear();
    int length = (int)((end - start) * info.samplerate);
    for (int i = 0; i < length; i += 1024) {
        sf_read_double(snd, data, 1024);
        for (int a = 0; a < 1024; ++a){
            audio.push_back(data[a]);
        }
    }
    printf("Audio File Loaded: count_frame = %lu\n", audio.size());
    sf_close(snd);
    
    return true;
}

int tapper_init_portaudio(int buffer_sz){
    buffer_size = buffer_sz;
    PaError err = Pa_Initialize();
    void * pa_data;
    err = Pa_OpenDefaultStream( &stream, 0, info.channels, paFloat32, info.samplerate, buffer_size, patestCallback, &pa_data ); 
    printf("chan.%d buffer.%d smprate.%d\n", info.channels, buffer_sz, info.samplerate);
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return err;
    }
    return err;
}


void tapper_play() {
    pointer = 0;
    PaError err = Pa_StartStream(stream);
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return;
    }
}

int tapper_get_pointer(){
    if (pointer >= audio.size()) return -1;
    return pointer;
}


void tapper_stop(){
    PaError err = Pa_StopStream( stream );
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return;
    }
    pointer = -1;
}

void tapper_restart(){
    PaError err = Pa_StopStream( stream );
    pointer = 0;
    err = Pa_StartStream(stream);
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return;
    }
    pointer = -1;
}

void tapper_goto(int g){
    pointer = g;
}

int tapper_terminate() {
    PaError err = Pa_Terminate();
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        return err;
    }
    return err;
}
    



