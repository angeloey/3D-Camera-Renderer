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
void sensorUpdate(float distance, float angle, uint8_t layer, float RangePot);
void rotaryButtonPressed(void);
void rotaryTurned(void);
void draw3dObject(void);
void rotatingCubeDemo(void);
void drawDebugCube(void);
void manualRotation(void);

    // Offsets/Coordinates for drawing on LCD
uint8_t xOffset3d = 240;        // Offsets for drawing 3D object centred on (0,0) which is at the top left of the LCD.
uint8_t yOffset3d = 127;
uint16_t radarXoffset = 110;    // Position offsets for drawing radar view.
uint16_t radarYoffset = 202;
uint16_t depthMapXoffset = 310; // Position offsets for drawing depth map.
uint16_t depthMapYoffset = 202;
uint16_t lastX;                 // Coordinates of last drawn line for radar view, used to overwrite in black without clearing entire radar.
uint16_t lastY;                 // purely cosmetic.

    // Obtaining XYZ values via scan
bool direction = false;         // Scan direction: False = CCW(-X), True = CW(+X) 
int16_t desiredAngle = 0;       // Scan/Stepper angle (Horizontal/X Axis)
uint8_t depthMapLayer = 0;      // Y value/layer of depthmap, also used for Servo angle (Vertical/Y Axis)
uint8_t rangeCutoff = 100;      // Any distances sensed past this value, are capped to this value (Depth/Z Axis)

    // Flags, mostly for executing functions incompatible with ISR (mutex, too slow, etc.)
bool scanFlag = false;          // Scan object flag, progress through 3D scan when this flag is set.
bool draw3dFlag = false;        // Draw 3D object when this flag is set.
bool spin = false;              // Rotate 3D object automatically when this flag is set.
bool rotateTouchFlag = false;   // Enable touch buttons flag.
bool loadTestCube = true;       // Until this flag is un-set, 3D renderer draws a tri-coloured cube.
bool drawDebugFlag = false;     // Draw Debug-Screen. Peripheral data, etc.

    // Menu navigation/control
int8_t menuCounter = 0;         // Used to store last/select a menu option via rotary Encoder.
uint8_t rotationAxis = 0;       // Axis of rotation for 3D objects. 0, 1, 2 == X, Y, Z.
uint16_t pixelIndex = 0;        // Current xyz index to write/read.

    // Misc.
uint32_t drawColour = LCD_COLOR_YELLOW; // This is modified by the pink colour slider. Default setting is yellow.
uint8_t red;                            
uint8_t green;          // RGB values, used to cycle through RGB spectrum via slider.
uint8_t blue;           
Utilities utils;        // Utilities class. Frequently used, non program-specific functions .


    // Initialization, Peripheral Objects/Structs.
IrSense IR(A0);                                     // initialize IR sensor, reading from Pin A0
Pot RangePot(A1);                                   // initialize Potentiometer, reading from Pin A1
Servo Servo(D0, 180, 2.5, 1.5);                     // initialize Servo motor, on pin D0, with a 180 degree range between 1.5 and 2ms.
Stepper Stepper(D1, D2, D3, D4, 7.5);               // initialize Stepper motor, on pins D1, D2, D3, D4, with a step angle of 7.5
Rotary Encoder(D5, D6, D7, &rotaryButtonPressed, &rotaryTurned);    // initialize Rotary Encoder on D5,D6,D7, and pass functions to object
//microStepper Stepper(A5, A4, A3, A2, 7.5);        // Cant use microstepping as the board only has 2 DAC outs :(
Object3D Render3D(-200);  // initialize 3D Object
TS_StateTypeDef touchState;                         // Touchscreen-state Struct


    // Touchscreen buttons
