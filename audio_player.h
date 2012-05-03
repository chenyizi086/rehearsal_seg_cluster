#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

bool tapper_set_music(const char *fn, float start, float end);
int  tapper_init_portaudio(int buffersize);
void tapper_play();
void tapper_stop();
void tapper_restart();
void tapper_goto(int);
int  tapper_get_pointer();
void tapper_create_test();
int tapper_terminate();


#endif



