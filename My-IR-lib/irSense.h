// Angelo Maoudis 14074479
// Angelo IrSense
#pragma once

#include "mbed.h"

class IrSense{
public:
    IrSense(PinName sensorOut);

    // Returns AnalogIn voltage on pin sensorOut
    float readVoltage();

    // Returns distance in CM from IR sensor
    float getDistance();

    // Returns last distance measured
    float lastDistance();
    
private:
    float _distance;
    
    float _voltage;

    AnalogIn _voltIn;
};
