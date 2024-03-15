// Angelo Maoudis 14074479
// Angelo rotaryEncoder
#include "PinNames.h"
#include "mbed.h"
#include <cstdint>

class Rotary {
public:

    // Mbed requires using Mbed::Callback<> instead of std::function<> 
    Rotary(PinName encoderA, PinName encoderB, PinName button, Callback<void(void)> buttonFunc, Callback<void(void)> turnFunc);

    // Returns direction most recently turned, CW = true, CCW = false
    bool getClockwise(void);

private:

    // Solves current encoder reading for last turned direction
    void solveEncoder(void);

    InterruptIn _encoderA;

    InterruptIn _encoderB;

    InterruptIn _button;

    bool clockwise;
};
