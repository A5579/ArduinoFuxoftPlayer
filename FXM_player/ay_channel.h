#ifndef AY_CHANNEL_H
#define AY_CHANNEL_H

#include "defines.h"



const PROGMEM uint16_t *get_ay_t(void);


constexpr size_t FIXED_INTEGER_SIZE = 256;
union FixedPoint {
    struct {
        uint8_t fraction;
        uint8_t integer;
    } parts;
    uint16_t raw = 0;
};

class Channel {
private:
    FixedPoint period;
    FixedPoint wave_position;

    FixedPoint noise_period;
    FixedPoint noise_position;

    uint8_t volume = 0;
    bool is_tone_enabled = false;
    bool is_noise_enabled = false;

    static FixedPoint tone_period_table[NUMBER_OF_NOTES];

    static uint8_t volumes[];

public:
    static FixedPoint noise_period_table[32];
    static void init_tone_period_table();
    static void init_noise_period_table();

    void init_tone(uint16_t tone);
    void init_noise(uint8_t noise);

    void set_volume(uint8_t _volume);
    uint8_t get_volume();
    bool get_is_noise_enabled();

    void enable_tone(bool _is_tone_enabled);

    void enable_noise(bool _is_noise_enabled);

    int8_t render();


};


#endif
