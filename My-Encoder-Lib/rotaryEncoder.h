// Angelo Maoudis 14074479
// Angelo rotaryEncoder
#include "PinNames.h"
#include "mbed.h"
#include <cstdint>

class Rotary {
public:

    // using std::function<> template class so i can pass functions to this object
    Rotary(PinName encoderA, PinName encoderB, PinName button, std::function<void(void)> buttonFunc, std::function<void(void)> turnFunc);

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
