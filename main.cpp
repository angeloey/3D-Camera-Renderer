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
#include "my3d.h"

void incrementScan(void);
void drawRadarView(float distance, float angle);
void drawDepthMap(float distance, float angle, uint8_t layer, uint8_t range);
void sensorUpdate(float distance, float angle, uint8_t layer, float rangePot);
void rotaryButtonPressed(void);
void rotaryTurned(void);
void draw3dObject(void);

Utilities utils;

Ticker nextStep;    // used to iterate through object scan
Ticker updateScreen;    // used to draw on screen for other views

irSense IR(A3); // initialize IR sensor, reading from Pin A3
Pot rangePot(A4); // initialize Potentiometer, reading from Pin A4
Servo servo(PC_7, 180, 2.5, 1.5); //initialize Servo motor, on pin PC_7 (D0), with a 90 degree range between 1.5 and 2ms.
Stepper stepper(D1, D2, D3, D4, 7.5); //Initialize Stepper motor, on pins D1, D2, D3, D4, with a step angle of 7.5 (not yet implemented)
Rotary encoder(D5, D6, D7, &rotaryButtonPressed, &rotaryTurned); //Initialize Rotary encoder on D5,D6,D7, and pass functions to object

uint16_t pixelIndex = 0;
float xArray[8100]; // x Coordinate array
float yArray[8100]; // y Coordinate array
float zArray[8100]; // z Coordinate array
uint8_t xOffset3d = 250;
uint8_t yOffset3d = 100;

float xArray1[8100]; // x Coordinate array
float yArray1[8100]; // y Coordinate array
float zArray1[8100]; // z Coordinate array

Object3d testObject(xArray, yArray, zArray, -200); // Initialize test 3d object


const double pi = 3.14159;
uint16_t radarXoffset = 110;
uint16_t radarYoffset = 222;
bool direction = false;             // False = CCW, True = CW
uint16_t depthMapXoffset = 310;
uint16_t depthMapYoffset = 222;
uint8_t depthMapLayer = 0;
uint8_t rangeCutoff = 100;

uint16_t lastX;
uint16_t lastY;

int16_t desiredAngle = 0;

bool scanFlag = false;
bool draw3dFlag = false;

uint8_t menuCounter = 0;

int axisCount = 0; // choose axis of rotation TEMPORARY
int count1 = 5;

    // Screen with relevant peripheral data, useful for debugging/testing
void drawDebugScreen(void){
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    char text [50];

    sprintf((char*)text, "IR Distance: %.2f    Voltage: %.2f", IR.getDistance(), IR.readVoltage()); 
    BSP_LCD_ClearStringLine(LINE(2));
    BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *)&text, LEFT_MODE);

    float potVal = rangePot.readVoltage();
    sprintf((char*)text, "Range-Pot Distance: %d    Voltage: %.2f", utils.valmap(potVal, 0, 3.3, 0, 100), potVal); 
    BSP_LCD_ClearStringLine(LINE(3));
    BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)&text, LEFT_MODE);

    sprintf((char*)text, "Encoder Clockwise: %d", encoder.getClockwise()); 
    BSP_LCD_ClearStringLine(LINE(4));
    BSP_LCD_DisplayStringAt(0, LINE(4), (uint8_t *)&text, LEFT_MODE);

    sprintf((char*)text, "Servo Position: %.2f", servo.readPos());
    BSP_LCD_ClearStringLine(LINE(5));
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&text, LEFT_MODE);

    sprintf((char*)text, "Scan Clockwise: %d    Angle: %d   Layer: %d", direction, desiredAngle, depthMapLayer);
    BSP_LCD_ClearStringLine(LINE(6));
    BSP_LCD_DisplayStringAt(0, LINE(6), (uint8_t *)&text, LEFT_MODE);

    sprintf((char*)text, "Radar Offset X: %d   Y: %d", radarXoffset, radarYoffset); 
    BSP_LCD_ClearStringLine(LINE(7));
    BSP_LCD_DisplayStringAt(0, LINE(7), (uint8_t *)&text, LEFT_MODE);

    sprintf((char*)text, "DepthMap Offset X: %d   Y: %d", depthMapXoffset, depthMapYoffset); 
    BSP_LCD_ClearStringLine(LINE(8));
    BSP_LCD_DisplayStringAt(0, LINE(8), (uint8_t *)&text, LEFT_MODE);
}

    // Select & Execute menu options when button is pressed
