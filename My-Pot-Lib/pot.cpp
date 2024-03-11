// Angelo Maoudis 14074479
// Angelo Potentiometer
#include "pot.h"

Pot::Pot(PinName potOut):voltIn(potOut){
    voltage = voltIn.read();
}

    // Returns AnalogIn voltage on pin potOut
float Pot::readVoltage(void){
    voltage = voltIn.read() * 3.3;
    return voltage;
}

    // Returns last measured reading, without measuring a new one
float Pot::getLastReading(void){
    return voltage;
}