// Angelo Maoudis 14074479
// Angelo my3d
#pragma once

#include "mbed.h"
#include <cstdint>
#include <stdint.h>

// good explanation of 3d rendering (minecraft video lol):
// https://www.youtube.com/watch?v=hFRlnNci3Rs

namespace constants{
constexpr int16_t MAX_VERTICES = 8100;   // 90x90
}

class Object3D {
public:
        // Initialize a 3d object with arrays containing x y and z coordinates, 90x90 scan
    Object3D(float focalLength);

        // Stores whatever is in the current Vertices buffer, to the VerticesSAVE struct
        // Returns true
    bool saveVertices(void);

        // Overwrites current Vertices buffer, with values in VerticesSAVE struct.
        // Returns true
    bool restoreSave(void);

        // Generate Projected Coordinates
    void generateProjected(void);

        // Modify contents of Vertices buffer, rotating around an axis
    void rotateVertices(float angle, uint8_t axis); // 0 = X axis, 1 = Y axis, 2 = Z axis

        // Arrays Containing Projected Coordinates
    float xProjected[constants::MAX_VERTICES]; 
    float yProjected[constants::MAX_VERTICES];

        // Focal Length, i.e camera distance *in context of weak perspective projection
    float focalLength;                 

        // Vertices struct: buffer & save/restore point.
    struct{                     
        float x[constants::MAX_VERTICES];  // xyz coordinates of each Vertex.
        float y[constants::MAX_VERTICES];
        float z[constants::MAX_VERTICES];
    }Vertices, VerticesSAVE;
    // General consensus online: multiple 1D arrays are faster than a multi-dimensional array, save for minimal gains in edge cases.
    // Decision: Prioritize readibility and use 1D array approach
    // Solution: Struct of 1D arrays! Readible, Encapsulated, AND Fast! 

private:
        // Calculate xProjected based on Z coord
    int16_t raycastX(float xCoord, float zCoord); 

        // Calculate yProjected based on Z coord
    int16_t raycastY(float yCoord, float zCoord); 

    float _xRotated;    // temporary buffer to store XYZ coordinates AFTER rotation
    float _yRotated;    // so that modified values are not used in the subsequent operation
    float _zRotated;    // when rotating around an axis (SEE rotateVertices in .cpp)    
};
