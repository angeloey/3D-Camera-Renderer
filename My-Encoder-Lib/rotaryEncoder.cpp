// Angelo Maoudis 14074479
// Angelo rotaryEncoder
#include "rotaryEncoder.h"
#include "PinNamesTypes.h"
#include <cstdio>

Rotary::Rotary(PinName encoderA, PinName encoderB, PinName button, Callback<void(void)> buttonFunc, Callback<void(void)> turnFunc):_encoderA(encoderA),_encoderB(encoderB),_button(button),func(turnFunc){
    clockwise = false;
    _encoderA.mode(PullDown);
    _encoderB.mode(PullDown);
    _button.mode(PullDown);

        // Call function passed here when button is pressed
    _button.rise(buttonFunc);

        // Update direction on rising edge Interrupts
    _encoderA.rise(callback(this, &Rotary::solveEncoder));
    _encoderA.fall(callback(this, &Rotary::solveEncoder));
}

    //  Read encoder states, and compare to previous states
    //  CW = A: 0 0 1 1     CCW = A: 0 0 1 1
    //       B: 1 0 0 1           B: 0 1 1 0
void Rotary::solveEncoder(void){
    uint8_t chanA = _encoderA.read(); // read channel A and B
    uint8_t chanB = _encoderB.read();
    currentState = (chanA << 1) | (chanB); // storing states of A and B //2bit

    //11->00->11->00 is CCW
    //10->01->10->01 is CW
    if((lastState == 0b11 && currentState == 0b00)||(lastState == 0b00 && currentState == 0b11)){
        clockwise = false;
        func();
    }else if((lastState == 0b10 && currentState == 0b01)||(lastState == 0b01 && currentState == 0b10)){
        clockwise = true;
        func();
    }
    lastState = currentState;
}

    // Returns direction most recently turned, CW = true, CCW = false
bool Rotary::getClockwise(void){
    return clockwise;
}