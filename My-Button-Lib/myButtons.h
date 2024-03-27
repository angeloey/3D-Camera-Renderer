// Angelo Maoudis 14074479
// Angelo Touch Buttons
#pragma once

#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

class Button {
public:
    Button(uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax, uint32_t colour, uint32_t symbolColour, uint8_t buttonType, TS_StateTypeDef& TouchState);

    // Draws Button on LCD
    void drawButton(void);

    // Checks if button is pressed
    bool isPressed(void);

private:

    //button colour
    uint32_t _colour;
    uint32_t _symbolColour;

    //button type 0,1,2,3,4,5,6 = blank, up, down, left, right, topRight, bottomRight // purely cosmetic
    uint8_t _buttonType;

    // button coordinates
    uint16_t _xMin;
    uint16_t _xMax;
    uint16_t _yMin;
    uint16_t _yMax;

    // Points for drawing shapes between. Used to draw symbols different button types
    Point up[3];
    Point down[3];
    Point left[3];
    Point right[3];
    Point topRight[3];
    Point bottomRight[3];

    // LCD TouchState
    TS_StateTypeDef TouchState;

    // button pressed flag
    bool _isPressed;    
};

class Slider {
public:
    Slider(uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax, uint32_t colour, uint32_t symbolColour, uint8_t buttonType, TS_StateTypeDef& TouchState, bool horizontal, uint16_t trackStart, uint16_t trackEnd);

    // Draws Button on LCD
    void drawButton(void);

    // Checks if button is pressed
    bool isPressed(void);

    //slider output, inc/dec with progress along its tracks, 0 - 100.
    uint8_t sliderOut;

private:

    // button coordinates
    uint16_t _xMin;
    uint16_t _xMax;
    uint16_t _yMin;
    uint16_t _yMax;

    //button colour
    uint32_t _colour;
    uint32_t _symbolColour;

    //button type 0,1,2,3,4,scales5,6 = blank, up, down, left, right, topRight, bottomRight // purely cosmetic
    uint8_t _buttonType;

    // Points for drawing shapes between. Used to draw symbols different button types
    Point up[3];
    Point down[3];
    Point left[3];
    Point right[3];
    Point topRight[3];
    Point bottomRight[3];

    // Is slider horizontal or vertical?
    bool _horizontal;

    // Should it reset to origin position when you arent holding it
    bool _autoReset;
    uint16_t _xMinReset;
    uint16_t _xMaxReset;
    uint16_t _yMinReset;
    uint16_t _yMaxReset;

    // Slider track
    uint16_t _trackStart;
    uint16_t _trackEnd;

    // LCD TouchState
    TS_StateTypeDef TouchState;

    // button pressed flag
    bool _isPressed;    
};
