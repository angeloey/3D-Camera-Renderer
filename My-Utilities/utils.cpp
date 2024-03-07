// Angelo Utilities // look at me doing fancy housekeeping
#include "utils.h"

    // Maps a value in a given range, to its corresponding value in another given range
    // Returns mapped value
int Utilities::valmap (float value, float istart, float istop, float ostart, float ostop){
    uint16_t mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

    // LCD wants colour in format 0xAARRGGBB, i.e 0xFFFFFFFF = white & 0xFF000000 = black etc.
    // Returns Hex code corresponding to given Alpha, Red, Green, & Blue Channels
unsigned long Utilities::argbToHex(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue){   
    return ((alpha & 0xFF) << 24) + ((red & 0xFF) << 16) + ((green & 0xFF) << 8)
           + (blue & 0xFF);
}
