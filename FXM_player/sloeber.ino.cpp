#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2020-11-21 20:18:00

#include "Arduino.h"
#include "Arduino.h"
#include "defines.h"
#include "music_data_FXM.h"
#include "ay_channel.h"
#include "music_state_FXM.h"

ISR(TIMER2_OVF_vect) ;
void setup() ;
void Send_data_to_AY_emul(Channel* ch_a, Channel* ch_b, Channel* ch_c, MusicState *st) ;
PROGMEM_data* play_Track_of_PL(uint8_t ind) ;
bool play_next_Track(bool infinitePlay) ;
void loop() ;

#include "FXM_player.ino"


#endif
