// Angelo Maoudis 14074479
// Angelo Utilities // look at me doing fancy housekeeping
#pragma once

#include "cstdint"

namespace constants{
    constexpr double PI_DOUBLE = 3.14159265358979323846;   
}

class Utilities{
public:
    // Maps a value in a given range, to its corresponding value in another given range
    // Returns mapped value
    int valmap (float value, float istart, float istop, float ostart, float ostop);

    // LCD wants colour in format 0xAARRGGBB, i.e 0xFFFFFFFF = white & 0xFF000000 = black etc.
    // Returns Hex code corresponding to given Alpha, Red, Green, & Blue Channels
    unsigned long argbToHex(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue);
    
    // Returns 0 if input is negative, otherwise returns input
    float returnPositive(float input);
};
