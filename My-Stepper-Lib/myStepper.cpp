// Angelo Maoudis 14074479
// Angelo myStepper
#include "myStepper.h"
#include <cstdint>

    // Stepper constructor for full or half step modes
Stepper::Stepper(PinName A1, PinName A2, PinName B1, PinName B2, float stepSize):_stepSize(stepSize), _A1(A1), _A2(A2), _B1(B1), _B2(B2){
    currentStep = 0;
    _A1.write(0);
    _A2.write(0);
    _B1.write(0);
    _B2.write(0);
}

    // Step x steps in given direction
void Stepper::step(uint8_t clockwise, uint16_t steps, uint8_t stepMode){
    if(clockwise == 0){
        for(uint16_t i = 0; i < steps; i++){
            currentStep++;
            if(currentStep > stepMode){
                currentStep = 0;
            }
            jumpToStep(stepMode);
        }
    }else{
        for(uint16_t i = 0; i < steps; i++){
            currentStep--;
            if(currentStep < 0){
                currentStep = stepMode;
            }
            jumpToStep(stepMode);
        }
    }
}

    // Step Table
void Stepper::jumpToStep(uint8_t stepMode){
    if(stepMode == 3){          // Full-Step Table
        switch(currentStep){
        case 0:
            _A1.write(1);
            _A2.write(0);
            _B1.write(0);
            _B2.write(1);
            break;
        case 1:
            _A1.write(1);
            _A2.write(1);
            _B1.write(0);
            _B2.write(0);
            break;
        case 2:
            _A1.write(0);
            _A2.write(1);
            _B1.write(1);
            _B2.write(0);
            break;
        case 3:
            _A1.write(0);
            _A2.write(0);
            _B1.write(1);
            _B2.write(1);
            break;
        default:
            _A1.write(0);
            _A2.write(0);
            _B1.write(0);
            _B2.write(0);
            break;
        }
    }else if(stepMode == 7){    //Half-Step Table
        switch(currentStep){
        case 0:
            _A1.write(1);
            _A2.write(0);
            _B1.write(0);
            _B2.write(1);
            break;
        case 1:
            _A1.write(1);
            _A2.write(0);
            _B1.write(0);
            _B2.write(0);
            break;
        case 2:
            _A1.write(1);
            _A2.write(1);
            _B1.write(0);
            _B2.write(0);
            break;
        case 3:
            _A1.write(0);
            _A2.write(1);
            _B1.write(0);
            _B2.write(0);
            break;
        case 4:
            _A1.write(0);
            _A2.write(1);
            _B1.write(1);
            _B2.write(0);
            break;
        case 5:
            _A1.write(0);
            _A2.write(0);
            _B1.write(1);
            _B2.write(0);
            break;
        case 6:
            _A1.write(0);
            _A2.write(0);
            _B1.write(1);
            _B2.write(1);
            break;
        case 7:
            _A1.write(0);
            _A2.write(0);
            _B1.write(0);
            _B2.write(1);
            break;
        default:
            _A1.write(0);
            _A2.write(0);
            _B1.write(0);
            _B2.write(0);
            break;
        }
    }
}



    // Stepper constructor for microstep mode
microStepper::microStepper(PinName A1, PinName A2, PinName B1, PinName B2, float stepSize):_A1(A1), _A2(A2), _B1(B1), _B2(B2){
    _degrees = 0;
    _A1.write(0);
    _A2.write(0);
    _B1.write(0);
    _B2.write(0);
}

    // Steps forward x times through microsteps at x resolution
void microStepper::step(uint8_t clockwise, uint16_t steps, float resolution){
    if(clockwise == 0){
        for(uint16_t i = 0; i < steps; i++){
            _degrees += resolution;
            if(_degrees > 180){
                _degrees = 0;
            }
            analogStep(_degrees);
        }
    }else{
        for(uint16_t i = 0; i < steps; i++){
            _degrees -= resolution;
            if(_degrees < 0){
                _degrees = 180;
            }
            analogStep(_degrees);
        }
    }
}

    // Returns 0 if input is negative, otherwise returns input
float returnPositive(float input){
    if(input > 0){
        return input;
    }else{
        return 0;
    }
}

    // Adjusts AnalogOut values to reflect desired step position
void microStepper::analogStep(int16_t degrees){                    // Output across phases should look like this \/
    _A1.write(returnPositive(sin(degrees * pi / 180)));                             //-''-....-''-......    A1
    _A2.write(returnPositive(cos(degrees * pi / 180)));                             //..-''-....-''-....    A2
    _B1.write(returnPositive(sin(180 + degrees * pi / 180)));                       //....-''-....-''-..    B1
    _B2.write(returnPositive(cos(180 + degrees * pi / 180)));                       //......-''-....-''-    B2
}
