// Angelo Maoudis 14074479
// Angelo myServo
#include "mbed.h"

class Servo {
public:

    // servoRange = servo range of motion in degrees
    // servoTime = servo pulse length required for maximum rotation in ms
    Servo(PinName servoPin, uint16_t servoRange, float servoTime);

    // Returns servo motors position in degrees
    float readPos();

    // Sets new servo motor position in degrees
    void writePos(float newPos);


private:
    PwmOut pwm;

    uint16_t _servoRange;

    float _servoTime;

    float position;

    uint8_t positionDegrees;
};