void rotaryButtonPressed(void){
    updateScreen.detach();
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    switch(menuCounter){
        case 0:
            if(count1 == 0){
                count1++;
                nextStep.attach(incrementScan, 20ms); // 50Hz
            }else{
                nextStep.attach(draw3dObject, 20ms); //50Hz
            }
            break;
        case 1:
            updateScreen.attach(draw3dObject, 20ms);
            break;
        case 2:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"BBBBBBBBBBBBB 3", CENTER_MODE);
            break;
        case 3:
            //updateScreen.attach(drawDebugScreen, 20ms); // 50Hz
            break;
        default:    // should never happen
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"SCCCCCCCCCCCCCCC", CENTER_MODE);
            break;
    }
}

    // Scroll through menu when encoder is turned
void rotaryTurned(void){
    (encoder.getClockwise() == true) ? menuCounter++ : menuCounter--;
    if(menuCounter > 3){
        menuCounter = 0;  // uint underflows to 255 without exception
    }

    BSP_LCD_ClearStringLine(LINE(1));
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);

    switch(menuCounter){
        case 0:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: START SCAN", CENTER_MODE);
            break;
        case 1:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: placeholder 2", CENTER_MODE);
            break;
        case 2:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: menu placeholder 3", CENTER_MODE);
            break;
        case 3:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: DEBUG SCREEN", CENTER_MODE);
            break;
        default:    // should never happen
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Something went wrong (rotaryTurned)", CENTER_MODE);
            break;
    }
}



    // Take peripheral reading, then move servo & stepper to next position