Button xIncrease(420, 460, 52, 92, LCD_COLOR_RED, LCD_COLOR_YELLOW, 1, touchState);         // Rotate around z axis
Button xDecrease(420, 460, 180, 220, LCD_COLOR_RED, LCD_COLOR_YELLOW, 2, touchState);
Button yDecrease(140, 180, 225, 265, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, 3, touchState);     // Rotate around y axis.
Button yIncrease(300, 340, 225, 265, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, 4, touchState);
Button zIncrease(20, 60, 52, 92, LCD_COLOR_BLUE, LCD_COLOR_YELLOW, 5, touchState);          // Rotate around z axis.
Button zDecrease(20, 60, 180, 220, LCD_COLOR_BLUE, LCD_COLOR_YELLOW, 6, touchState);
Button resetObject(350, 470, 230, 262, LCD_COLOR_MAGENTA, LCD_COLOR_CYAN, 0, touchState);   // Restore save.
Button fovDecrease(75, 130, 230, 262, LCD_COLOR_DARKCYAN, LCD_COLOR_WHITE, 0, touchState);  // fov-, Increases focal length.
Button fovIncrease(10, 65, 230, 262, LCD_COLOR_DARKBLUE, LCD_COLOR_WHITE, 0, touchState);   // fov+, Decreases focal length.
Slider slideColour(10, 65, 10, 42, LCD_COLOR_LIGHTMAGENTA, LCD_COLOR_WHITE, 0, touchState, true, 30, 450);

    // Mbed stuff, Tickers/Interrupts/Etc.
Ticker nextStep;     // used to iterate through object scan
Ticker updateScreen; // Refresh screen with updated view from selected mode, normally 50Hz

//----------------------------Function definitons--------------------------------------------

    // Select & Execute menu options when button is pressed // Triggered by interrupts in rotary lib
void rotaryButtonPressed(void){
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    switch(menuCounter){
        case 0:
            updateScreen.detach();
            nextStep.detach();
                // Attatch ticker to this flag. Increments scan progress
            nextStep.attach(incrementScan, 20ms); // 50Hz
            break;
        case 1:
            updateScreen.detach();
            nextStep.detach();
                // Attatch ticker to this flag. Redraws Object
            updateScreen.attach(draw3dObject, 20ms); // 50Hz
            spin = true;
            break;
        case 2:
            updateScreen.detach();
            nextStep.detach();
                // Restore 3D object from save
            Render3D.restoreSave();
                // Draw buttons
            xIncrease.drawButton(); xDecrease.drawButton();
            yIncrease.drawButton(); yDecrease.drawButton();
            zIncrease.drawButton(); zDecrease.drawButton();
            fovIncrease.drawButton(); fovDecrease.drawButton();
            resetObject.drawButton();
            slideColour.drawButton();
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
            drawDebugFlag = !drawDebugFlag;
            break;
        default:    // should never happen
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Something went wrong (rotaryButtonPressed)", CENTER_MODE);
            break;
    }
}

    // Scroll through menu when Encoder is turned
