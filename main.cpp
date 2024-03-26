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
#include "myButtons.h"

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
void drawDebugCube(void);
void manualRotation(void);

    // General consensus online: multiple 1D arrays are faster than a multi-dimensional array, save for minimal gains in edge cases.
    // Decision: Prioritize readibility and use 1D array approach
    // XYZ values for 3d render are stored/modified in these arrays
float xArray[8100];       
float yArray[8100];       // Current working set of xyz coordinates
float zArray[8100];
float xArraySAVE[8100];
float yArraySAVE[8100];   // Used to save xyz values. It's a "restore point".
float zArraySAVE[8100];
uint16_t pixelIndex = 0;  // Current xyzArray index to write/read

    // Offsets/Coordinates for drawing on LCD
uint8_t xOffset3d = 240;        // Offsets for drawing 3D object centred on (0,0) which is at the top left of the LCD
uint8_t yOffset3d = 120;
uint16_t radarXoffset = 110;    // Position offsets for drawing radar view
uint16_t radarYoffset = 222;
uint16_t depthMapXoffset = 310; // Position offsets for drawing depth map
uint16_t depthMapYoffset = 222;
uint16_t lastX; // Coordinates of last drawn line for radar view, used to overwrite in black without clearing entire radar
uint16_t lastY; // purely cosmetic

    // Obtaining XYZ values via scan
bool direction = false;     // Scan direction: False = CCW(-X), True = CW(+X) 
int16_t desiredAngle = 0;   // Scan/Stepper angle (Horizontal/X Axis)
uint8_t depthMapLayer = 0;  // Y value/layer of depthmap, also used for servo angle (Vertical/Y Axis)
uint8_t rangeCutoff = 100;  // Any distances sensed past this value, are capped to this value (Depth/Z Axis)

    // Flags, mostly for executing functions incompatible with ISR (mutex, too slow, etc.)
bool scanFlag = false;          // Scan object flag, progress through 3D scan when this flag is set.
bool draw3dFlag = false;        // Draw 3D object when this flag is set.
bool spin = false;              // Rotate 3D object automatically when this flag is set.
bool rotateTouchFlag = false;   // Enable touch buttons flag.
bool loadTestCube = true;       // Until this flag is un-set, 3D renderer draws a tri-coloured cube.

    // Menu navigation/control
int8_t menuCounter = 0;    // Used to store last/select a menu option via rotary encoder
int16_t xRotateIndex = 0;
int16_t yRotateIndex = 0;  // Angle indexes for manual object rotation //UNUSED?
int16_t zRotateIndex = 0;

    // Misc.
int rotationAxis = 0; // Axis of rotation for 3D objects
const double pi = 3.14159265359; // nom nom nom


    // Initialization, Classes/Objects/Structs/Etc.
irSense IR(A0); // initialize IR sensor, reading from Pin A0
Pot rangePot(A1); // initialize Potentiometer, reading from Pin A1
Servo servo(PC_7, 180, 2.5, 1.5); // initialize Servo motor, on pin PC_7 (D0), with a 90 degree range between 1.5 and 2ms.
Stepper stepper(D1, D2, D3, D4, 7.5); // initialize Stepper motor, on pins D1, D2, D3, D4, with a step angle of 7.5 (not yet implemented)
Rotary encoder(D5, D6, D7, &rotaryButtonPressed, &rotaryTurned); // initialize Rotary encoder on D5,D6,D7, and pass functions to object
Object3d testObject(xArray, yArray, zArray, -200); // initialize test 3d object
Utilities utils; // initialize myUtils as utils
//microStepper stepper(A5, A4, A3, A2, 7.5); // Cant use microstepping as the board only has 2 DAC outs :(

    // Touchscreen buttons
TS_StateTypeDef touchState;
Button xIncrease(420, 460, 52, 92, LCD_COLOR_RED, LCD_COLOR_YELLOW, 1, touchState);         // Rotate around z axis
Button xDecrease(420, 460, 180, 220, LCD_COLOR_RED, LCD_COLOR_YELLOW, 2, touchState);
Button yDecrease(140, 180, 225, 265, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, 3, touchState);     // Rotate around y axis.
Button yIncrease(300, 340, 225, 265, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, 4, touchState);
Button zIncrease(20, 60, 52, 92, LCD_COLOR_BLUE, LCD_COLOR_YELLOW, 5, touchState);          // Rotate around z axis.
Button zDecrease(20, 60, 180, 220, LCD_COLOR_BLUE, LCD_COLOR_YELLOW, 6, touchState);
Button resetObject(350, 470, 230, 262, LCD_COLOR_MAGENTA, LCD_COLOR_CYAN, 0, touchState);   // Restore save.
Button fovDecrease(75, 130, 230, 262, LCD_COLOR_DARKCYAN, LCD_COLOR_WHITE, 0, touchState);  // fov-, Increases focal length.
Button fovIncrease(10, 65, 230, 262, LCD_COLOR_DARKBLUE, LCD_COLOR_WHITE, 0, touchState);   // fov+, Decreases focal length.
Button slideReset(10, 65, 10, 42, LCD_COLOR_LIGHTMAGENTA, LCD_COLOR_WHITE, 0, touchState);
    // Mbed stuff, Tickers/Interrupts/Etc.
