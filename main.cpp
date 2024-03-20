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

    //Function declarations
void incrementScan(void);
void drawRadarView(float distance, float angle);
void drawDepthMap(float distance, float angle, uint8_t layer, uint8_t range);
void sensorUpdate(float distance, float angle, uint8_t layer, float rangePot);
void rotaryButtonPressed(void);
void rotaryTurned(void);
void drawDebugScreen(void);
void draw3dObject(void);
void rotatingCubeDemo(void);

    // General consensus online: multiple 1D arrays are faster than a multi-dimensional array, save for minimal gains in edge cases.
    // Decision: Prioritize readibility and use 1D array approach
    // XYZ values for 3d render are stored/modified in these arrays
float xArray[8100]; // x
float yArray[8100]; // y
float zArray[8100]; // z
float xArraySAVE[8100]; // x used to store original XYZ values & refresh after rotation
float yArraySAVE[8100]; // y
float zArraySAVE[8100]; // z
uint16_t pixelIndex = 0; // current xyzArray index to write/read

    // Offsets/Coordinates for drawing on LCD
uint8_t xOffset3d = 240; // Offsets for drawing 3D object centred on (0,0) which is at the top left of the LCD
uint8_t yOffset3d = 136;
uint16_t radarXoffset = 110; // Position offsets for drawing radar view
uint16_t radarYoffset = 222;
uint16_t depthMapXoffset = 310; // Position offsets for drawing depth map
uint16_t depthMapYoffset = 222;
uint16_t lastX; // Coordinates of last drawn line for radar view, used to overwrite in black without clearing entire radar
uint16_t lastY; // purely cosmetic

    // Obtaining XYZ values via scan
bool direction = false;  // Scan direction: False = CCW(-X), True = CW(+X) 
int16_t desiredAngle = 0;  // Scan/Stepper angle (Horizontal/X Axis)
uint8_t depthMapLayer = 0; // Y value/layer of depthmap, also used for servo angle (Vertical/Y Axis)
uint8_t rangeCutoff = 100; // Any distances sensed past this value, are capped to this value (Depth/Z Axis)

    // Flags, mostly for executing functions incompatible with ISR (mutex, too slow, etc.)
bool scanFlag = false;  // Scan object flag
bool draw3dFlag = false;  // 3D Render flag
bool spin = false; // Rotate 3D Render flag

    // Menu navigation/control
int8_t menuCounter = 0; // used to store last/select a menu option via rotary encoder
int count1 = 0; // temporary, rotary encoder button, use CTRL-F

    // Misc.
int axisCount = 0; // choose axis of rotation TEMPORARY
const double pi = 3.14159265359;

    // Initialization, Classes/Objects/Structs/Etc.
irSense IR(A0); // initialize IR sensor, reading from Pin A0
Pot rangePot(A1); // initialize Potentiometer, reading from Pin A1
Servo servo(PC_7, 180, 2.5, 1.5); // initialize Servo motor, on pin PC_7 (D0), with a 90 degree range between 1.5 and 2ms.
Stepper stepper(D1, D2, D3, D4, 7.5); // initialize Stepper motor, on pins D1, D2, D3, D4, with a step angle of 7.5 (not yet implemented)
Rotary encoder(D5, D6, D7, &rotaryButtonPressed, &rotaryTurned); // initialize Rotary encoder on D5,D6,D7, and pass functions to object
Object3d testObject(xArray, yArray, zArray, -200); // initialize test 3d object
Utilities utils; // initialize myUtils as utils
//microStepper stepper(A5, A4, A3, A2, 7.5); // Cant use microstepping as the board only has 2 DAC outs :(

    // Mbed stuff, Tickers/Interrupts/Etc.
Ticker nextStep; // used to iterate through object scan
Ticker updateScreen; // Refresh screen with updated view from selected mode, normally 50Hz


//----------------------------Function definitons--------------------------------------------

    // Screen with relevant peripheral data, useful for debugging/testing
