// Angelo Maoudis 14074479
// Angelo myServo
#include "myServo.h"
#include <stdint.h>

Servo::Servo(PinName servoPin, uint16_t servoRange, float servoTimeStart, float servoTimeEnd):pwm(servoPin), _servoRange(servoRange), _servoTimeStart(servoTimeStart), _servoTimeEnd(servoTimeEnd){
    writePos(0);
    pwm.period_ms(20);
}

    // Have this here so that this library is not dependant on myUtils.h
    // Returns a mapped value of the input, from one numerical range to another.
float servoMap (float value, float istart, float istop, float ostart, float ostop){  
    float mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

    // Returns servo motors position in degrees
float Servo::readPos() {
    return positionDegrees;
}

    // Sets new servo motor position. PWM output at pulsewidth corresponding to desired position.
    // Achieved by mapping the value "newPos" in range 0 -> _servoRange (degrees)
    // to its equivalent value in the range _servoTimeStart -> _servoTimeEnd (uSeconds).
void Servo::writePos(float newPos) {
    positionDegrees = newPos;
    position = servoMap(newPos, 0, _servoRange, (_servoTimeStart * 1000), (_servoTimeEnd * 1000));    // map: 0 -> _servoRange in degrees, to: _servoTimeStart -> _servoTimeEnd in uSeconds.
    pwm.pulsewidth_us(position);
}