Ticker nextStep;     // used to iterate through object scan
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
    updateScreen.detach();
    nextStep.detach();
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    switch(menuCounter){
        case 0:
                // Attatch ticker to this flag. Increments scan progress
            nextStep.attach(incrementScan, 20ms); // 50Hz
            break;
        case 1:
                // Attatch ticker to this flag. Redraws Object
            updateScreen.attach(draw3dObject, 20ms); // 50Hz
            break;
        case 2:
                // Restore 3D object from save
            for(int i = 0; i < 8; i++){
                xArray[i] = xArraySAVE[i]; 
                yArray[i] = yArraySAVE[i];
                zArray[i] = zArraySAVE[i];
            }
                // Draw buttons
            xIncrease.drawButton(); xDecrease.drawButton();
            yIncrease.drawButton(); yDecrease.drawButton();
            zIncrease.drawButton(); zDecrease.drawButton();
            fovIncrease.drawButton(); fovDecrease.drawButton();
            resetObject.drawButton();
            BSP_LCD_SetFont(&Font16);
            BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
            BSP_LCD_DisplayStringAt(80, 241, (uint8_t*)"fov-", LEFT_MODE);
            BSP_LCD_DisplayStringAt(15, 241, (uint8_t*)"fov+", LEFT_MODE);
            BSP_LCD_DisplayStringAt(365, 241, (uint8_t*)"reset", LEFT_MODE);
            BSP_LCD_SetFont(&Font12);
                // Attatch ticker to this flag. Rotates and draws based on ts input
            updateScreen.attach(manualRotation, 1ms); // 50Hz
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
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Manual Object Rotation", CENTER_MODE);
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

    // Draw 2D image with depth data. Closer distance = brighter pixel
void drawDepthMap(float distance, float angle, uint8_t layer, uint8_t range){ 
    uint16_t drawX = (angle) + depthMapXoffset; //0-90deg from left = x coord 0 to 90 from offset //TODO: "normalise" the image so it isnt skewed 
    uint16_t drawY = depthMapYoffset - layer;                                                     //(Z = cos(angle) * distance??? check maths before implementing)
    uint8_t red = utils.valmap(distance,0,range,0xFF,0x00);
    BSP_LCD_DrawPixel(drawX, drawY, utils.argbToHex(0xFF, red, 0x00, 0x00));
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        // Draw bounding box
    BSP_LCD_DrawRect(depthMapXoffset - 1, depthMapYoffset - 91, 91, 91);
}

    // Updates draw functions with sensor/pot data & calls them
void sensorUpdate(float distance, float angle, uint8_t layer, float rangePot){
        // Cap distance values via pot. Scales brightness in function "drawDepthMap"
    rangeCutoff = utils.valmap(rangePot, 0, 3.3, 0, 100);
    if(distance > rangeCutoff) {distance = rangeCutoff; }
    char text [50];
         // Clear line. Display scan progress. Draw 2D views
    sprintf((char*)text, "Distance: %f Layer: %d MaxRange: %d", distance, layer, rangeCutoff);
    BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawRadarView(distance, angle);
    drawDepthMap(distance, angle, layer, rangeCutoff);
}

    // Project model defined in xyz arrays onto the LCD 
void draw3dObject(void){
    draw3dFlag = true;
    spin = false;
    rotateTouchFlag = false;
    loadTestCube = false;
}

    // Take peripheral reading, then move servo & stepper to next position
void incrementScan(void){
    scanFlag = true;
    spin = false;
    rotateTouchFlag = false;
    loadTestCube = false;
}

    // Rotate and draw object. Based on ts input.
void manualRotation(void){
    rotateTouchFlag = true;
    spin = false;
    scanFlag = false;
    draw3dFlag = false;
}

    // Rotate a cube around all 3 axis, 3d rendering demo for testing/debugging
void rotatingCubeDemo(void){
        // Storing 8 vertices of a cube in xyz arrays
     xArray[0] = -40; yArray[0] = -40; zArray[0] = 40;  // front bottom left
     xArray[1] = 40; yArray[1] = -40; zArray[1] = 40;   // front bottom right
     xArray[2] = 40; yArray[2] = 40; zArray[2] = 40;    // front top right
     xArray[3] = -40; yArray[3] = 40; zArray[3] = 40;   // front top left
     xArray[4] = -40; yArray[4] = -40; zArray[4] = -40; // back bottom left
     xArray[5] = 40; yArray[5] = -40; zArray[5] = -40;  // back bottom right
     xArray[6] = 40; yArray[6] = 40; zArray[6] = -40;   // back top right
     xArray[7] = -40; yArray[7] = 40; zArray[7] = -40;  // back top left
        // Save a copy of current xyz arrays that wont be modified by rotation (only first 8 for this demo)
    for(int i = 0; i < 8; i++){
        xArraySAVE[i] = xArray[i];
        yArraySAVE[i] = yArray[i];
        zArraySAVE[i] = zArray[i];
    }
        // Display the cube at every angle from 0 to 360 along an axis
    for(int j = 0; j<361; j++){
        testObject.rotateProjection(j, rotationAxis);
        testObject.generateProjected();
        BSP_LCD_Clear(LCD_COLOR_BLACK);
        drawDebugCube();
            // Restore xyz data from save
        for(int i = 0; i < 8; i++){
            xArray[i] = xArraySAVE[i]; 
            yArray[i] = yArraySAVE[i];
            zArray[i] = zArraySAVE[i];
        }
    }
        // Cycle axis of rotation every full rotation
    rotationAxis++;
    if(rotationAxis > 2){
        rotationAxis = 0;
        updateScreen.detach();
    }
}

    // Draw whatever is in the first 8 indexes, as if it were a debug cube. Development tool.
void drawDebugCube(void){
        BSP_LCD_SetTextColor(LCD_COLOR_RED);    // Edges connecting front and rear faces
        BSP_LCD_DrawLine(testObject.xProjected[7]+xOffset3d, testObject.yProjected[7]+yOffset3d, testObject.xProjected[3]+xOffset3d, testObject.yProjected[3]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[6]+xOffset3d, testObject.yProjected[6]+yOffset3d, testObject.xProjected[2]+xOffset3d, testObject.yProjected[2]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[4]+xOffset3d, testObject.yProjected[4]+yOffset3d, testObject.xProjected[0]+xOffset3d, testObject.yProjected[0]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[5]+xOffset3d, testObject.yProjected[5]+yOffset3d, testObject.xProjected[1]+xOffset3d, testObject.yProjected[1]+yOffset3d);
        BSP_LCD_SetTextColor(LCD_COLOR_YELLOW); // Front face
        BSP_LCD_DrawLine(testObject.xProjected[0]+xOffset3d, testObject.yProjected[0]+yOffset3d, testObject.xProjected[1]+xOffset3d, testObject.yProjected[1]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[1]+xOffset3d, testObject.yProjected[1]+yOffset3d, testObject.xProjected[2]+xOffset3d, testObject.yProjected[2]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[2]+xOffset3d, testObject.yProjected[2]+yOffset3d, testObject.xProjected[3]+xOffset3d, testObject.yProjected[3]+ yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[3]+xOffset3d, testObject.yProjected[3]+yOffset3d, testObject.xProjected[0]+xOffset3d, testObject.yProjected[0]+yOffset3d);
        BSP_LCD_SetTextColor(LCD_COLOR_BLUE);   // Rear face
        BSP_LCD_DrawLine(testObject.xProjected[4]+xOffset3d, testObject.yProjected[4]+yOffset3d, testObject.xProjected[5]+xOffset3d, testObject.yProjected[5]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[5]+xOffset3d, testObject.yProjected[5]+yOffset3d, testObject.xProjected[6]+xOffset3d, testObject.yProjected[6]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[6]+xOffset3d, testObject.yProjected[6]+yOffset3d, testObject.xProjected[7]+xOffset3d, testObject.yProjected[7]+yOffset3d);
        BSP_LCD_DrawLine(testObject.xProjected[7]+xOffset3d, testObject.yProjected[7]+yOffset3d, testObject.xProjected[4]+xOffset3d, testObject.yProjected[4]+yOffset3d);    
}


//----------------------------Main stuff----------------------------------------------------------

    // Main
int main(){
    
        // Setup LCD, Show startup message, Clear to black
    BSP_LCD_Init();
    BSP_LCD_SetXSize(480); BSP_LCD_SetYSize(272);
    BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
    BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"14074479 MAOUDISA", CENTER_MODE);
    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_Clear(LCD_COLOR_BLACK);

        // Initialize touchscreen
    BSP_TS_Init(480, 272);

        // Attatch ticker to this flag. Using this demo as a splash screen
    updateScreen.attach(rotatingCubeDemo, 1ms); // 50Hz


    // Do nothing here until a flag is set
    while(1) {

            // Manual control over 3D render (Slow)
        if(rotateTouchFlag == true){
                // Rotate or reset according to buttons pressed
            if(fovIncrease.isPressed()) {testObject._focalLength++;}                  // Note: yes. if-elseif-else is faster here. (stops comparisons when true).
            if(fovDecrease.isPressed()) {testObject._focalLength--;}                 // using if-if-if to support multiple simultaneous button presses.
            if(xIncrease.isPressed()) {testObject.rotateProjection(1, 0);}
            if(xDecrease.isPressed()) {testObject.rotateProjection(-1, 0);}
            if(yIncrease.isPressed()) {testObject.rotateProjection(1, 1);}
            if(yDecrease.isPressed()) {testObject.rotateProjection(-1, 1);}
            if(zIncrease.isPressed()) {testObject.rotateProjection(1, 2);}
            if(zDecrease.isPressed()) {testObject.rotateProjection(-1, 2);}
            if(resetObject.isPressed()){
                for(int i = 0; i < 8; i++){
                    xArray[i] = xArraySAVE[i]; 
                    yArray[i] = yArraySAVE[i];
                    zArray[i] = zArraySAVE[i];
                }
            }
                // Generate coordinates, Clear Object, Draw image. (Only clear immidiately before drawing to reduce strobing)
                // Buttons are not redrawn, But also not cleared. Faster.
            testObject.generateProjected();
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(80, 50, 320, 155);
            BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
            BSP_LCD_DrawRect(79, 49, 321, 156);
            if(loadTestCube == true){
                drawDebugCube();
            }else{
                for(int i = 0; i < 8099; i++){
                    BSP_LCD_DrawLine(testObject.xProjected[i] +xOffset3d, testObject.yProjected[i] +yOffset3d, testObject.xProjected[i+1] +xOffset3d,testObject.yProjected[i+1] +yOffset3d);
                }
            }
            rotateTouchFlag = false;
        }

            // Scanning Routine, progress one step (causes mutex if in ISR)
        if(scanFlag == true){
                // Update peripheral data. Clear lcd between layers
            sensorUpdate(IR.getDistance(), desiredAngle, depthMapLayer, rangePot.readVoltage());
            (direction == true) ? desiredAngle++ : desiredAngle--;
            if(desiredAngle >= 90 || desiredAngle < 0){
                direction = !direction;
                lastX = 0, lastY = 0;
                depthMapLayer++;
                depthMapLayer++;
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, 22, 300, 250);
                    // Check for scan completion. Detatch ticker & save xyz arrays
                if(depthMapLayer > 90){
                    nextStep.detach();
                    depthMapLayer = 0;
                    pixelIndex = 0;
                    for(int i = 0; i < 8100; i++){ 
                        xArraySAVE[i] = xArray[i];
                        yArraySAVE[i] = yArray[i];
                        zArraySAVE[i] = zArray[i];
                    }
                    draw3dFlag = true;
                }
            }
                // Move stepper and servo. Store xyz Coordinates
            if(desiredAngle %4 == 0) {stepper.step(direction, 1, 7);} // Temporary? Uni only has crap steppers. (step angle too large, even with half step)
            servo.writePos((float)depthMapLayer);
            yArray[pixelIndex] = -45 + depthMapLayer;
            xArray[pixelIndex] = -45 + desiredAngle;
            zArray[pixelIndex] = (int16_t)(rangeCutoff / 2) - (int16_t)round(IR.lastDistance());
            if(IR.lastDistance() >= rangeCutoff) {zArray[pixelIndex] = -(int16_t)(rangeCutoff / 2);}
            pixelIndex++;
            draw3dFlag = true; // Maybe temporary? Draws 3d object as it is scanned if true
            scanFlag = false;
        }

            // Draw object in 3d (Dont want this in ISR, lots of operations)
        if(draw3dFlag == true){
            if(spin == true){
                for(int j = 0; j<361; j++){
                    testObject.rotateProjection(j, rotationAxis);
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
                rotationAxis++; // Cycle axis of rotation every full rotation
                if(rotationAxis > 2){
                    rotationAxis = 0;
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