void rotaryTurned(void){
    (Encoder.getClockwise() == true) ? menuCounter++ : menuCounter--;
    if(menuCounter > 3){
        menuCounter = 0; 
    }else if(menuCounter < 0){
        menuCounter = 3;
    }
    BSP_LCD_ClearStringLine(1);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    switch(menuCounter){
        case 0:
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
    uint16_t x = distance * cos(constants::PI_DOUBLE * 2 * (angle + 225) / 360); 
    uint16_t y = distance * sin(constants::PI_DOUBLE * 2 * (angle + 225) / 360); // find x,y coords of a point on a circle, of radius 'distance' at angle 'angle'
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
        // Draw bounding box
    BSP_LCD_SetTextColor(drawColour);
    BSP_LCD_DrawRect(depthMapXoffset - 1, depthMapYoffset - 91, 91, 91);
}

    // Updates draw functions with sensor/pot data & calls them
void sensorUpdate(float distance, float angle, uint8_t layer, float RangePot){
        // Cap distance values via pot. Scales brightness in function "drawDepthMap"
    rangeCutoff = utils.valmap(RangePot, 0, 3.3, 0, 100);
    if(distance > rangeCutoff) {distance = rangeCutoff; }
    char text [50];
         // Clear line. Display scan progress. Draw 2D views
    sprintf((char*)text, "Distance: %f Layer: %d MaxRange: %d", distance, layer, rangeCutoff);
    BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawRadarView(distance, angle);
    drawDepthMap(distance, angle, layer, rangeCutoff);
}

    // Rotate a cube around all 3 axis, 3d rendering demo for testing/debugging
void rotatingCubeDemo(void){
        // Storing 8 vertices of a cube in render object.
    Render3D.vertices.x[0] = -40; Render3D.vertices.y[0] = -40; Render3D.vertices.z[0] = 40;  // front bottom left
    Render3D.vertices.x[1] = 40; Render3D.vertices.y[1] = -40; Render3D.vertices.z[1] = 40;   // front bottom right
    Render3D.vertices.x[2] = 40; Render3D.vertices.y[2] = 40; Render3D.vertices.z[2] = 40;    // front top right
    Render3D.vertices.x[3] = -40; Render3D.vertices.y[3] = 40; Render3D.vertices.z[3] = 40;   // front top left
    Render3D.vertices.x[4] = -40; Render3D.vertices.y[4] = -40; Render3D.vertices.z[4] = -40; // back bottom left
    Render3D.vertices.x[5] = 40; Render3D.vertices.y[5] = -40; Render3D.vertices.z[5] = -40;  // back bottom right
    Render3D.vertices.x[6] = 40; Render3D.vertices.y[6] = 40; Render3D.vertices.z[6] = -40;   // back top right
    Render3D.vertices.x[7] = -40; Render3D.vertices.y[7] = 40; Render3D.vertices.z[7] = -40;  // back top left
        // Save a copy of current vertices that wont be modified by rotation.
    Render3D.saveVertices();
        // Display the cube at every angle from 0 to 360 along an axis.
    for(int j = 0; j<361; j++){
        Render3D.rotateVertices(j, rotationAxis);
        Render3D.generateProjected();
        drawDebugCube();
            // Restore vertex data from save.
        Render3D.restoreSave();
    }
        // Cycle axis of rotation every full rotation.
    rotationAxis++;
    if(rotationAxis > 2){
        rotationAxis = 0;
        updateScreen.detach();
    }
}

    // Draw whatever is in the first 8 indexes, as if it were a debug cube. Development tool.
void drawDebugCube(void){
    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(80, 50, 320, 155);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);    // Edges connecting front and rear faces
    BSP_LCD_DrawLine(Render3D.xProjected[7]+xOffset3d, Render3D.yProjected[7]+yOffset3d, Render3D.xProjected[3]+xOffset3d, Render3D.yProjected[3]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[6]+xOffset3d, Render3D.yProjected[6]+yOffset3d, Render3D.xProjected[2]+xOffset3d, Render3D.yProjected[2]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[4]+xOffset3d, Render3D.yProjected[4]+yOffset3d, Render3D.xProjected[0]+xOffset3d, Render3D.yProjected[0]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[5]+xOffset3d, Render3D.yProjected[5]+yOffset3d, Render3D.xProjected[1]+xOffset3d, Render3D.yProjected[1]+yOffset3d);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW); // Front face
    BSP_LCD_DrawLine(Render3D.xProjected[0]+xOffset3d, Render3D.yProjected[0]+yOffset3d, Render3D.xProjected[1]+xOffset3d, Render3D.yProjected[1]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[1]+xOffset3d, Render3D.yProjected[1]+yOffset3d, Render3D.xProjected[2]+xOffset3d, Render3D.yProjected[2]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[2]+xOffset3d, Render3D.yProjected[2]+yOffset3d, Render3D.xProjected[3]+xOffset3d, Render3D.yProjected[3]+ yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[3]+xOffset3d, Render3D.yProjected[3]+yOffset3d, Render3D.xProjected[0]+xOffset3d, Render3D.yProjected[0]+yOffset3d);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);   // Rear face
    BSP_LCD_DrawLine(Render3D.xProjected[4]+xOffset3d, Render3D.yProjected[4]+yOffset3d, Render3D.xProjected[5]+xOffset3d, Render3D.yProjected[5]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[5]+xOffset3d, Render3D.yProjected[5]+yOffset3d, Render3D.xProjected[6]+xOffset3d, Render3D.yProjected[6]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[6]+xOffset3d, Render3D.yProjected[6]+yOffset3d, Render3D.xProjected[7]+xOffset3d, Render3D.yProjected[7]+yOffset3d);
    BSP_LCD_DrawLine(Render3D.xProjected[7]+xOffset3d, Render3D.yProjected[7]+yOffset3d, Render3D.xProjected[4]+xOffset3d, Render3D.yProjected[4]+yOffset3d);    
}


