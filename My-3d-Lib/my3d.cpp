// Angelo Maoudis 14074479
// Angelo my3d
#include "my3d.h"
#include <cstdint>
#include <stdint.h>

Object3d::Object3d(uint16_t xArray[8100], uint16_t yArray[8100], uint16_t zArray[8100], uint16_t focalLength):_xArray(xArray), _yArray(yArray), _zArray(zArray),_focalLength(focalLength){
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
uint16_t Object3d::getXProjected(uint16_t xCoord, uint16_t zCoord){
    uint16_t adjacent = _focalLength + zCoord;
    uint16_t xProjected = (_focalLength * xCoord) / adjacent;
    return xProjected;
}

    // Calculate yProjected for given pixel
uint16_t Object3d::getYProjected(uint16_t yCoord, uint16_t zCoord){
    uint16_t adjacent = _focalLength + zCoord;
    uint16_t yProjected = (_focalLength * yCoord) / adjacent;
    return yProjected;
}