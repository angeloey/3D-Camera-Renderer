// Angelo Maoudis 14074479
// Angelo irSense
#include "mbed.h"

class irSense{
public:
    irSense(PinName sensorOut);

    // Returns AnalogIn voltage on pin sensorOut
    float readVoltage();

    // Returns distance in CM from IR sensor
    float getDistance();
    
private:
    float distance;
    
    float voltage;

    AnalogIn voltIn;
};
