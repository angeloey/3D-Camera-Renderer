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
    Object3d(int16_t xArray[8100], int16_t yArray[8100], int16_t zArray[8100], int16_t focalLength);

        // Generate Projected Coordinates
    void generateProjected(void);

        // Modify original coordinates, rotating around an axis
    void rotateProjection(float angle, uint8_t axis); // 0 = X axis, 1 = Y axis, 2 = Z axis

    int16_t xProjected[8100]; // Array Containing xProjected Coordinates
    int16_t yProjected[8100]; // Array Containing yProjected Coordinates

private:
        // Calculate xProjected based on Z coord
    int16_t getXProjected(int16_t xCoord, int16_t zCoord); 

        // Calculate yProjected based on Z coord
    int16_t getYProjected(int16_t yCoord, int16_t zCoord); 

    int16_t* _xArray; // ptr to Array Containing X Coordinates
    int16_t* _yArray; // ptr to Array Containing Y Coordinates
    int16_t* _zArray; // ptr to Array Containing Z Coordinates

    uint16_t _focalLength; // Focal Length, i.e camera distance *in context of weak perspective projection

    const double pi = 3.14159;
    
};
