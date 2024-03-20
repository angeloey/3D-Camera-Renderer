// Angelo Maoudis 14074479
// Angelo myStepper
#include "DigitalOut.h"
#include "mbed.h"
#include <cstdint>

class Stepper { // Full-Step and Half-Step Stepper Motor
public:
    Stepper(PinName A1, PinName A2, PinName B1, PinName B2, float stepSize);

    // Steps the servo in the given direction x amount of times
    // Clockwise = 1 > CW    Clockwise = 0 > CCW
    // stepMode = 3 > full-step    stepMode = 7 > half-step
    void step(uint8_t clockwise, uint16_t steps, uint8_t stepMode);

    // Jumps to current step in the step table
    void jumpToStep(uint8_t stepMode);

private:

    float _stepSize;

    uint8_t _stepMode;

    int8_t currentStep;

    DigitalOut _A1;

    DigitalOut _A2;

    DigitalOut _B1;

    DigitalOut _B2;
};

class microStepper { // Microstepping Stepper Motor
public:
    microStepper(PinName A1, PinName A2, PinName B1, PinName B2, float stepSize);

    // Steps forward x times through microsteps at x resolution
    void step(uint8_t clockwise, uint16_t steps, float resolution);

    // Adjusts AnalogOut values to reflect desired step position
    void analogStep(float degrees);

private:

    const double pi = 3.14159;

    float _degrees;

    AnalogOut _A1;

    AnalogOut _A2;

    AnalogOut _B1;

    AnalogOut _B2;
};
