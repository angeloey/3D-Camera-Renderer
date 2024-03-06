#include "mbed.h"

class irSense{
    public:

    irSense(PinName sensorOut);

    float readVoltage();

    float getDistance();

    float voltage;

    private:

    float distance;
    //float voltage;
    AnalogIn voltIn;


};
