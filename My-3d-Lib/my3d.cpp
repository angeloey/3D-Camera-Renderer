// Angelo Maoudis 14074479
// Angelo my3d
#include "my3d.h"
#include <cstdint>
#include <stdint.h>

Object3d::Object3d(float focalLength):_focalLength(focalLength){
}

    // Omit selected axis and multiply Vertices coordinates by 2D rotation matrix
void Object3d::rotateVertices(float angle, uint8_t axis){                                   // X doesn't change when rotating around X axis. Same for Y & Z.
    angle = angle*(pi/(float)180);                                                          // Therefore, to rotate around an axis, Omit desired axis and multiply
    for(uint16_t i = 0; i < 8100; i++){                                                     // Vertex coordinates by 2D rotation matrix:
        switch(axis){                                                                       //             
            case 0: // X Axis selected                                                      //            [cos(theta) -sin(theta)]
                _yRotated = (float)vertices.y[i]*cos(angle) - (float)vertices.z[i]*sin(angle);    //  X [Y Z] * [sin(theta)  cos(theta)] = Xr [Yr Zr]
                _zRotated = (float)vertices.y[i]*sin(angle) + (float)vertices.z[i]*cos(angle);    //
                vertices.y[i] = _yRotated;                                                     // Analogous for X, Y, and Z. Rotating around one axis at a time.
                vertices.z[i] = _zRotated;
                break;
            case 1: // Y Axis selected
                _xRotated = (float)vertices.x[i]*cos(angle) + (float)vertices.z[i]*sin(angle);
                _zRotated = (float)vertices.z[i]*cos(angle) - (float)vertices.x[i]*sin(angle);
                vertices.x[i] = _xRotated;
                vertices.z[i] = _zRotated;
                break;
            case 2: // Z Axis rotation
                _xRotated = (float)vertices.x[i]*cos(angle) - (float)vertices.y[i]*sin(angle);
                _yRotated = (float)vertices.x[i]*sin(angle) + (float)vertices.y[i]*cos(angle);
                vertices.x[i] = _xRotated;
                vertices.y[i] = _yRotated;
                break;
            default:
                // this shouldn't happen
                break;
        }
    }
}

    // Populate projected arrays with calculated coordinates. Public. Accessable via Object3D.xProjected[i]
    // These are the XY Pixel coordinates of a 2D screen displaying the Vertices, as calculated below.
void Object3d::generateProjected(void){
    for(uint16_t i = 0; i < 8100; i++){
        xProjected[i] = raycastX(vertices.x[i], vertices.z[i]);
        yProjected[i] = raycastY(vertices.y[i], vertices.z[i]);
    }
}

    // Calculate xProjected for given pixel. 
    // xProjected = where a raycast from the camera to a Vertex would intersect the Screen on the X axis.
    // "weak perspective projection"
int16_t Object3d::raycastX(float xCoord, float zCoord){                 // Simpler process to only tackle 2 dimensions at a time. (X + Z).
    float adjacent = _focalLength + zCoord;                             // focalLengh = Distance from the Camera to the screen.
    int16_t xProjected = round((_focalLength * xCoord) / adjacent);     // adjacent = Distance from the Vertex to the camera.
    return xProjected;                                                  // Same principles apply to Y. 
}                                                                       

    // Calculate yProjected for given pixel. See above.                 //  |         X <- Vertex x-Coordinate                
int16_t Object3d::raycastY(float yCoord, float zCoord){                 //  |           *                                       
    float adjacent = _focalLength + zCoord;                             //  |             *   (Xp = xProjected)                 
    int16_t yProjected = round((_focalLength * yCoord) / adjacent);     //  | |-------------Xp------------------- <- Screen   
    return yProjected;                                                  //  | |               * <- raycast                    
}                                                                       //  | |                 *                               
                                                                        //  | | <- Focallength    C <- Camera                 
                                                                        //  ^ Adjacent                                        

                                                                        // Triangles (X, Adjacent) and (Xp, Focallength) are similar.
                                                                        // therefore: Xp / X  ==  FocalLength / Adjacent
                                                                        // thus Xp  ==  (Focallength * X) / Adjacent
                                                                        // Repeat for Y. Process is analagous.