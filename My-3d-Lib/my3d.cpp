// Angelo Maoudis 14074479
// Angelo my3d
#pragma once

#include "myUtils.h"
#include "my3d.h"
#include <cstdint>
#include <stdint.h>

Object3D::Object3D(float focalLength):focalLength(focalLength){
}

    // Stores whatever is in the current Vertices buffer, to the VerticesSAVE struct
    // Returns true
bool Object3D::saveVertices(void){
    VerticesSAVE = Vertices;
    return true;
}

    // Overwrites current Vertices buffer, with values in VerticesSAVE struct.
    // Returns true
bool Object3D::restoreSave(void){
    Vertices = VerticesSAVE;
    return true;
}

    // Omit selected axis and multiply Vertices coordinates by 2D rotation matrix
void Object3D::rotateVertices(float angle, uint8_t axis){                                          // X doesn't change when rotating around X axis. Same for Y & Z.
    angle = angle*(constants::PI_DOUBLE/(float)180);                                               // Therefore, to rotate around an axis, Omit desired axis and multiply
    for(uint16_t i = 0; i < 8100; i++){                                                            // Vertex coordinates by 2D rotation matrix:
        switch(axis){                                                                              //             
            case 0: // X Axis selected                                                             //            [cos(theta) -sin(theta)]
                _yRotated = (float)Vertices.y[i]*cos(angle) - (float)Vertices.z[i]*sin(angle);     //  X [Y Z] * [sin(theta)  cos(theta)] = Xr [Yr Zr]
                _zRotated = (float)Vertices.y[i]*sin(angle) + (float)Vertices.z[i]*cos(angle);     //
                Vertices.y[i] = _yRotated;                                                         // Analogous for X, Y, and Z. Rotating around one axis at a time.
                Vertices.z[i] = _zRotated;
                break;
            case 1: // Y Axis selected
                _xRotated = (float)Vertices.x[i]*cos(angle) + (float)Vertices.z[i]*sin(angle);
                _zRotated = (float)Vertices.z[i]*cos(angle) - (float)Vertices.x[i]*sin(angle);
                Vertices.x[i] = _xRotated;
                Vertices.z[i] = _zRotated;
                break;
            case 2: // Z Axis rotation
                _xRotated = (float)Vertices.x[i]*cos(angle) - (float)Vertices.y[i]*sin(angle);
                _yRotated = (float)Vertices.x[i]*sin(angle) + (float)Vertices.y[i]*cos(angle);
                Vertices.x[i] = _xRotated;
                Vertices.y[i] = _yRotated;
                break;
            default:
                // this shouldn't happen
                break;
        }
    }
}

    // Populate projected arrays with calculated coordinates. Public. Accessable via Object3D.xProjected[i]
    // These are the XY Pixel coordinates of a 2D screen displaying the Vertices, as calculated below.
void Object3D::generateProjected(void){
    for(uint16_t i = 0; i < 8100; i++){
        xProjected[i] = raycastX(Vertices.x[i], Vertices.z[i]);
        yProjected[i] = raycastY(Vertices.y[i], Vertices.z[i]);
    }
}

    // Calculate xProjected for given pixel. 
    // xProjected = where a raycast from the camera to a Vertex would intersect the Screen on the X axis.
    // "weak perspective projection"
int16_t Object3D::raycastX(float xCoord, float zCoord){                 // Simpler process to only tackle 2 dimensions at a time. (X + Z).
    float adjacent = focalLength + zCoord;                              // focalLengh = Distance from the Camera to the screen.
    int16_t xProjected = round((focalLength * xCoord) / adjacent);      // adjacent = Distance from the Vertex to the camera.
    return xProjected;                                                  // Same principles apply to Y. 
}                                                                       

    // Calculate yProjected for given pixel. See above.                 //  |         X <- Vertex x-Coordinate                
int16_t Object3D::raycastY(float yCoord, float zCoord){                 //  |           *                                       
    float adjacent = focalLength + zCoord;                              //  |             *   (Xp = xProjected)                 
    int16_t yProjected = round((focalLength * yCoord) / adjacent);      //  | |-------------Xp------------------- <- Screen   
    return yProjected;                                                  //  | |               * <- raycast                    
}                                                                       //  | |                 *                               
                                                                        //  | | <- Focallength    C <- Camera                 
                                                                        //  ^ Adjacent                                        

                                                                        // Triangles (X, Adjacent) and (Xp, Focallength) are similar.
                                                                        // therefore: Xp / X  ==  FocalLength / Adjacent
                                                                        // thus Xp  ==  (Focallength * X) / Adjacent
                                                                        // Repeat for Y. Process is analagous.