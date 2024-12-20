// Angelo Maoudis 14074479
// Angelo Potentiometer
#pragma once

#include "mbed.h"

class Pot{
public:
    Pot(PinName potOut);

    // Returns AnalogIn voltage on pin potOut
    float readVoltage();

    // Returns last measured reading, without measuring a new one
    float getLastReading();
    
private:
    float _voltage;

    AnalogIn _voltIn;
};
