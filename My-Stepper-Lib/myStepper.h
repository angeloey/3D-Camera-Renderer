// Angelo Maoudis 14074479
// Angelo myStepper
#include "DigitalOut.h"
#include "mbed.h"

class Stepper {
public:
    Stepper(PinName A1, PinName A2, PinName B1, PinName B2, float stepSize);

    // Steps the servo in the given direction x amount of times
    // Clockwise = 1 > CW    Clockwise = 0 > CCW
    void step(uint8_t clockwise, uint16_t steps);


private:

    float _stepSize;

    int8_t currentStep;

    DigitalOut _A1;

    DigitalOut _A2;

    DigitalOut _B1;

    DigitalOut _B2;
};
