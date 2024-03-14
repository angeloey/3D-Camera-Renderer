// Angelo Maoudis 14074479
#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "math.h"

#include "irSense.h"
#include "myUtils.h"
#include "pot.h"
#include "myServo.h"
#include "myStepper.h"
#include "rotaryEncoder.h"
#include <cstdint>

void incrementScan();
void drawRadarView(float distance, float angle);
void drawDepthMap(float distance, float angle, uint8_t layer, uint8_t range);
void sensorUpdate(float distance, float angle, uint8_t layer, float rangePot);
void rotaryButtonPressed(void);
void rotaryTurned(void);

Utilities utils;

Ticker nextStep;

irSense IR(A3); // initialize IR sensor, reading from Pin A3
Pot rangePot(A4); // initialize Potentiometer, reading from Pin A4
Servo servo(PC_7, 180, 2.5, 1.5); //initialize Servo motor, on pin PC_7 (D0), with a 90 degree range between 1.5 and 2ms.
Stepper stepper(D1, D2, D3, D4, 7.5); //Initialize Stepper motor, on pins D1, D2, D3, D4, with a step angle of 7.5 (not yet implemented)
Rotary encoder(D5, D6, D7, &rotaryButtonPressed, &rotaryTurned); //Initialize Rotary encoder on D5,D6,D7, and pass functions to object

const double pi = 3.14159;
uint8_t radarXoffset = 110;
uint8_t radarYoffset = 222;
bool direction = false;             // False = CCW, True = CW
uint16_t depthMapXoffset = 310;
uint8_t depthMapYoffset = 222;
uint8_t depthMapLayer = 0;

uint16_t lastX;
uint16_t lastY;
int desiredAngle = 0;

void rotaryButtonPressed(void){

}

void rotaryTurned(void){

}

    // Take peripheral reading, then move servo & stepper to next position
void incrementScan(void){
    sensorUpdate(IR.getDistance(),desiredAngle,depthMapLayer,rangePot.readVoltage());
    (direction == true) ? desiredAngle++ : desiredAngle--;
    if(desiredAngle >= 90 || desiredAngle < 0){
        direction = !direction;
        lastX = 0, lastY = 0;
        depthMapLayer++;
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(0, 22, 220, 250);// clear a 220x250 box on left side of screen to clear radar view between layers
        depthMapLayer = (depthMapLayer >= 90) ? 0 : depthMapLayer; // currently a 90x90 image, subject to change
    }
    stepper.step(direction, 1, 7);
    servo.writePos((float)depthMapLayer);
}

    // Draw a line from centre, where: length = distance sensed, and angle = current angle of the sensor
void drawRadarView(float distance, float angle){ 
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawLine(radarXoffset, radarYoffset, lastX + radarXoffset, lastY + radarYoffset); // overwrite last drawn line in black

    uint16_t x = distance * cos(pi * 2 * (angle + 225) / 360); 
    uint16_t y = distance * sin(pi * 2 * (angle + 225) / 360); /// find x,y coords of a point on a circle of radius 'distance' at angle 'angle'
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawLine(radarXoffset, radarYoffset, x + radarXoffset, y + radarYoffset); // add centre point offset to calculated x & y values & draw a line from centre to x,y
    
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DrawCircle(x + radarXoffset, y + radarYoffset, 1); // mark the end of the line with a yellow blob
    BSP_LCD_DrawLine(x + radarXoffset, y + radarYoffset, lastX + radarXoffset, lastY + radarYoffset); // connect the blobs

    lastX = x;
    lastY = y; // store last used x,y values
}

    // Draw 2D image with depth data
void drawDepthMap(float distance, float angle, uint8_t layer, uint8_t range){ 
    uint16_t drawX = (angle) + depthMapXoffset; //0-90deg from left = x coord 0 to 90 from offset //TODO: "normalise" the image so it isnt skewed 
    uint16_t drawY = depthMapYoffset - layer;                                                     //(Z = cos(angle) * distance??? check maths before implementing)
    uint8_t red = utils.valmap(distance,0,range,0xFF,0x00);
    BSP_LCD_DrawPixel(drawX, drawY, utils.argbToHex(0xFF, red, 0x00, 0x00)); //pixel gets redder depending on distance data
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawRect(depthMapXoffset - 1, depthMapYoffset - 91, 91, 91); // draw bounding box for image
}

    // Calls display functions with updated/formatted peripheral data
void sensorUpdate(float distance, float angle, uint8_t layer, float rangePot){
    uint8_t rangeCutoff = utils.valmap(rangePot, 0, 3.3, 0, 100);
    if(distance > rangeCutoff){
        distance = rangeCutoff; // using this to control range with pot, while also scaling red values in function "drawDepthMap"
    }
    char text [50];
    sprintf((char*)text, "Distance: %f Layer: %d MaxRange: %d", distance, layer, rangeCutoff); //debug readout for now
    BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawRadarView(distance, angle);
    drawDepthMap(distance, angle, layer, rangeCutoff);
}

    // Main
int main(){

        // Setup LCD, Show startup message, Clear to black
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"14074479 MAOUDISA", CENTER_MODE);
    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_Clear(LCD_COLOR_BLACK);

        // Attatch ticker to increment scan progress every 100ms
    nextStep.attach(incrementScan, 100ms);


    while(1) {

    }
}

// stepper angle is 3.6 in half step, 7.2 in full step //ish, cant find data online
// TODO:
// https://github.com/cbm80amiga/ST7735_3d_filled_vector/blob/master/gfx3d.h