void drawDebugScreen(void){
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    char text [50];
    sprintf((char*)text, "IR Distance: %.2f    Voltage: %.2f", IR.getDistance(), IR.readVoltage()); 
    BSP_LCD_ClearStringLine(2);
    BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *)&text, LEFT_MODE);
    float potVal = rangePot.readVoltage();
    sprintf((char*)text, "Range-Pot Distance: %d    Voltage: %.2f", utils.valmap(potVal, 0, 3.3, 0, 100), potVal); 
    BSP_LCD_ClearStringLine(3);
    BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)&text, LEFT_MODE);
    sprintf((char*)text, "Encoder Clockwise: %d", encoder.getClockwise()); 
    BSP_LCD_ClearStringLine(4);
    BSP_LCD_DisplayStringAt(0, LINE(4), (uint8_t *)&text, LEFT_MODE);
    sprintf((char*)text, "Servo Position: %.2f", servo.readPos());
    BSP_LCD_ClearStringLine(5);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&text, LEFT_MODE);
    sprintf((char*)text, "Scan Clockwise: %d    Angle: %d   Layer: %d", direction, desiredAngle, depthMapLayer);
    BSP_LCD_ClearStringLine(6);
    BSP_LCD_DisplayStringAt(0, LINE(6), (uint8_t *)&text, LEFT_MODE);
    sprintf((char*)text, "Radar Offset X: %d   Y: %d", radarXoffset, radarYoffset); 
    BSP_LCD_ClearStringLine(7);
    BSP_LCD_DisplayStringAt(0, LINE(7), (uint8_t *)&text, LEFT_MODE);
    sprintf((char*)text, "DepthMap Offset X: %d   Y: %d", depthMapXoffset, depthMapYoffset); 
    BSP_LCD_ClearStringLine(8);
    BSP_LCD_DisplayStringAt(0, LINE(8), (uint8_t *)&text, LEFT_MODE);
}

    // Select & Execute menu options when button is pressed
void rotaryButtonPressed(void){
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    switch(menuCounter){
        case 0:
            nextStep.attach(incrementScan, 20ms); // 50Hz
            break;
        case 1:
            updateScreen.attach(draw3dObject, 1ms); //limited by speed of calcs anyway
            break;
        case 2:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"BBBBBBBBBBBBB 3", CENTER_MODE);
            break;
        case 3:
            //updateScreen.attach(drawDebugScreen, 20ms); // 50Hz
            break;
        default:    // should never happen
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Something went wrong (rotaryButtonPressed)", CENTER_MODE);
            break;
    }
}

    // Scroll through menu when encoder is turned
void rotaryTurned(void){
    (encoder.getClockwise() == true) ? menuCounter++ : menuCounter--;
    if(menuCounter > 3){
        menuCounter = 0; 
    }else if(menuCounter < 0){
        menuCounter = 3;
    }
    BSP_LCD_ClearStringLine(1);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    switch(menuCounter){
        case 0:
            BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Start Scan", CENTER_MODE);
            break;
        case 1:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Rotate Scanned Object", CENTER_MODE);
            break;
        case 2:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: menu placeholder 33333", CENTER_MODE);
            break;
        case 3:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Debug Mode", CENTER_MODE);
            break;
        default:    // should never happen
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Something went wrong (rotaryTurned)", CENTER_MODE);
            break;
    }
}

    // Draw radar representaion of current scan (X&Z progress along each Y layer)
void drawRadarView(float distance, float angle){ 
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawLine(radarXoffset, radarYoffset, lastX + radarXoffset, lastY + radarYoffset); // overwrite last drawn line in black
    uint16_t x = distance * cos(pi * 2 * (angle + 225) / 360); 
    uint16_t y = distance * sin(pi * 2 * (angle + 225) / 360); // find x,y coords of a point on a circle, of radius 'distance' at angle 'angle'
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawLine(radarXoffset, radarYoffset, x + radarXoffset, y + radarYoffset); // draw a line from radar offset, to x,y + offset
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DrawCircle(x + radarXoffset, y + radarYoffset, 1); // mark the end of the line with a yellow blob
    BSP_LCD_DrawLine(x + radarXoffset, y + radarYoffset, lastX + radarXoffset, lastY + radarYoffset); // connect the blobs
    lastX = x; // store last used x,y values
    lastY = y;
}

    // Draw 2D image with depth data (closer distance = brighter pixel)
void drawDepthMap(float distance, float angle, uint8_t layer, uint8_t range){ 
    uint16_t drawX = (angle) + depthMapXoffset; //0-90deg from left = x coord 0 to 90 from offset //TODO: "normalise" the image so it isnt skewed 
    uint16_t drawY = depthMapYoffset - layer;                                                     //(Z = cos(angle) * distance??? check maths before implementing)
    uint8_t red = utils.valmap(distance,0,range,0xFF,0x00);
    BSP_LCD_DrawPixel(drawX, drawY, utils.argbToHex(0xFF, red, 0x00, 0x00)); //pixel gets redder depending on distance data
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawRect(depthMapXoffset - 1, depthMapYoffset - 91, 91, 91); // draw bounding box for image
}

    // Updates draw functions with sensor/pot data & calls them
