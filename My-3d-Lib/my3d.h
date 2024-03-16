// Angelo Maoudis 14074479
// Angelo my3d
#include "mbed.h"
#include <cstdint>
#include <stdint.h>

// good explanation of 3d rendering (minecraft video lol):
// https://www.youtube.com/watch?v=hFRlnNci3Rs

class Object3d {
public:
        // Initialize a 3d object with arrays containing x y and z coordinates, 90x90 scan
    Object3d(uint16_t xArray[8100], uint16_t yArray[8100], uint16_t zArray[8100]);

    uint16_t _xArray[8100]; // Array Containing X Coordinates
    uint16_t _yArray[8100]; // Array Containing Y Coordinates
    uint16_t _zArray[8100]; // Array Containing Z Coordinates

    uint16_t _cameraDistance;
    uint16_t _cameraXangle;
    uint16_t _cameraYangle;

private:

    
};
