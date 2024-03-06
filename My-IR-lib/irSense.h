#include "mbed.h"

class irSense{
    public:

    irSense(PinName sensorOut);

    float readVoltage();

    float getDistance();

    private:

    float distance;
    float voltage;
    AnalogIn volt;


};
