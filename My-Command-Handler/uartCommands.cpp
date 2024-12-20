// Angelo Maoudis 14074479
// Angelo myStepper
#include "uartCommands.h"
#include <cstdio>

    // Constructor, setup serial port
UartInterface::UartInterface(PinName pinTX, PinName pinRX, uint32_t Baud, Callback<void(void)> uartFunc):SerialPort(pinTX, pinRX, Baud), _uartFunc(uartFunc){

}

    // process incoming serial data
void UartInterface::processInput(void){

        // get char from serial port
    if (SerialPort.readable()){
        SerialPort.read(&_character, 1);
            // if the character recieved from serial port isn't newline/carriage return, add it to buffer (_command) at index
        if(_character != '\n' && _character != '\r')
        {
            _command[_index++] = _character;
            (_index > constants::MAX_COMMAND_LENGTH) ? _index = 0 : _index;
        }
            // if newline is recieved, terminate string and run the command
        if(_character == '\n')
        {
            _command[_index] = '\0';
            _index = 0;
            runCommand(_command);
        }
    }
}

    // Execute command based on recieved data
void UartInterface::runCommand(char *commandToRun){
        // compare _command string with known commands
    if(strcmp(commandToRun, "Scan") == 0)
    {
        printf("Starting Scan\r\n");
        _uartFunc();

    }else if (strcmp(commandToRun, "testTwo") == 0)
    {
        // Do something else
        printf("testTwo_Recieved\r\n");
    }else{
        printf("Could not recognise command\r\n");
    }
}