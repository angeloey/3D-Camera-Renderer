#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "math.h"

#include "irSense.h"
#include "utils.h"

Utilities utils;

void drawRadarView();
void drawDepthMap();
void sensorUpdate();

const double pi = 3.14159;
uint8_t radarXoffset = 110;
uint8_t radarYoffset = 222;
uint8_t direction = 0;
uint16_t depthMapXoffset = 310;
uint8_t depthMapYoffset = 222;
uint8_t depthMapLayer = 0;

uint16_t lastX;
uint16_t lastY;
int fakeAngle = 225; //placeholder


void drawRadarView(float distance, float angle){ //draw a line from centre, where: length = distance sensed, and angle = current angle of the sensor
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawLine(radarXoffset, radarYoffset, lastX + radarXoffset, lastY + radarYoffset); // overwrite last drawn line in black

    uint16_t x = distance * cos(pi * 2 * angle / 360); 
    uint16_t y = distance * sin(pi * 2 * angle / 360); /// find x,y coords of a point on a circle of radius 'distance' at angle 'angle'
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawLine(radarXoffset, radarYoffset, x + radarXoffset, y + radarYoffset); // add centre point offset to calculated x & y values & draw a line from centre to x,y
    
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DrawCircle(x + radarXoffset, y + radarYoffset, 1); // mark the end of the line with a yellow blob
    BSP_LCD_DrawLine(x + radarXoffset, y + radarYoffset, lastX + radarXoffset, lastY + radarYoffset); // connect the dots

    lastX = x;
    lastY = y; // store last used x,y values
}

void drawDepthMap(float distance, float angle, uint8_t layer){ //draw 2D image with depth data
    uint16_t drawX = (angle - 225) + depthMapXoffset; //0-90deg from left = x coord 0 to 90 from offset //TODO: "normalise" the image so it isnt skewed
    uint16_t drawY = depthMapYoffset - layer;
    uint8_t red = utils.valmap(distance,0,100,0xFF,0x00);
    BSP_LCD_DrawPixel(drawX, drawY, utils.argbToHex(0xFF, red, 0x00, 0x00)); //pixel gets redder depending on distance data
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawRect(depthMapXoffset - 1, depthMapYoffset - 91, 91, 91); // draw bounding box for image
}

void sensorUpdate(float distance, float angle, uint8_t layer){ // shit to do when updating sensor reading
    char text [50];
    sprintf((char*)text, "Distance: %f Layer: %d", distance, layer);
    BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawRadarView(distance, angle);
    drawDepthMap(distance, angle, layer);
}

int main(){
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"14074479 MAOUDISA", CENTER_MODE);
    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_Clear(LCD_COLOR_BLACK);

    irSense IR(A3); // initialize IR sensor, reading from Pin D2

    while(1) {
        sensorUpdate(IR.getDistance(),fakeAngle,depthMapLayer);
        wait_us(1000 * 10);
        if(direction == 1){
            if (fakeAngle < 315){ //placeholder stuff //TODO: get a servo/stepper and use some real angles
                fakeAngle++;
            } else {
                direction = 0;
                lastX = 0;
                lastY = 0;
                depthMapLayer++;
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, 22, 220, 250);// clear a 220x250 box on left side of screen to clear radar view between layers
            }
        }else{
            if (fakeAngle > 225){ //placeholder stuff
                fakeAngle--;
            } else {
                lastX = 0;
                lastY = 0;
                direction = 1;
                depthMapLayer++;
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, 22, 220, 250);// clear a 220x250 box on left side of screen to clear radar view between layers
            }
        }
        if (depthMapLayer >= 90){ // currently a 90x90 image, subject to change
            depthMapLayer = 0;
        }
    }
}
