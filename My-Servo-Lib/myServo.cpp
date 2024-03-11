// Angelo myServo
#include "myServo.h"

Servo::Servo(PinName pwm):pwm(pwm){
    writePos(0);
}

    // Returns servo motors position
float Servo::readPos() {
    return position;
}

    // Sets new servo motor position
void Servo::writePos(float newPos) {
    position = newPos;
    pwm.pulsewidth_us(position);
}

