/*

* Thanks to Anton Dmitrievsky for ay_channel.cpp code and Idea
*
* the changes I made to ay_channel.cpp are wet and need correction
*
*/


#include <math.h>
#include "ay_channel.h"

//FixedPoint Channel::tone_period_table[NUMBER_OF_NOTES];
FixedPoint Channel::noise_period_table[32];

uint8_t Channel::volumes[] = {0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 6, 7, 9, 11, 13, 15};
//uint8_t Channel::volumes[] = {0, 0, 1, 1, 2, 2, 2, 4, 6, 8, 12, 14, 18, 22, 26, 30};

//void Channel::init_tone_period_table() {
//    for (size_t n = 0; n < NUMBER_OF_NOTES; n++) {
//        const auto frequency = 32.703125 * pow(2, double(n) / 12);
//        tone_period_table[n].raw =   lround((FIXED_INTEGER_SIZE * frequency / SAMPLES_PER_SEC) * 256 );
//    }
//}

void Channel::init_noise_period_table() {
    for (size_t n = 0; n < 32; n++) {
        const auto frequency = 31373.0 / (n+1);
        noise_period_table[n].raw =   lround((FIXED_INTEGER_SIZE * frequency / SAMPLES_PER_SEC) * 256 );
    }
}

uint16_t xs = 1;

uint16_t xor_shift() {
    xs ^= xs << 7u;
    xs ^= xs >> 9u;
    xs ^= xs << 8u;
    return xs;
}


void Channel::init_tone(uint16_t tone) {
	// period.raw = pgm_read_word_near(tone_period_table_prg+tone);
	// or real time calculate
	// period.raw = AY_3_8912_TONE_CLK / (tone & 0xfff) / SAMPLES_PER_SEC * 256 * 256;   //// 16 - is AY tone divider
	if (tone<3)
		period.raw =65535;
	else
	{
		constexpr float n = 256 * AY_3_8912_TONE_CLK / SAMPLES_PER_SEC * 256;
		period.raw = (uint16_t) (n  / (tone & 0xfff));
	}
}


void Channel::init_noise(uint8_t noise) {
    noise_period.raw = noise_period_table[noise].raw;
}

void Channel::set_volume(uint8_t _volume) {
    volume = volumes[_volume];
}

uint8_t Channel::get_volume() {
    return is_tone_enabled ? volume : 0;
}

void Channel::enable_tone(bool _is_tone_enabled) {
    is_tone_enabled = _is_tone_enabled;
}

void Channel::enable_noise(bool _is_noise_enabled) {
    is_noise_enabled = _is_noise_enabled;
}

bool Channel::get_is_noise_enabled() {
    return is_noise_enabled;
}

int8_t Channel::render() {
	int8_t ret;
	static int16_t noise;
	wave_position.raw += period.raw;
	noise_position.raw += noise_period.raw;

	if (noise_position.parts.integer > (FIXED_INTEGER_SIZE / 2))
	{
		noise = xor_shift();
		noise_position.raw = 0;
	}

	ret  = (is_tone_enabled ? (wave_position.parts.integer < (FIXED_INTEGER_SIZE / 2) ? (int8_t) volume : -(int8_t) volume ) : 0);
	ret += (is_noise_enabled ? (volume * (noise&1 )) & 0x1f : 0);
	return  ret;
}

