// Angelo irSense
#include "irSense.h"

irSense::irSense(PinName sensorOut):voltIn(sensorOut){
    voltage = voltIn.read();
}

    // Returns AnalogIn voltage on pin sensorOut
float irSense::readVoltage(void){
    voltage = voltIn.read() * 3.3;
    return voltage;
}

    // Have this here so that this library is not dependant on utils.h
float distanceMap (float value, float istart, float istop, float ostart, float ostop){  
    float mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

    // Returns distance in CM from IR sensor
float irSense::getDistance(){
    readVoltage();
    if(voltage >= 2.3){                                 // crudely approximating from graph included in datasheet
        distance = distanceMap(voltage,2.3,3.2,10,5);   // have to call this loads because relationship is non-linear
    } else if (voltage < 2.3 && voltage >= 1.3){        // See DATASHEET
        distance = distanceMap(voltage,1.3,2.3,20,10);
    } else if (voltage < 1.3 && voltage >= 0.9){
        distance = distanceMap(voltage,0.9,1.3,30,20);
    } else if (voltage < 0.9 && voltage >= 0.7){
        distance = distanceMap(voltage,0.7,0.9,40,30);
    } else if (voltage < 0.7 && voltage >= 0.5){
        distance = distanceMap(voltage,0.5,0.7,60,40);
    } else if (voltage < 0.5 && voltage >= 0.38){
        distance = distanceMap(voltage,0.3,0.5,80,60);
    } else {
        distance = 100;
    }
    return distance;
}