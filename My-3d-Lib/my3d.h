// Angelo Maoudis 14074479
// Angelo my3d
#include "mbed.h"
#include <cstdint>
#include <stdint.h>

// good explanation of 3d rendering (minecraft video lol):
// https://www.youtube.com/watch?v=hFRlnNci3Rs

#define MAX_VERTICES 8100   // 90x90

class Object3d {
public:
        // Initialize a 3d object with arrays containing x y and z coordinates, 90x90 scan
    Object3d(float focalLength);

        // Generate Projected Coordinates
    void generateProjected(void);

        // Modify original coordinates, rotating around an axis
    void rotateVertices(float angle, uint8_t axis); // 0 = X axis, 1 = Y axis, 2 = Z axis

    float xProjected[8100];     // Array Containing xProjected Coordinates
    float yProjected[8100];     // Array Containing yProjected Coordinates

    float _focalLength;         // Focal Length, i.e camera distance *in context of weak perspective projection

    struct{                     // Struct xyz coordinates of each Vertex.
        float x[MAX_VERTICES];  // Also used to store a copy, to restore.
        float y[MAX_VERTICES];
        float z[MAX_VERTICES];
    }vertices, verticesSAVE;
    // General consensus online: multiple 1D arrays are faster than a multi-dimensional array, save for minimal gains in edge cases.
    // Decision: Prioritize readibility and use 1D array approach
    // Solution: Struct of 1D arrays! Readible, Encapsulated, AND Fast! 

private:
        // Calculate xProjected based on Z coord
    int16_t raycastX(float xCoord, float zCoord); 

        // Calculate yProjected based on Z coord
    int16_t raycastY(float yCoord, float zCoord); 

    float _xRotated; // used to temporarily store XYZ coordinates AFTER rotation
    float _yRotated; // so that modified values are not used in the subsequent operation
    float _zRotated; // when rotating around an axis (SEE rotateProjection in .cpp)

    const double pi = 3.14159265359;
    
};
