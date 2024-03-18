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
    Object3d(uint16_t xArray[8100], uint16_t yArray[8100], uint16_t zArray[8100], uint16_t focalLength);

        // Generate Projected Coordinates
    void generateProjected(void);

    uint16_t xProjected[8100]; // Array Containing xProjected Coordinates
    uint16_t yProjected[8100]; // Array Containing yProjected Coordinates

private:
        // Calculate xProjected based on Z coord
    uint16_t getXProjected(uint16_t xCoord, uint16_t zCoord); 

        // Calculate yProjected based on Z coord
    uint16_t getYProjected(uint16_t yCoord, uint16_t zCoord); 

    uint16_t* _xArray; // ptr to Array Containing X Coordinates
    uint16_t* _yArray; // ptr to Array Containing Y Coordinates
    uint16_t* _zArray; // ptr to Array Containing Z Coordinates

    uint16_t _focalLength; // Focal Length, i.e camera distance *in context of weak perspective projection
    uint16_t _cameraXangle;
    uint16_t _cameraYangle;
    
};
