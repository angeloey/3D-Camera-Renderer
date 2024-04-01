// Angelo Maoudis 14074479
// Angelo myStepper
#pragma once

#include "mbed.h"

namespace constants{
constexpr int16_t MAX_COMMAND_LENGTH = 8; 
}


class UartInterface { 
public:
    UartInterface(PinName pinTX, PinName pinRX, uint32_t Baud);

        // Process incoming serial data
    void processInput(void);
    
        // Execute command based on incoming data
    void runCommand(char commandToRun[constants::MAX_COMMAND_LENGTH]);

private:

    char _character;

    char _msg[32];

    char _command[constants::MAX_COMMAND_LENGTH];

    uint8_t _index;
};