void sensorUpdate(float distance, float angle, uint8_t layer, float rangePot){
    rangeCutoff = utils.valmap(rangePot, 0, 3.3, 0, 100);
    if(distance > rangeCutoff){
        distance = rangeCutoff; // Cap distance values, also scales brightness values in function "drawDepthMap"
    }
    char text [50];
    sprintf((char*)text, "Distance: %f Layer: %d MaxRange: %d", distance, layer, rangeCutoff); // Display sensor/peripheral readings
    BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawRadarView(distance, angle);
    drawDepthMap(distance, angle, layer, rangeCutoff);
}

    // Project model defined in xyz arrays onto the LCD 
void draw3dObject(void){
    draw3dFlag = true;
    spin = true; // temporarily here, spin that shit
}

    // Take peripheral reading, then move servo & stepper to next position
void incrementScan(void){
    scanFlag = true;
    spin = false;
}

    // Rotate a cube around all 3 axis, 3d rendering demo for testing/debugging
void rotatingCubeDemo(void){
        // Storing 8 vertices of a cube in xyz arrays
     xArray[0] = -40; yArray[0] = -40; zArray[0] = 40; // front bottom left
     xArray[1] = 40; yArray[1] = -40; zArray[1] = 40; // front bottom right
     xArray[2] = 40; yArray[2] = 40; zArray[2] = 40; // front top right
     xArray[3] = -40; yArray[3] = 40; zArray[3] = 40; // front top left
     xArray[4] = -40; yArray[4] = -40; zArray[4] = -40; // back bottom left
     xArray[5] = 40; yArray[5] = -40; zArray[5] = -40; // back bottom right
     xArray[6] = 40; yArray[6] = 40; zArray[6] = -40; // back top right
     xArray[7] = -40; yArray[7] = 40; zArray[7] = -40; // back top left
        // Save a copy of current xyz arrays that wont be modified by rotation (only first 8 for this demo)
    for(int i = 0; i < 8; i++){
        xArraySAVE[i] = xArray[i];
        yArraySAVE[i] = yArray[i];
        zArraySAVE[i] = zArray[i];
    }
        // Display the cube at every angle from 0 to 360 along an axis
    for(int j = 0; j<361; j++){
        testObject.rotateProjection(j, axisCount);
        testObject.generateProjected();
        BSP_LCD_Clear(LCD_COLOR_BLACK);
        BSP_LCD_SetTextColor(LCD_COLOR_YELLOW); // Front face of Cube
        BSP_LCD_DrawLine(testObject.xProjected[0] + xOffset3d, testObject.yProjected[0] + yOffset3d, testObject.xProjected[1] + xOffset3d, testObject.yProjected[1] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[1] + xOffset3d, testObject.yProjected[1] + yOffset3d, testObject.xProjected[2] + xOffset3d, testObject.yProjected[2] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[2] + xOffset3d, testObject.yProjected[2] + yOffset3d, testObject.xProjected[3] + xOffset3d, testObject.yProjected[3] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[3] + xOffset3d, testObject.yProjected[3] + yOffset3d, testObject.xProjected[0] + xOffset3d, testObject.yProjected[0] + yOffset3d);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE); // Rear face of Cube
        BSP_LCD_DrawLine(testObject.xProjected[4] + xOffset3d, testObject.yProjected[4] + yOffset3d, testObject.xProjected[5] + xOffset3d, testObject.yProjected[5] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[5] + xOffset3d, testObject.yProjected[5] + yOffset3d, testObject.xProjected[6] + xOffset3d, testObject.yProjected[6] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[6] + xOffset3d, testObject.yProjected[6] + yOffset3d, testObject.xProjected[7] + xOffset3d, testObject.yProjected[7] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[7] + xOffset3d, testObject.yProjected[7] + yOffset3d, testObject.xProjected[4] + xOffset3d, testObject.yProjected[4] + yOffset3d);
        BSP_LCD_SetTextColor(LCD_COLOR_RED); // Edges connecting front and rear faces of Cube
        BSP_LCD_DrawLine(testObject.xProjected[7] + xOffset3d, testObject.yProjected[7] + yOffset3d, testObject.xProjected[3] + xOffset3d, testObject.yProjected[3] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[6] + xOffset3d, testObject.yProjected[6] + yOffset3d, testObject.xProjected[2] + xOffset3d, testObject.yProjected[2] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[4] + xOffset3d, testObject.yProjected[4] + yOffset3d, testObject.xProjected[0] + xOffset3d, testObject.yProjected[0] + yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[5] + xOffset3d, testObject.yProjected[5] + yOffset3d, testObject.xProjected[1] + xOffset3d, testObject.yProjected[1] + yOffset3d);
            // Restore xyz data from save
        for(int i = 0; i < 8; i++){
            xArray[i] = xArraySAVE[i]; 
            yArray[i] = yArraySAVE[i];
            zArray[i] = zArraySAVE[i];
        }
    }
    axisCount++; // Cycle axis of rotation every full rotation
    if(axisCount > 2){
        axisCount = 0;
        updateScreen.detach();
    }
}


