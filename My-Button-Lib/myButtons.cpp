// Angelo Maoudis 14074479
// Angelo Touch Buttons
#include "myButtons.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

    // Normal touch button
Button::Button(uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax, uint32_t colour, uint32_t symbolColour, uint8_t buttonType, TS_StateTypeDef& touchState):_xMin(xMin), _xMax(xMax), _yMin(yMin), _yMax(yMax), _colour(colour), _symbolColour(symbolColour), _buttonType(buttonType){

        // Define points for drawing shapes between. Used to draw symbols on different button types
        // Up facing arrow
    up[0].X = xMin + ((xMax - xMin)/2);        up[0].Y = yMin +5;
    up[1].X = xMax -5;                         up[1].Y = yMax - ((yMax - yMin)/2); 
    up[2].X = xMin +5;                         up[2].Y = yMax - ((yMax - yMin)/2);

        // Down facing arrow
    down[0].X = xMin + ((xMax - xMin)/2);      down[0].Y = yMax -5;
    down[1].X = xMax -5;                       down[1].Y = yMax - ((yMax - yMin)/2);
    down[2].X = xMin +5;                       down[2].Y = yMax - ((yMax - yMin)/2);

        // Left facing arrow
    left[0].X = xMin +5;                       left[0].Y = yMin + ((yMax - yMin)/2);
    left[1].X = xMin + ((xMax - xMin)/2);      left[1].Y = yMin +5; 
    left[2].X = xMin + ((xMax - xMin)/2);      left[2].Y = yMax -5; 

        // Right facing arrow
    right[0].X = xMax -5;                      right[0].Y = yMin + ((yMax - yMin)/2);
    right[1].X = xMin + ((xMax - xMin)/2);     right[1].Y = yMin +5;
    right[2].X = xMin + ((xMax - xMin)/2);     right[2].Y = yMax -5;

        // Top Right facing arrow
    topRight[0].X = xMax -5;                   topRight[0].Y = yMin +5;
    topRight[1].X = xMin +5;                   topRight[1].Y = yMin +5;
    topRight[2].X = xMax -5;                   topRight[2].Y = yMax -5;

        // Bottom Right facing arrow
    bottomRight[0].X = xMax -5;                bottomRight[0].Y = yMax -5;
    bottomRight[1].X = xMax -5;                bottomRight[1].Y = yMin +5;
    bottomRight[2].X = xMin +5;                bottomRight[2].Y = yMax -5;

    _isPressed = false;
}

    // Draw button and its corresponding symbol
void Button::drawButton(){
    BSP_LCD_SetTextColor(_colour);
    BSP_LCD_DrawRect(_xMin, _yMin, (_xMax - _xMin), (_yMax - _yMin));
    BSP_LCD_SetTextColor(_symbolColour);
    switch(_buttonType){
        case 0: //blank
            break;
        case 1: //up
            BSP_LCD_DrawPolygon(up, 3);
            break;
        case 2: //down
            BSP_LCD_DrawPolygon(down, 3);
            break;
        case 3: //left
            BSP_LCD_DrawPolygon(left, 3);
            break;
        case 4: //right
            BSP_LCD_DrawPolygon(right, 3);
            break;
        case 5: //topRight
            BSP_LCD_DrawPolygon(topRight, 3);
            break;
        case 6: //bottomRight
            BSP_LCD_DrawPolygon(bottomRight, 3);
            break;
        default:
            break;
    }
}

    // Return true if the screen is touched within the buttons coordinates
bool Button::isPressed(void){
    BSP_TS_GetState(&touchState);
            if (touchState.touchDetected) {              
                if (*touchState.touchX >= _xMin && *touchState.touchX <= _xMax && *touchState.touchY >= _yMin && *touchState.touchY <= _yMax) {
                    _isPressed = true;
                }
            } else{
                _isPressed = false;
            }
    return _isPressed;
}




    // Sliding touch button
