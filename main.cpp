#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

#include "math.h"

#include "irSense.h"

int valmap();
void drawObject();
void sensorUpdate();
void touchCallback();

const double pi = 3.14159;
uint16_t lastX;
uint16_t lastY;
int fakeAngle = 0; //placeholder
uint16_t targetX;
uint16_t targetY;
uint16_t lastTargetX;
uint16_t lastTargetY;


int valmap (float value, float istart, float istop, float ostart, float ostop){          // map one range of values to another
    uint16_t mappedVal = ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
    return mappedVal;
}

void drawObject(float distance, float angle){ //draw a line from centre, where: length = distance sensed, and angle = current angle of the sensor
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawLine(240, 136, lastX + 240, lastY + 136); // overwrite last drawn line in black

    uint16_t x = distance * cos(pi * 2 * angle / 360); 
    uint16_t y = distance * sin(pi * 2 * angle / 360); /// find x,y coords of a point on a circle of radius 'distance' at angle 'angle'
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawLine(240, 136, x + 240, y + 136); // add centre point offset to calculated x & y values & draw a line from centre to x,y
    
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DrawCircle(x + 240, y + 136, 1); // mark the end of the line with a yellow blob
    BSP_LCD_DrawLine(x + 240, y + 136, lastX + 240, lastY + 136); // connect the dots

    lastX = x;
    lastY = y; // store last used x,y values
}

void sensorUpdate(float distance){ // shit to do when updating sensor reading
    char text [50];

    sprintf((char*)text, "Distance: %f", distance);
    BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawObject(distance, fakeAngle); //placeholder angle till i go get a servo or stepper or something

}

int main(){
    irSense IR(A3); // initialize IR sensor, reading from Pin D2

    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"14074479 MAOUDISA", CENTER_MODE);

    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_Clear(LCD_COLOR_BLACK);


    while(1) {
        sensorUpdate(IR.getDistance());
        wait_us(1000 * 50);
        if (fakeAngle < 360){ //placeholder stuff
            fakeAngle++;
        } else {
            fakeAngle = 0;
            BSP_LCD_Clear(LCD_COLOR_BLACK);
        }

    }
}