// ---------------------Flags set by Interrupts-------------------------------------------------------------------------
    // Project model defined in xyz arrays onto the LCD 
void draw3dObject(void){
    draw3dFlag = true;
    rotateTouchFlag = false;
    loadTestCube = false;
}

    // Take peripheral reading, then move Servo & Stepper to next position
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
            if(fovIncrease.isPressed()) {Render3D._focalLength+=1;}                 // Note: yes. if-elseif-else is faster here. (stops comparisons when true).
            if(fovDecrease.isPressed()) {Render3D._focalLength-=1;}                 // using if-if-if to support multiple simultaneous button presses.
            if(xIncrease.isPressed()) {Render3D.rotateVertices(1, 0);}
            if(xDecrease.isPressed()) {Render3D.rotateVertices(-1, 0);}
            if(yIncrease.isPressed()) {Render3D.rotateVertices(1, 1);}
            if(yDecrease.isPressed()) {Render3D.rotateVertices(-1, 1);}
            if(zIncrease.isPressed()) {Render3D.rotateVertices(1, 2);}
            if(zDecrease.isPressed()) {Render3D.rotateVertices(-1, 2);}
            if(resetObject.isPressed()){
                loadTestCube = false;
                Render3D.restoreSave();
            }
                // Choose object colour via slider.
            slideColour.isPressed();
            if(slideColour._sliderOut <= 33){
                red = utils.valmap(slideColour._sliderOut, 0, 33, 0, 255);
                green = utils.valmap(slideColour._sliderOut, 0, 33, 255, 0);
                drawColour = utils.argbToHex(0xFF, red, green, blue);
            }else if(slideColour._sliderOut <= 66){
                green = utils.valmap(slideColour._sliderOut, 0, 33, 0, 255);
                blue = utils.valmap(slideColour._sliderOut, 0, 33, 255, 0);
                drawColour = utils.argbToHex(0xFF, red, green, blue);
            }else if(slideColour._sliderOut <= 100){
                blue = utils.valmap(slideColour._sliderOut, 0, 33, 0, 255);
                red = utils.valmap(slideColour._sliderOut, 0, 33, 255, 0);
                drawColour = utils.argbToHex(0xFF, red, green, blue);
            }
                // Generate coordinates, Clear Object, Draw image. (Only clear immidiately before drawing to reduce strobing)
                // Buttons are not redrawn, But also not cleared. Faster. (Exception > Sliders)
            Render3D.generateProjected();
            while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
            BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
            BSP_LCD_DrawRect(79, 49, 321, 156);
            if(loadTestCube == true){
                drawDebugCube();
            }else{
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(80, 50, 320, 155);
                BSP_LCD_SetTextColor(drawColour);
                for(int i = 0; i < 8099; i++){
                    BSP_LCD_DrawLine(Render3D.xProjected[i] +xOffset3d, Render3D.yProjected[i] +yOffset3d, Render3D.xProjected[i+1] +xOffset3d,Render3D.yProjected[i+1] +yOffset3d);
                }
            }
            rotateTouchFlag = false;
        }

            // Scanning Routine, progress one step (causes mutex if in ISR)
        if(scanFlag == true){
                // Update peripheral data. Clear lcd between layers
            sensorUpdate(IR.getDistance(), desiredAngle, depthMapLayer, RangePot.readVoltage());
            (direction == true) ? desiredAngle++ : desiredAngle--;
            if(desiredAngle >= 90 || desiredAngle < 0){
                direction = !direction;
                lastX = 0, lastY = 0;
                depthMapLayer++;
                depthMapLayer++;
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, 22, 300, 250);
                    // Check for scan completion. Detatch ticker & save vertices
                if(depthMapLayer > 90){
                    nextStep.detach();
                    depthMapLayer = 0;
                    pixelIndex = 0;
                    Render3D.saveVertices();
                    draw3dFlag = true;
                }
            }
                // Move Stepper and Servo. Store xyz Coordinates
            if(desiredAngle %4 == 0) {Stepper.step(direction, 1, 7);} // Temporary? Uni only has crap Steppers. (step angle too large, even with half step)
            Servo.writePos((float)depthMapLayer);
            Render3D.vertices.y[pixelIndex] = -45 + depthMapLayer;
            Render3D.vertices.x[pixelIndex] = -45 + desiredAngle;
            Render3D.vertices.z[pixelIndex] = (int16_t)(rangeCutoff / 2) - (int16_t)round(IR.lastDistance());
            if(IR.lastDistance() >= rangeCutoff) {Render3D.vertices.z[pixelIndex] = -(int16_t)(rangeCutoff / 2);}
            pixelIndex++;
            draw3dFlag = true; // Maybe temporary? Draws 3d object as it is scanned if true
            scanFlag = false;
        }

            // Draw object in 3d (Dont want this in ISR, lots of operations)
        if(draw3dFlag == true){
            if(spin == true){
                for(int j = 0; j<361; j++){
                    Render3D.rotateVertices(j, rotationAxis);
                    Render3D.generateProjected();
                    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
                    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                    BSP_LCD_FillRect(80, 50, 320, 155);
                    BSP_LCD_SetTextColor(drawColour);
                    for(int i = 0; i < 8099; i++){ // Connect each projected vertex to its neighbour
                        BSP_LCD_DrawLine(Render3D.xProjected[i] +xOffset3d, Render3D.yProjected[i] +yOffset3d, Render3D.xProjected[i+1] +xOffset3d,Render3D.yProjected[i+1] +yOffset3d);
                    }
                        // Restore vertex data from save
                    Render3D.restoreSave();
                }
                rotationAxis++; // Cycle axis of rotation every full rotation
                if(rotationAxis > 2){
                    rotationAxis = 0;
                    updateScreen.detach();
                }
            }else{
                Render3D.generateProjected();
                BSP_LCD_SetTextColor(drawColour);
                for(int i = 0; i < 8099; i++){ // Connect each projected vertex to its neighbour
                    BSP_LCD_DrawLine(Render3D.xProjected[i] +xOffset3d, Render3D.yProjected[i] +yOffset3d, Render3D.xProjected[i+1] +xOffset3d,Render3D.yProjected[i+1] +yOffset3d);
                }
            }
            draw3dFlag = false;
        }

            // Draw debug information on top of current screen. Toggleable.
        if(drawDebugFlag == true){
            while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
            BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
            char text [50];
            sprintf((char*)text, "IR Distance: %.2f    Voltage: %.2f", IR.getDistance(), IR.readVoltage()); 
            //BSP_LCD_ClearStringLine(2);
            BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *)&text, LEFT_MODE);
            float potVal = RangePot.readVoltage();
            sprintf((char*)text, "Range-Pot Distance: %d    Voltage: %.2f", utils.valmap(potVal, 0, 3.3, 0, 100), potVal); 
            //BSP_LCD_ClearStringLine(3);
            BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Encoder Clockwise: %d", Encoder.getClockwise()); 
            //BSP_LCD_ClearStringLine(4);
            BSP_LCD_DisplayStringAt(0, LINE(4), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Servo Position: %.2f", Servo.readPos());
            //BSP_LCD_ClearStringLine(5);
            BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Scan Clockwise: %d    Angle: %d   Layer: %d", direction, desiredAngle, depthMapLayer);
            //BSP_LCD_ClearStringLine(6);
            BSP_LCD_DisplayStringAt(0, LINE(6), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Radar Offset X: %d   Y: %d", radarXoffset, radarYoffset); 
            //BSP_LCD_ClearStringLine(7);
            BSP_LCD_DisplayStringAt(0, LINE(7), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "DepthMap Offset X: %d   Y: %d", depthMapXoffset, depthMapYoffset); 
            //BSP_LCD_ClearStringLine(8);
            BSP_LCD_DisplayStringAt(0, LINE(8), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "3D Array Index %d", pixelIndex); 
            //BSP_LCD_ClearStringLine(8);
            BSP_LCD_DisplayStringAt(0, LINE(9), (uint8_t *)&text, LEFT_MODE);
        }
    }
}

// Stepper angle is 3.6 in half step, 7.2 in full step //ish, cant find data online
// TODO:
// https://github.com/cbm80amiga/ST7735_3d_filled_vector/blob/master/gfx3d.h
