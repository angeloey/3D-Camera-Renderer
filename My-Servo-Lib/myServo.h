// Angelo myServo
#include "mbed.h"

class Servo {
public:
    Servo(PinName pwm);

    // Returns servo motors position
    float readPos();

    // Sets new servo motor position
    void writePos(float newPos);


private:
    PwmOut pwm;

    float position;

};
