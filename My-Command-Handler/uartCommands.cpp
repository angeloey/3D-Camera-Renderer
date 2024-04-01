// Angelo Maoudis 14074479
// Angelo myStepper
#include "uartCommands.h"
#include <cstdio>

UartInterface::UartInterface(PinName pinTX, PinName pinRX, uint32_t Baud){

        // Create Serial object 
    UnbufferedSerial UnbufferedSerialPort(pinTX, pinRX, Baud);

}

    // process incoming serial data
void UartInterface::processInput(void){

}

    // Execute command based on recieved data
void UartInterface::runCommand(char commandToRun[constants::MAX_COMMAND_LENGTH]){

}