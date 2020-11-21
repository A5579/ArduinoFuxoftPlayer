#ifndef DEFINES_H
#define DEFINES_H

#include <arduino.h>

constexpr size_t NUMBER_OF_NOTES = 80;
constexpr unsigned int SAMPLES_PER_SEC = 31373;
constexpr auto AY_3_8912_TONE_CLK = 1773400L / 16; // AY_3_8912_TONE_CLK divide by their internal fixed hw divider

#endif
