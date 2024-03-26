// Angelo Maoudis 14074479
// Angelo IrSense
#include "irSense.h"

IrSense::IrSense(PinName sensorOut):voltIn(sensorOut){
    voltage = voltIn.read();
}

    // Returns AnalogIn voltage on pin sensorOut
float IrSense::readVoltage(void){
    voltage = voltIn.read() * 3.3;
    return voltage;
}

    // Returns last distance measured
float IrSense::lastDistance(void){
    return distance;
}

    // Have this here so that this library is not dependant on myUtils.h
    // Returns a mapped value of the input, from one numerical range to another.
float distanceMap (float value, float istart, float istop, float ostart, float ostop){  
    float mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

    // Returns distance in CM from IR sensor
float IrSense::getDistance(){
    readVoltage();
    if(voltage >= 2.3){                                 // crudely approximating from graph included in datasheet
        distance = distanceMap(voltage,2.3,3.2,10,5);   // have to if-elseif loads because relationship is non-linear
    } else if (voltage < 2.3 && voltage >= 1.3){        // See DATASHEET included in README.MD
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