//----------------------------Main stuff----------------------------------------------------------

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

    updateScreen.attach(rotatingCubeDemo, 1ms);

    // Do nothing here until a flag is set
    while(1) {
            // Scanning Routine, progress one step (causes mutex if in ISR)
        if(scanFlag == true){
            sensorUpdate(IR.getDistance(), desiredAngle, depthMapLayer, rangePot.readVoltage());
            (direction == true) ? desiredAngle++ : desiredAngle--;
            if(desiredAngle >= 90 || desiredAngle < 0){
                direction = !direction;
                lastX = 0, lastY = 0;
                depthMapLayer++;
                depthMapLayer++;
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, 22, 300, 250);// clear radar view between layers
                if(depthMapLayer > 90){ // 90x90 scan complete
                    nextStep.detach();
                    depthMapLayer = 0;
                    pixelIndex = 0;
                    for(int i = 0; i < 8100; i++){ // Save a copy of current xyz arrays that wont be modified by rotation
                        xArraySAVE[i] = xArray[i];
                        yArraySAVE[i] = yArray[i];
                        zArraySAVE[i] = zArray[i];
                    }
                    draw3dFlag = true;
                }
            }
            if(desiredAngle %4 == 0){   // temporary, stepper step size is too big. TODO: use microstepper (need power supply)
            stepper.step(direction, 1, 7);
            }
            servo.writePos((float)depthMapLayer);
            yArray[pixelIndex] = -45 + depthMapLayer; // Store xyz Coordinates
            xArray[pixelIndex] = -45 + desiredAngle;
            zArray[pixelIndex] = (int16_t)(rangeCutoff / 2) - (int16_t)round(IR.lastDistance());
            if(IR.lastDistance() >= rangeCutoff){
                zArray[pixelIndex] = -(int16_t)(rangeCutoff / 2);
            }
            pixelIndex++;
            draw3dFlag = true; // maybe temporary? draws 3d object as it is scanned
            scanFlag = false;
        }

            // Draw object in 3d (dont want this in ISR, lots of operations)
        if(draw3dFlag == true){
            if(spin == true){
                for(int j = 0; j<361; j++){
                    testObject.rotateProjection(j, axisCount);
                    testObject.generateProjected();
                    BSP_LCD_Clear(LCD_COLOR_BLACK);
                    for(int i = 0; i < 8099; i++){ // Connect each projected point to its neighbour
                        BSP_LCD_DrawLine(testObject.xProjected[i] +xOffset3d, testObject.yProjected[i] +yOffset3d, testObject.xProjected[i+1] +xOffset3d,testObject.yProjected[i+1] +yOffset3d);
                    }
                    for(int i = 0; i < 8100; i++){ // Restore xyz data from save
                        xArray[i] = xArraySAVE[i];
                        yArray[i] = yArraySAVE[i];
                        zArray[i] = zArraySAVE[i];
                    }
                }
                axisCount++; // Cycle axis of rotation every full rotation
                if(axisCount > 2){
                    axisCount = 0;
                    updateScreen.detach();
                }
            }else{
                testObject.generateProjected();
                //BSP_LCD_Clear(LCD_COLOR_BLACK);
                BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
                for(int i = 0; i < 8099; i++){ // Connect each projected point to its neighbour
                    BSP_LCD_DrawLine(testObject.xProjected[i] +xOffset3d, testObject.yProjected[i] +yOffset3d, testObject.xProjected[i+1] +xOffset3d,testObject.yProjected[i+1] +yOffset3d);
                }
            }
            draw3dFlag = false;
        }
    }
}

// stepper angle is 3.6 in half step, 7.2 in full step //ish, cant find data online
// TODO:
// https://github.com/cbm80amiga/ST7735_3d_filled_vector/blob/master/gfx3d.h

//TODO: test with microstepper