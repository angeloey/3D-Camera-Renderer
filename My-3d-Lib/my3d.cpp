// Angelo Maoudis 14074479
// Angelo my3d
#include "my3d.h"
#include <cstdint>
#include <stdint.h>

Object3d::Object3d(int16_t xArray[8100], int16_t yArray[8100], int16_t zArray[8100], int16_t focalLength):_xArray(xArray), _yArray(yArray), _zArray(zArray),_focalLength(focalLength){
}

    // omit selected axis and multiply coordinates by 2D rotation matrix
void Object3d::rotateProjection(float angle, uint8_t axis){
    angle = angle*(pi/180);
    for(uint16_t i = 0; i < 8100; i++){
        switch(axis){
            case 0: // X Axis selected
                _yArray[i] = _yArray[i]*cos(angle) - _zArray[i]*sin(angle);
                _zArray[i] = _yArray[i]*sin(angle) + _zArray[i]*cos(angle);
                break;
            case 1: // Y Axis selected
                _xArray[i] = _xArray[i]*cos(angle) + _zArray[i]*sin(angle);
                _zArray[i] = _zArray[i]*cos(angle) - _xArray[i]*sin(angle);
                break;
            case 2: // Z Axis rotation
                _xArray[i] = _xArray[i]*cos(angle) - _yArray[i]*sin(angle);
                _yArray[i] = _xArray[i]*sin(angle) + _yArray[i]*cos(angle);
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
int16_t Object3d::getXProjected(int16_t xCoord, int16_t zCoord){
    int16_t adjacent = _focalLength + zCoord;
    int16_t xProjected = (_focalLength * xCoord) / adjacent;
    return xProjected;
}

    // Calculate yProjected for given pixel
int16_t Object3d::getYProjected(int16_t yCoord, int16_t zCoord){
    int16_t adjacent = _focalLength + zCoord;
    int16_t yProjected = (_focalLength * yCoord) / adjacent;
    return yProjected;
}