Slider::Slider(uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax, uint32_t colour, uint32_t symbolColour, uint8_t buttonType, TS_StateTypeDef& touchstate, bool horizontal, uint16_t trackStart, uint16_t trackEnd):_xMin(xMin), _xMax(xMax), _yMin(yMin), _yMax(yMax), _colour(colour), _symbolColour(symbolColour), _buttonType(buttonType), _horizontal(horizontal), _trackStart(trackStart), _trackEnd(trackEnd){

        // Define points for drawing shapes between. Used to draw symbols on different button types
        // Up facing arrow
    up[0].X = xMin + ((xMax - xMin)/2);        up[0].Y = yMin +5;
    up[1].X = xMax -5;                         up[1].Y = yMax - ((yMax - yMin)/2); 
    up[2].X = xMin +5;                         up[2].Y = yMax - ((yMax - yMin)/2);

        // Down facing arrow
    down[0].X = xMin + ((xMax - xMin)/2);      down[0].Y = yMax -5;
    down[1].X = xMax -5;                       down[1].Y = yMax - ((yMax - yMin)/2);
    down[2].X = xMin +5;                       down[2].Y = yMax - ((yMax - yMin)/2);

        // Left facing arrow
    left[0].X = xMin +5;                       left[0].Y = yMin + ((yMax - yMin)/2);
    left[1].X = xMin + ((xMax - xMin)/2);      left[1].Y = yMin +5; 
    left[2].X = xMin + ((xMax - xMin)/2);      left[2].Y = yMax -5; 

        // Right facing arrow
    right[0].X = xMax -5;                      right[0].Y = yMin + ((yMax - yMin)/2);
    right[1].X = xMin + ((xMax - xMin)/2);     right[1].Y = yMin +5;
    right[2].X = xMin + ((xMax - xMin)/2);     right[2].Y = yMax -5;

        // Top Right facing arrow
    topRight[0].X = xMax -5;                   topRight[0].Y = yMin +5;
    topRight[1].X = xMin +5;                   topRight[1].Y = yMin +5;
    topRight[2].X = xMax -5;                   topRight[2].Y = yMax -5;

        // Bottom Right facing arrow
    bottomRight[0].X = xMax -5;                bottomRight[0].Y = yMax -5;
    bottomRight[1].X = xMax -5;                bottomRight[1].Y = yMin +5;
    bottomRight[2].X = xMin +5;                bottomRight[2].Y = yMax -5;

    _isPressed = false;
}

    // Draw button and its corresponding symbol
void Slider::drawButton(){
    BSP_LCD_SetTextColor(_colour);
    BSP_LCD_DrawRect(_xMin, _yMin, (_xMax - _xMin), (_yMax - _yMin));
    BSP_LCD_SetTextColor(_symbolColour);
    switch(_buttonType){
        case 0: //blank
            break;
        case 1: //up
            BSP_LCD_DrawPolygon(up, 3);
            break;
        case 2: //down
            BSP_LCD_DrawPolygon(down, 3);
            break;
        case 3: //left
            BSP_LCD_DrawPolygon(left, 3);
            break;
        case 4: //right
            BSP_LCD_DrawPolygon(right, 3);
            break;
        case 5: //topRight
            BSP_LCD_DrawPolygon(topRight, 3);
            break;
        case 6: //bottomRight
            BSP_LCD_DrawPolygon(bottomRight, 3);
            break;
        default:
            break;
    }
}

    // Return true if the screen is touched within the buttons coordinates
bool Slider::isPressed(void){
    BSP_TS_GetState(&touchState);
            if (touchState.touchDetected) {              
                if (*touchState.touchX >= _xMin && *touchState.touchX <= _xMax && *touchState.touchY >= _yMin && *touchState.touchY <= _yMax) {
                    _isPressed = true;
                }
            } else{
                _isPressed = false;
            }
    return _isPressed;
}