void incrementScan(void){
    scanFlag = true;
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
    rangeCutoff = utils.valmap(rangePot, 0, 3.3, 0, 100);
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
bool spin = false;
void draw3dObject(void){
    draw3dFlag = true;
    spin = true;
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
    //nextStep.attach(incrementScan, 20ms);

        // Creating 8 corners of a cube for testing 3d rendering
     xArray[0] = -40; yArray[0] = -40; zArray[0] = 40; // front bottom left
     xArray[1] = 40; yArray[1] = -40; zArray[1] = 40; // front bottom right
     xArray[2] = 40; yArray[2] = 40; zArray[2] = 40; // front top right
     xArray[3] = -40; yArray[3] = 40; zArray[3] = 40; // front top left

     xArray[4] = -40; yArray[4] = -40; zArray[4] = -40; // back bottom left
     xArray[5] = 40; yArray[5] = -40; zArray[5] = -40; // back bottom right
     xArray[6] = 40; yArray[6] = 40; zArray[6] = -40; // back top right
     xArray[7] = -40; yArray[7] = 40; zArray[7] = -40; // back top left

    for(int i = 0; i < 8100; i++){
        xArray1[i] = xArray[i];
        yArray1[i] = yArray[i];
        zArray1[i] = zArray[i];
    }




    while(1) {
            // Scanning Routine: A lot of this causes mutex, using flags to execute outside of ISR
        if(scanFlag == true){
            sensorUpdate(IR.getDistance(), desiredAngle, depthMapLayer, rangePot.readVoltage());
            (direction == true) ? desiredAngle++ : desiredAngle--;
            if(desiredAngle >= 90 || desiredAngle < 0){
                direction = !direction;
                lastX = 0, lastY = 0;
                depthMapLayer++;
                depthMapLayer++;
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, 22, 300, 250);// clear a 220x250 box on left side of screen to clear radar view between layers
                if(depthMapLayer > 90){ // currently a 90x90 image, subject to change
                    nextStep.detach();
                    depthMapLayer = 0;
                    pixelIndex = 0;
                }
            }
            if(desiredAngle %4 == 0){
            stepper.step(direction, 1, 7);
            }
            //microstep.step(direction, 1, 0.002);
            servo.writePos((float)depthMapLayer);

                // Store Y Coordinate
            yArray[pixelIndex] = -45 + depthMapLayer;
            
                // Store X Coordinate
            xArray[pixelIndex] = -45 + desiredAngle;

                // Store Z Coordinate
            zArray[pixelIndex] = -(int16_t)(rangeCutoff / 2) + (int16_t)round(IR.lastDistance());
            
            if(IR.lastDistance() >= rangeCutoff){
                zArray[pixelIndex] = (int16_t)(rangeCutoff / 2);
            }

            pixelIndex++;

            draw3dFlag = true;
            scanFlag = false;
        }

            // Draw object in 3d
        if(draw3dFlag == true){
            if(spin == true){
                //BSP_LCD_Clear(LCD_COLOR_BLACK);

                for(int j = 0; j<360; j++){
                    testObject.rotateProjection(j, axisCount);
                    testObject.generateProjected();
                    BSP_LCD_Clear(LCD_COLOR_BLACK);
                    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
                    BSP_LCD_DrawLine(testObject.xProjected[0] + xOffset3d, testObject.yProjected[0] + yOffset3d, testObject.xProjected[1] + xOffset3d, testObject.yProjected[1] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[1] + xOffset3d, testObject.yProjected[1] + yOffset3d, testObject.xProjected[2] + xOffset3d, testObject.yProjected[2] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[2] + xOffset3d, testObject.yProjected[2] + yOffset3d, testObject.xProjected[3] + xOffset3d, testObject.yProjected[3] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[3] + xOffset3d, testObject.yProjected[3] + yOffset3d, testObject.xProjected[0] + xOffset3d, testObject.yProjected[0] + yOffset3d);

                    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
                    BSP_LCD_DrawLine(testObject.xProjected[4] + xOffset3d, testObject.yProjected[4] + yOffset3d, testObject.xProjected[5] + xOffset3d, testObject.yProjected[5] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[5] + xOffset3d, testObject.yProjected[5] + yOffset3d, testObject.xProjected[6] + xOffset3d, testObject.yProjected[6] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[6] + xOffset3d, testObject.yProjected[6] + yOffset3d, testObject.xProjected[7] + xOffset3d, testObject.yProjected[7] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[7] + xOffset3d, testObject.yProjected[7] + yOffset3d, testObject.xProjected[4] + xOffset3d, testObject.yProjected[4] + yOffset3d);
                    
                    BSP_LCD_SetTextColor(LCD_COLOR_RED);
                    BSP_LCD_DrawLine(testObject.xProjected[7] + xOffset3d, testObject.yProjected[7] + yOffset3d, testObject.xProjected[3] + xOffset3d, testObject.yProjected[3] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[6] + xOffset3d, testObject.yProjected[6] + yOffset3d, testObject.xProjected[2] + xOffset3d, testObject.yProjected[2] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[4] + xOffset3d, testObject.yProjected[4] + yOffset3d, testObject.xProjected[0] + xOffset3d, testObject.yProjected[0] + yOffset3d);

                    BSP_LCD_DrawLine(testObject.xProjected[5] + xOffset3d, testObject.yProjected[5] + yOffset3d, testObject.xProjected[1] + xOffset3d, testObject.yProjected[1] + yOffset3d);

                    for(int i = 0; i < 8100; i++){
                        xArray[i] = xArray1[i];
                        yArray[i] = yArray1[i];
                        zArray[i] = zArray1[i];
                    }
                }
                axisCount++;
                if(axisCount > 2){
                    axisCount = 0;
                }
                //testObject.rotateProjection(1, 1);
            }
            //testObject.generateProjected();
            // BSP_LCD_Clear(LCD_COLOR_BLACK);
            // BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
            // for(int i = 0; i < 8099; i++){
            //     BSP_LCD_DrawLine(testObject.xProjected[i] +xOffset3d, testObject.yProjected[i] +yOffset3d, testObject.xProjected[i+1] +xOffset3d, testObject.yProjected[i+1] +yOffset3d);
            // }

            draw3dFlag = false;
        }
    }
}

// stepper angle is 3.6 in half step, 7.2 in full step //ish, cant find data online
// TODO:
// https://github.com/cbm80amiga/ST7735_3d_filled_vector/blob/master/gfx3d.h
