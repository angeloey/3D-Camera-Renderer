// Angelo Maoudis 14074479
// Angelo Touch Buttons
#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

class Button{
public:
    Button(uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax, uint32_t colour, uint32_t symbolColour, uint8_t buttonType, TS_StateTypeDef& touchstate);

    // Draws Button on LCD
    void drawButton(void);

    // Checks if button is pressed
    bool isPressed(void);

    // button coordinates
    uint16_t _xMin;
    uint16_t _xMax;
    uint16_t _yMin;
    uint16_t _yMax;

    //button colour
    uint32_t _colour;
    uint32_t _symbolColour;

    //button type 0,1,2,3,4,5,6 = blank, up, down, left, right, topRight, bottomRight // purely cosmetic
    uint8_t _buttonType;

private:

    // Points for drawing shapes between. Used to draw symbols different button types
    Point up[3];
    Point down[3];
    Point left[3];
    Point right[3];
    Point topRight[3];
    Point bottomRight[3];

    // LCD touchstate
    TS_StateTypeDef touchState;

    // button pressed flag
    bool _isPressed;    
};