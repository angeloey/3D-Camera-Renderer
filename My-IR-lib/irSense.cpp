// Angelo Maoudis 14074479
// Angelo IrSense
#include "irSense.h"

IrSense::IrSense(PinName sensorOut):_voltIn(sensorOut){
    _voltage = _voltIn.read();
}

    // Returns AnalogIn voltage on pin sensorOut
float IrSense::readVoltage(void){
    _voltage = _voltIn.read() * 3.3;
    return _voltage;
}

    // Returns last distance measured
float IrSense::lastDistance(void){
    return _distance;
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
    if(_voltage >= 2.3){                                 // crudely approximating from graph included in datasheet
        _distance = distanceMap(_voltage,2.3,3.2,10,5);   // have to if-elseif loads because relationship is non-linear
    } else if (_voltage < 2.3 && _voltage >= 1.3){        // See DATASHEET included in README.MD
        _distance = distanceMap(_voltage,1.3,2.3,20,10);
    } else if (_voltage < 1.3 && _voltage >= 0.9){
        _distance = distanceMap(_voltage,0.9,1.3,30,20);
    } else if (_voltage < 0.9 && _voltage >= 0.7){
        _distance = distanceMap(_voltage,0.7,0.9,40,30);
    } else if (_voltage < 0.7 && _voltage >= 0.5){
        _distance = distanceMap(_voltage,0.5,0.7,60,40);
    } else if (_voltage < 0.5 && _voltage >= 0.38){
        _distance = distanceMap(_voltage,0.3,0.5,80,60);
    } else {
        _distance = 100;
    }
    return _distance;
}