#include "irSense.h"

irSense::irSense(PinName sensorOut):volt(sensorOut){
    voltage = volt.read()
}

float irSense::readVoltage(void){
    voltage = volt.read()
}

int valmap (float value, float istart, float istop, float ostart, float ostop){          // map one range of values to another
    uint16_t mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

float irSense::getDistance(){
    readVoltage();
    // crudely approximating from graph included in datasheet
    if(voltage >= 2.3){ // 3.1 - 2-3 = 5 - 10
        distance = valmap(voltage,2.3,3.2,5,10);
    } else if (voltage < 2.3 || voltage >= 1.3){
        distance = valmap(voltage,1.3,2.3,10,20);
    } else if (voltage < 1.3 || voltage >= 0.9){
        distance = valmap(voltage,0.9,1.3,20,30);
    } else if (voltage < 0.9 || voltage >= 0.7){
        distance = valmap(voltage,0.7,0.9,30,40);
    } else if (voltage < 0.7 || voltage >= 0.5){
        distance = valmap(voltage,0.5,0.7,40,50);
    } else if (voltage < 0.5 || voltage >= 0.38){
        distance = valmap(voltage,0.38,0.5,50,80);
    } else {
        distance = 100;
    }
}