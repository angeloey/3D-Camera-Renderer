// Angelo Maoudis 14074479
// Angelo rotaryEncoder
#pragma once

#include "PinNames.h"
#include "mbed.h"
#include <cstdint>
#include <functional>

class Rotary {
public:

    // Mbed requires using Mbed::Callback<> instead of std::function<> 
    Rotary(PinName encoderA, PinName encoderB, PinName button, Callback<void(void)> buttonFunc, Callback<void(void)> turnFunc);

    // Returns direction most recently turned, CW = true, CCW = false
    bool getClockwise(void);

private:

    // Solves current encoder reading for last turned direction 
    void solveEncoder(void);

    std::function<void(void)> func;

    InterruptIn _encoderA;

    InterruptIn _encoderB;

    InterruptIn _button;

    int8_t _currentState;

    int8_t _lastState;

    bool _clockwise;
};
