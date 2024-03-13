// Angelo Maoudis 14074479
// Angelo rotaryEncoder
#include "PinNames.h"
#include "mbed.h"
#include <cstdint>

class Rotary {
public:

    Rotary(PinName encoderA, PinName encoderB, PinName button);

    // Returns counter value
    uint8_t readCount();

private:

    InterruptIn _encoderA;

    InterruptIn _encoderB;

    InterruptIn _button;

    uint8_t counter;
};
