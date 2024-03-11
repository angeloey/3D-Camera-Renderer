// Angelo Maoudis 14074479
// Angelo myServo
#include "myServo.h"
#include <stdint.h>

Servo::Servo(PinName servoPin, uint16_t servoRange, float servoTime):pwm(servoPin), _servoRange(servoRange), _servoTime(servoTime){
    writePos(0);
    pwm.period_ms(20);
}

    // Have this here so that this library is not dependant on myUtils.h
float servoMap (float value, float istart, float istop, float ostart, float ostop){  
    float mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

    // Returns servo motors position in degrees
float Servo::readPos() {
    return positionDegrees;
}

    // Sets new servo motor position
void Servo::writePos(float newPos) {
    positionDegrees = newPos;
    position = servoMap(newPos, 0, _servoRange, 0, (_servoTime * 1000));    // map 0 -> _servoRange degrees to, 0 -> _servoTime uSeconds
    pwm.pulsewidth_us(position);
}

