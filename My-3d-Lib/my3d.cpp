// Angelo Maoudis 14074479
// Angelo my3d
#include "my3d.h"
#include <cstdint>
#include <stdint.h>

Object3d::Object3d(float xArray[8100], float yArray[8100], float zArray[8100], float focalLength):_xArray(xArray), _yArray(yArray), _zArray(zArray),_focalLength(focalLength){
}

    // Omit selected axis and multiply coordinates by 2D rotation matrix
void Object3d::rotateProjection(float angle, uint8_t axis){
    angle = angle*(pi/(float)180);
    for(uint16_t i = 0; i < 8100; i++){
        switch(axis){
            case 0: // X Axis selected
                _yRotated = (float)_yArray[i]*cos(angle) - (float)_zArray[i]*sin(angle);
                _zRotated = (float)_yArray[i]*sin(angle) + (float)_zArray[i]*cos(angle);
                _yArray[i] = _yRotated;
                _zArray[i] = _zRotated;
                break;
            case 1: // Y Axis selected
                _xRotated = (float)_xArray[i]*cos(angle) + (float)_zArray[i]*sin(angle);
                _zRotated = (float)_zArray[i]*cos(angle) - (float)_xArray[i]*sin(angle);
                _xArray[i] = _xRotated;
                _zArray[i] = _zRotated;
                break;
            case 2: // Z Axis rotation
                _xRotated = (float)_xArray[i]*cos(angle) - (float)_yArray[i]*sin(angle);
                _yRotated = (float)_xArray[i]*sin(angle) + (float)_yArray[i]*cos(angle);
                _xArray[i] = _xRotated;
                _yArray[i] = _yRotated;
                break;
            default:
                // this shouldn't happen
                break;
        }
    }
}

    // Populate projected arrays with calculated coordinates
void Object3d::generateProjected(void){
    for(uint16_t i = 0; i < 8100; i++){
        xProjected[i] = getXProjected(_xArray[i], _zArray[i]);
        yProjected[i] = getYProjected(_yArray[i], _zArray[i]);
    }
}

    // Calculate xProjected for given pixel
    // "weak perspective projection"
int16_t Object3d::getXProjected(float xCoord, float zCoord){
    float adjacent = _focalLength + zCoord;
    int16_t xProjected = round((_focalLength * xCoord) / adjacent);
    return xProjected;
}

    // Calculate yProjected for given pixel
int16_t Object3d::getYProjected(float yCoord, float zCoord){
    float adjacent = _focalLength + zCoord;
    int16_t yProjected = round((_focalLength * yCoord) / adjacent);
    return yProjected;
}