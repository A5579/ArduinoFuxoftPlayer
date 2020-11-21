/*
 *
 * 			ZX Specrum  AY-3-8910(12) Sound player
 *
 * Play Fuxoft Sound tracks on stand alone Arduino
 * Need connect only some speaker or amplifier line in is better on D3 pin (via capacitor?)
 * and you can hear...
 *
 * Yon can connect key button to D5 pin and GND that will
 * short press - next
 * long press - 7 sec. preview every track of playlist
 *
 * Also terminal out present 115200baud
 *
 *
 * Playlist, and data tracks are in music_data_FXM.cpp file
 *
 *
 * Thanks Frantisek Fuka - FUXOFT.CZ for all sound tracks and fxm format description
 *
 * Thanks to Bulba Sergey Vladimirovich for Source code of AY Emulator
 *
 * Thanks to Anton Dmitrievsky for ay_channel.cpp code and Idea
 *
 * the changes I made to ay_channel.cpp are wet and need correction
 * since there are inaccuracies in sound reproduction, sorry
 *
 *
 * Naumenko Aleksandr Vladimirovich
 *
*/


#include "Arduino.h"
#include "defines.h"
#include "music_data_FXM.h"
#include "ay_channel.h"
#include "music_state_FXM.h"

Channel ch_a, ch_b, ch_c;
MusicState state;

static uint8_t current_track_index;



const int next_buttonPin = 5;   // the number of the pushbutton pin
								// short press - next, and set normal mode
								// long press (4s) enter overview mode {7s play for each track from playlist}


bool overview_mode;


ISR(TIMER2_OVF_vect) {
	static uint8_t tick = 0;
	// will be called 31373 times per second
	OCR2B = (uint8_t)(ch_a.render() + ch_b.render() + ch_c.render()) + 128;

	digitalWrite(LED_BUILTIN, ch_a.get_volume() > (tick & 0x0f));
	tick ++;
}



void setup() {
	pinMode(3, OUTPUT); // Will use OC2B, digital pin #3, PD3
	// initialize the pushbutton pin as an input:
	pinMode(next_buttonPin, INPUT_PULLUP);


	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM00); // set phase correct PWM mode
	TCCR2A |= (1 << COM2B1); // clear OC2B on compare match when up-counting. Set OC2B on compare match when down-counting.
	TCCR2B |= (1 << CS00); // set pwm frequency to 31373 Hz

	TIMSK2 |= (1 << TOIE2); // enable overflow interrupts




	//  Channel::init_tone_period_table();
	Channel::init_noise_period_table();

	Serial.begin(115200);

	current_track_index =0;
	play_Track_of_PL(current_track_index);

	state.voice1.indexCH= 1;		// for debug code
	state.voice2.indexCH= 2;
	state.voice3.indexCH= 3;

	overview_mode  = false;
}



void Send_data_to_AY_emul(Channel* ch_a, Channel* ch_b, Channel* ch_c, MusicState *st)
{
	ch_a->init_tone((st->voice1.Ton)&0xfff);
	ch_a->set_volume(st->voice1.Amplitude);
	ch_a->enable_tone(st->voice1.FXM_Mixer&8);
	ch_a->enable_noise((st->voice1.FXM_Mixer&1) );
	ch_a->init_noise(st->Noise_Base& 0x1f);

	ch_b->init_tone((st->voice2.Ton)&0xfff);
	ch_b->set_volume(st->voice2.Amplitude);
	ch_b->enable_tone(st->voice2.FXM_Mixer&8);
	ch_b->enable_noise(st->voice2.FXM_Mixer&1 );
	ch_b->init_noise(st->Noise_Base& 0xff);

	ch_c->init_tone((st->voice3.Ton)&0xfff);
	ch_c->set_volume(st->voice3.Amplitude);
	ch_c->enable_tone(st->voice3.FXM_Mixer&8);
	ch_c->enable_noise(st->voice3.FXM_Mixer&1 );
	ch_c->init_noise(st->Noise_Base& 0xff);
}


PROGMEM_data* play_Track_of_PL(uint8_t ind)
{
	PROGMEM_data *track;

	track = get_sound_track(ind);
	if (track)
	{
		state.set_data(track);
		state.init_music();

		Serial.print("Play track # ");
		Serial.print(ind + 1, DEC);
		Serial.print(" of ");
		Serial.println(get_playlist_size(), DEC);
	}
	return track;
}



bool play_next_Track(bool infinitePlay)
{
	current_track_index++;
	if (current_track_index > (get_playlist_size()-1))
	{
		if (infinitePlay)
		{
			current_track_index = 0;
			Serial.println("Jump to start of playlist...");
		}
		else
			return false;
	}
	play_Track_of_PL(current_track_index);
	return true;
}



void loop() {
	static uint32_t last_managed = 0;
	static uint32_t delta_t;
	static uint32_t last_managed_overview = 0;
	static uint8_t next_button_pressed_time = 0; // in 20 ms
	do
	{
		delta_t = millis() - last_managed;

		if (delta_t >= 20 )
		{
			last_managed = millis();

			if (state.play20ms())
				Send_data_to_AY_emul(&ch_a, &ch_b, &ch_c, &state);
			else	// end of track
			{
				Serial.println("End of track, play next...");
				if (!play_next_Track(overview_mode))
					break;

			}
			//	Serial.print("frame time: "); Serial.print(millis() - last_managed, DEC); Serial.println("ms");	// debug line

			if (ch_c.get_is_noise_enabled())
				UDR0 = 0; // it's not necessary to wait until transmission finishes, we only wanna flash the led

			// overview move
			if ( overview_mode && (last_managed - last_managed_overview > 7000) )
			{
				last_managed_overview = last_managed;
				Serial.println("Jump to next track...");
				play_next_Track(overview_mode);
			}

			// key button routine
			if (digitalRead(next_buttonPin) == LOW)
			{
				if (next_button_pressed_time < 255)
					next_button_pressed_time++;
			}
			else // release button
			{
				if ((next_button_pressed_time>5) && (next_button_pressed_time < 3*50) )		// release after short press
				{
					overview_mode = false;

					Serial.println("next key short press! ");
					if (!play_next_Track(overview_mode))
						break;

				}
				next_button_pressed_time = 0;
			}
			if (next_button_pressed_time == 4*50)	// long press become
			{
				overview_mode = true;
				last_managed_overview = last_managed;
				Serial.println("next key long press - enter overview mode...");
				play_next_Track(overview_mode);

			}
		}
	}
	while (true);
	Serial.println("it's must be End of play list");
	Serial.println("This is THE END. at last");
}
