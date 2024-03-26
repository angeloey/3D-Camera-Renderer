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
    Object3d(float xArray[8100], float yArray[8100], float zArray[8100], float focalLength);

        // Generate Projected Coordinates
    void generateProjected(void);

        // Modify original coordinates, rotating around an axis
    void rotateProjection(float angle, uint8_t axis); // 0 = X axis, 1 = Y axis, 2 = Z axis

    float xProjected[8100]; // Array Containing xProjected Coordinates
    float yProjected[8100]; // Array Containing yProjected Coordinates

    float _focalLength; // Focal Length, i.e camera distance *in context of weak perspective projection

private:
        // Calculate xProjected based on Z coord
    int16_t getXProjected(float xCoord, float zCoord); 

        // Calculate yProjected based on Z coord
    int16_t getYProjected(float yCoord, float zCoord); 

    float* _xArray; // ptr to Array Containing X Coordinates
    float* _yArray; // ptr to Array Containing Y Coordinates
    float* _zArray; // ptr to Array Containing Z Coordinates

    float _xRotated; // used to temporarily store XYZ coordinates AFTER rotation
    float _yRotated; // so that modified values are not used in the subsequent operation
    float _zRotated; // when rotating around an axis (SEE rotateProjection in .cpp)

    const double pi = 3.14159265359;
    
};
