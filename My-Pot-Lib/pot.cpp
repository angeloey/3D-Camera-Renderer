// Angelo Maoudis 14074479
// Angelo Potentiometer
#include "pot.h"

Pot::Pot(PinName potOut):_voltIn(potOut){
    _voltage = _voltIn.read();
}

    // Returns AnalogIn voltage on pin potOut
float Pot::readVoltage(void){
    _voltage = _voltIn.read() * 3.3;
    return _voltage;
}

    // Returns last measured reading, without measuring a new one
float Pot::getLastReading(void){
    return _voltage;
}