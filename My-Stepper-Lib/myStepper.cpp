// Angelo Maoudis 14074479
// Angelo myStepper
#include "myStepper.h"
#include <cstdint>

Stepper::Stepper(PinName A1, PinName A2, PinName B1, PinName B2, float stepSize):_stepSize(stepSize), _A1(A1), _A2(A2), _B1(B1), _B2(B2){
    currentStep = 0;
    _A1.write(0);
    _A2.write(0);
    _B1.write(0);
    _B2.write(0);
}

void Stepper::step(uint8_t clockwise, uint16_t steps){
    if(clockwise == 0){
        for(uint16_t i = 0; i < steps; i++){
            currentStep++;
            if(currentStep > 3){
                currentStep = 0;
            }
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
        }
    }else{
        for(uint16_t i = 0; i < steps; i++){
            currentStep--;
            if(currentStep < 0){
                currentStep = 3;
            }
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
        }
    }
}

