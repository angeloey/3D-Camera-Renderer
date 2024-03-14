// Angelo Maoudis 14074479
// Angelo rotaryEncoder
#include "rotaryEncoder.h"

Rotary::Rotary(PinName encoderA, PinName encoderB, PinName button, std::function<void(void)> buttonFunc, std::function<void(void)> turnFunc):_encoderA(encoderA),_encoderB(encoderB),_button(button){
    clockwise = false;

        // Call function passed here when button is pressed
    _button.rise(buttonFunc);

        // Update Direction on A Interrupts
    _encoderA.rise(callback(this, &Rotary::solveEncoder));
    _encoderA.fall(callback(this, &Rotary::solveEncoder));

        // Call function passed here on B Interrupts
    _encoderB.rise(turnFunc);
    _encoderB.fall(turnFunc);
}

    //  Read encoder states, and compare to previous states
    //  CW = A: 0 0 1 1     CCW = A: 0 0 1 1
    //       B: 1 0 0 1           B: 0 1 1 0
void Rotary::solveEncoder(void){
    (_encoderB.read() == 0 && _encoderA == 1) ? clockwise = true : clockwise = false; // Rising edge (A = 1)
    (_encoderB.read() == 1 && _encoderA == 0) ? clockwise = true : clockwise = false; // Falling edge (A = 0)
}

    // Returns direction most recently turned, CW = true, CCW = false
bool Rotary::getClockwise(void){
    return clockwise;
}