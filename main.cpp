// Angelo Maoudis 14074479
#include "mbed.h"
#include "rtos.h"
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
#include "uartCommands.h"

    // Magic Numbers / Constants                 // using constexpr over #define for evaluation at compile time. considered better practice in modern c++
namespace constants{
        // Menu/Option Constants
    constexpr int8_t MAX_MENU_ENTRIES = 4 -1;
        // Offsets/Coordinates for drawing on LCD
    constexpr uint8_t OFFSET_3D_X = 240;         // Offsets for drawing 3D object centred on (0,0) which is at the top left of the LCD.
    constexpr uint8_t OFFSET_3D_Y = 127;
    constexpr uint8_t OFFSET_RADAR_X = 110;      // Position offsets for drawing radar view.
    constexpr uint8_t OFFSET_RADAR_Y = 202;
    constexpr uint16_t OFFSET_DEPTHMAP_X = 310;  // Position offsets for drawing depth map.
    constexpr uint8_t OFFSET_DEPTHMAP_Y = 202;
}

    // Function declarations
void incrementScan(void);
void drawRadarView(float distance, float angle);
void drawDepthMap(float irDistance, float scanAngle, uint8_t scanLayer, uint8_t maxRange);
void updatePeripherals(float currentIrDistance, float currentScanAngle, uint8_t currentScanLayer, float currentPotReading);
void rotaryButtonPressed(void);
void rotaryTurned(void);
void draw3dObject(void);
void rotatingCubeDemo(void);
void drawDebugCube(void);
void manualRotation(void);
void tsButtonThreadFunction(void);
void toggleLCDLayer(void);

    // Obtaining XYZ values via scan
bool scanningClockwise = false; // Scan direction: False = CCW(-X), True = CW(+X) 
int8_t desiredScanAngle = 0;   // Scan/Stepper angle (Horizontal/X Axis)
uint8_t depthMapLayer = 0;      // Y value/layer of depthmap, also used for Servo angle (Vertical/Y Axis)
uint8_t rangeCutoff = 100;      // Any distances sensed past this value, are capped to this value (Depth/Z Axis)

    // Flags, mostly for executing functions incompatible with ISR (mutex, too slow, etc.)
bool progressScanFlag = false;  // Scan object flag, progress through 3D scan when this flag is set.
bool draw3dObjectFlag = false;  // Draw 3D object when this flag is set.
bool spinRenderFlag = false;    // Rotate 3D object automatically when this flag is set.
bool rotateTouchFlag = false;   // Enable touch buttons flag.
bool loadTestCubeFlag = true;   // Until this flag is un-set, 3D renderer draws a tri-coloured cube.
bool drawDebugFlag = false;     // Draw Debug-Screen. Peripheral data, etc.

    // Menu navigation/control
int8_t menuCounter = 0;         // Used to store last/select a menu option via rotary Encoder.
uint8_t rotationAxis = 0;       // Axis of rotation for 3D objects. 0, 1, 2 == X, Y, Z.
uint16_t pixelIndex = 8100;     // Current xyz index to write/read.

    // Misc.
uint32_t drawColour = LCD_COLOR_YELLOW; // This is modified by SliderDrawColour. Default setting is yellow.
uint8_t redArgb = 0xFF;                            
uint8_t greenArgb = 0xFF;                // ARGB values, used to cycle through ARGB spectrum via slider. Alpha is always 0xFF when these are used
uint8_t blueArgb = 0xFF;                 // Default is 0xFF (255) (white)
uint16_t lastX;                          // Coordinates of last drawn line for radar view, used to overwrite in black without clearing entire radar.
uint16_t lastY;                          // purely cosmetic.
int activeLCDLayer = 0;

    // Initialization, Peripheral Objects/Structs.
IrSense IR(A0);                                     // initialize IR sensor, reading from Pin A0
Pot RangePot(A1);                                   // initialize Potentiometer, reading from Pin A1
Servo Servo(D0, 180, 2.5, 1.5);                     // initialize Servo motor, on pin D0, with a 180 degree range between 1.5 and 2ms.
Stepper Stepper(D1, D2, D3, D4, 7.5);               // initialize Stepper motor, on pins D1, D2, D3, D4, with a step angle of 7.5
Rotary Encoder(D5, D6, D7, &rotaryButtonPressed, &rotaryTurned);    // initialize Rotary Encoder on D5,D6,D7, and pass functions to object
//MicroStepper Stepper(A5, A4, A3, A2, 7.5);        // Cant use microstepping as the board only has 2 DAC outs :(
Object3D Render3D(-200);  // initialize 3D Object
TS_StateTypeDef Touchstate;                         // Touchscreen-state Struct
UartInterface UartSerial(USBTX, USBRX, 115200);

    // Initialization, Touchscreen Button Objects
Button ButtonIncreaseRotationX(420, 460, 52, 92, LCD_COLOR_RED, LCD_COLOR_YELLOW, 1, Touchstate);               // Rotate around z axis
Button ButtonDecreaseRotationX(420, 460, 180, 220, LCD_COLOR_RED, LCD_COLOR_YELLOW, 2, Touchstate);
Button ButtonDecreaseRotationY(140, 180, 225, 265, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, 3, Touchstate);           // Rotate around y axis.
Button ButtonIncreaseRotationY(300, 340, 225, 265, LCD_COLOR_GREEN, LCD_COLOR_YELLOW, 4, Touchstate);
Button ButtonIncreaseRotationZ(20, 60, 52, 92, LCD_COLOR_BLUE, LCD_COLOR_YELLOW, 5, Touchstate);                // Rotate around z axis.
Button ButtonDecreaseRotationZ(20, 60, 180, 220, LCD_COLOR_BLUE, LCD_COLOR_YELLOW, 6, Touchstate);
Button ButtonResetRotation(350, 470, 230, 262, LCD_COLOR_MAGENTA, LCD_COLOR_CYAN, 0, Touchstate);               // Restore to saved vertices.
Button ButtonDecreaseFov(75, 130, 230, 262, LCD_COLOR_DARKCYAN, LCD_COLOR_WHITE, 0, Touchstate);                // fov-, Increases focal length.
Button ButtonIncreaseFov(10, 65, 230, 262, LCD_COLOR_DARKBLUE, LCD_COLOR_WHITE, 0, Touchstate);                 // fov+, Decreases focal length.
Slider SliderDrawColour(10, 65, 10, 42, LCD_COLOR_LIGHTMAGENTA, LCD_COLOR_WHITE, 0, Touchstate, true, 30, 450); // Change drawing colour via slider

    // Classes/Mbed Objects, Tickers/Interrupts/Etc.
Utilities Utils;            // Utilities class. Frequently used, non program-specific functions.
Ticker TickerNextStep;      // used to iterate through object scan
Ticker TickerUpdateScreen;  // Refresh screen with updated view from selected mode, normally 50Hz

    //Threads / RTOS / Mutex
Thread ThreadTSButtons;       // Dedicated thread for handling TS Button Presses
Mutex MutexTSButtons;         // Mutex lock for when reset button needs to load (8100*3) vertices.
Semaphore SemaphoreTSButtons(1, 1); // Semaphore, used to control execution of TS Button thread

//----------------------------Function definitons--------------------------------------------

    // Select & Execute menu options when button is pressed // Triggered by interrupts in rotary lib
void rotaryButtonPressed(void){
    for(int i = 0; i < 2; i++){
        BSP_LCD_Clear(LCD_COLOR_BLACK);
        switch(menuCounter){
            case 0:
                TickerUpdateScreen.detach();
                TickerNextStep.detach();
                    // Attatch ticker to this flag. Increments scan progress
                TickerNextStep.attach(incrementScan, 20ms); // 50Hz
                break;
            case 1:
                TickerUpdateScreen.detach();
                TickerNextStep.detach();
                    // Attatch ticker to this flag. Redraws Object
                TickerUpdateScreen.attach(draw3dObject, 20ms); // 50Hz
                spinRenderFlag = true;
                break;
            case 2:
                TickerUpdateScreen.detach();
                TickerNextStep.detach();
                    // Restore 3D object from save
                Render3D.restoreSave();
                    // Draw buttons
                BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
                BSP_LCD_DrawRect(79, 49, 321, 156);
                ButtonIncreaseRotationX.drawButton(); ButtonDecreaseRotationX.drawButton();
                ButtonIncreaseRotationY.drawButton(); ButtonDecreaseRotationY.drawButton();
                ButtonIncreaseRotationZ.drawButton(); ButtonDecreaseRotationZ.drawButton();
                ButtonIncreaseFov.drawButton(); ButtonDecreaseFov.drawButton();
                ButtonResetRotation.drawButton();
                SliderDrawColour.drawButton();
                BSP_LCD_SetFont(&Font16);
                BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
                BSP_LCD_DisplayStringAt(80, 241, (uint8_t*)"fov-", LEFT_MODE);
                BSP_LCD_DisplayStringAt(15, 241, (uint8_t*)"fov+", LEFT_MODE);
                BSP_LCD_DisplayStringAt(365, 241, (uint8_t*)"reset", LEFT_MODE);
                BSP_LCD_SetFont(&Font12);
                    // Attatch ticker to this flag. Rotates and draws based on ts input
                TickerUpdateScreen.attach(manualRotation, 1ms); // 50Hz
                break;
            case 3:
                drawDebugFlag = !drawDebugFlag;
                break;
            default:    // should never happen
                BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Something went wrong (rotaryButtonPressed)", CENTER_MODE);
                break;
            while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
            toggleLCDLayer();
        }
    }
}

    // Scroll through menu when Encoder is turned
void rotaryTurned(void){
    (Encoder.getClockwise() == true) ? menuCounter++ : menuCounter--;
    if(menuCounter > constants::MAX_MENU_ENTRIES){
        menuCounter = 0; 
    }else if(menuCounter < 0){
        menuCounter = constants::MAX_MENU_ENTRIES;
    }
    BSP_LCD_ClearStringLine(1);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    switch(menuCounter){
        case 0:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Start Scan", CENTER_MODE);
            toggleLCDLayer();
            BSP_LCD_ClearStringLine(1);
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Start Scan", CENTER_MODE);
            break;
        case 1:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Rotate Scanned Object", CENTER_MODE);
            toggleLCDLayer();
            BSP_LCD_ClearStringLine(1);
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Rotate Scanned Object", CENTER_MODE);
            break;
        case 2:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Manual Object Rotation", CENTER_MODE);
            toggleLCDLayer();
            BSP_LCD_ClearStringLine(1);
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Manual Object Rotation", CENTER_MODE);
            break;
        case 3:
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Debug Mode", CENTER_MODE);
            toggleLCDLayer();
            BSP_LCD_ClearStringLine(1);
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Menu: Debug Mode", CENTER_MODE);
            break;
        default:    // should never happen
            BSP_LCD_DisplayStringAt(0, LINE(1), (uint8_t *)"Something went wrong (rotaryTurned)", CENTER_MODE);
            break;
    }
}

    // Draw radar representaion of current scan (X&Z progress along each Y layer)
void drawRadarView(float irDistance, float scanAngle){ 
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawLine(constants::OFFSET_RADAR_X, constants::OFFSET_RADAR_Y, lastX + constants::OFFSET_RADAR_X, lastY + constants::OFFSET_RADAR_Y); // overwrite last drawn line in black
    uint16_t x = irDistance * cos(constants::PI_DOUBLE * 2 * (scanAngle + 225) / 360); 
    uint16_t y = irDistance * sin(constants::PI_DOUBLE * 2 * (scanAngle + 225) / 360); // find x,y coords of a point on a circle, of radius 'distance' at angle 'angle'
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DrawLine(constants::OFFSET_RADAR_X, constants::OFFSET_RADAR_Y, x + constants::OFFSET_RADAR_X, y + constants::OFFSET_RADAR_Y); // draw a line from radar offset, to x,y + offset
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DrawCircle(x + constants::OFFSET_RADAR_X, y + constants::OFFSET_RADAR_Y, 1); // mark the end of the line with a yellow blob
    BSP_LCD_DrawLine(x + constants::OFFSET_RADAR_X, y + constants::OFFSET_RADAR_Y, lastX + constants::OFFSET_RADAR_X, lastY + constants::OFFSET_RADAR_Y); // connect the blobs
    lastX = x; // store last used x,y values
    lastY = y;
}

    // Draw 2D image with depth data. Closer distance = brighter pixel
void drawDepthMap(float irDistance, float scanAngle, uint8_t scanLayer, uint8_t maxRange){ 
    uint16_t drawX = (scanAngle) + constants::OFFSET_DEPTHMAP_X; //0-90deg from left = x coord 0 to 90 from offset //TODO: "normalise" the image so it isnt skewed 
    uint16_t drawY = constants::OFFSET_DEPTHMAP_Y - scanLayer;                                                     //(Z = cos(angle) * distance??? check maths before implementing)
    uint8_t redBrightness = Utils.valMap(irDistance, 0, maxRange, 0xFF, 0x00);
    BSP_LCD_DrawPixel(drawX, drawY, Utils.argbToHex(0xFF, redBrightness, 0x00, 0x00));
        // Draw bounding box
    BSP_LCD_SetTextColor(drawColour);
    BSP_LCD_DrawRect(constants::OFFSET_DEPTHMAP_X - 1, constants::OFFSET_DEPTHMAP_Y - 91, 91, 91);
}

    // Updates draw functions with peripheral data & calls them
void updatePeripherals(float currentIrDistance, float currentScanAngle, uint8_t currentScanLayer, float currentPotReading){
        // Cap distance values via pot. Scales brightness in function "drawDepthMap"
    rangeCutoff = Utils.valMap(currentPotReading, 0, 3.3, 0, 100);
    if(currentIrDistance > rangeCutoff) {currentIrDistance = rangeCutoff; }
    char text [64];
         // Clear line. Display scan progress. Draw 2D views
    sprintf((char*)text, "Distance: %f Layer: %d Angle: %f MaxRange: %d", currentIrDistance, currentScanLayer, currentScanAngle, rangeCutoff);
    //BSP_LCD_ClearStringLine(LINE(0));
    BSP_LCD_DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    drawRadarView(currentIrDistance, currentScanAngle);
    drawDepthMap(currentIrDistance, currentScanAngle, currentScanLayer, rangeCutoff);
}

    // Rotate a cube around all 3 axis, 3d rendering demo for testing/debugging
void rotatingCubeDemo(void){
        // Storing 8 Vertices of a cube in render object.
    Render3D.Vertices.x[0] = -40; Render3D.Vertices.y[0] = -40; Render3D.Vertices.z[0] = 40;  // front bottom left
    Render3D.Vertices.x[1] = 40; Render3D.Vertices.y[1] = -40; Render3D.Vertices.z[1] = 40;   // front bottom right
    Render3D.Vertices.x[2] = 40; Render3D.Vertices.y[2] = 40; Render3D.Vertices.z[2] = 40;    // front top right
    Render3D.Vertices.x[3] = -40; Render3D.Vertices.y[3] = 40; Render3D.Vertices.z[3] = 40;   // front top left
    Render3D.Vertices.x[4] = -40; Render3D.Vertices.y[4] = -40; Render3D.Vertices.z[4] = -40; // back bottom left
    Render3D.Vertices.x[5] = 40; Render3D.Vertices.y[5] = -40; Render3D.Vertices.z[5] = -40;  // back bottom right
    Render3D.Vertices.x[6] = 40; Render3D.Vertices.y[6] = 40; Render3D.Vertices.z[6] = -40;   // back top right
    Render3D.Vertices.x[7] = -40; Render3D.Vertices.y[7] = 40; Render3D.Vertices.z[7] = -40;  // back top left
    Render3D.saveVertices();
        // Display the cube at every angle from 0 to 360 along an axis. for 361 so it ends at 360
    for(int j = 0; j < 361; j += 3){
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
        BSP_LCD_Clear(LCD_COLOR_BLACK);
        TickerUpdateScreen.detach();
    }
}

    // Draw the first 8 Vertices in the buffer as if they were a wireframe cube. Cube loaded here at startup. Development tool.
void drawDebugCube(void){
    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(80, 50, 320, 155);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);    // Edges connecting front and rear faces
    BSP_LCD_DrawLine(Render3D.xProjected[7]+constants::OFFSET_3D_X, Render3D.yProjected[7]+constants::OFFSET_3D_Y, Render3D.xProjected[3]+constants::OFFSET_3D_X, Render3D.yProjected[3]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[6]+constants::OFFSET_3D_X, Render3D.yProjected[6]+constants::OFFSET_3D_Y, Render3D.xProjected[2]+constants::OFFSET_3D_X, Render3D.yProjected[2]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[4]+constants::OFFSET_3D_X, Render3D.yProjected[4]+constants::OFFSET_3D_Y, Render3D.xProjected[0]+constants::OFFSET_3D_X, Render3D.yProjected[0]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[5]+constants::OFFSET_3D_X, Render3D.yProjected[5]+constants::OFFSET_3D_Y, Render3D.xProjected[1]+constants::OFFSET_3D_X, Render3D.yProjected[1]+constants::OFFSET_3D_Y);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW); // Front face
    BSP_LCD_DrawLine(Render3D.xProjected[0]+constants::OFFSET_3D_X, Render3D.yProjected[0]+constants::OFFSET_3D_Y, Render3D.xProjected[1]+constants::OFFSET_3D_X, Render3D.yProjected[1]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[1]+constants::OFFSET_3D_X, Render3D.yProjected[1]+constants::OFFSET_3D_Y, Render3D.xProjected[2]+constants::OFFSET_3D_X, Render3D.yProjected[2]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[2]+constants::OFFSET_3D_X, Render3D.yProjected[2]+constants::OFFSET_3D_Y, Render3D.xProjected[3]+constants::OFFSET_3D_X, Render3D.yProjected[3]+ constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[3]+constants::OFFSET_3D_X, Render3D.yProjected[3]+constants::OFFSET_3D_Y, Render3D.xProjected[0]+constants::OFFSET_3D_X, Render3D.yProjected[0]+constants::OFFSET_3D_Y);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);   // Rear face
    BSP_LCD_DrawLine(Render3D.xProjected[4]+constants::OFFSET_3D_X, Render3D.yProjected[4]+constants::OFFSET_3D_Y, Render3D.xProjected[5]+constants::OFFSET_3D_X, Render3D.yProjected[5]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[5]+constants::OFFSET_3D_X, Render3D.yProjected[5]+constants::OFFSET_3D_Y, Render3D.xProjected[6]+constants::OFFSET_3D_X, Render3D.yProjected[6]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[6]+constants::OFFSET_3D_X, Render3D.yProjected[6]+constants::OFFSET_3D_Y, Render3D.xProjected[7]+constants::OFFSET_3D_X, Render3D.yProjected[7]+constants::OFFSET_3D_Y);
    BSP_LCD_DrawLine(Render3D.xProjected[7]+constants::OFFSET_3D_X, Render3D.yProjected[7]+constants::OFFSET_3D_Y, Render3D.xProjected[4]+constants::OFFSET_3D_X, Render3D.yProjected[4]+constants::OFFSET_3D_Y);    
}

void toggleLCDLayer(void){
    if(activeLCDLayer == 1){
        BSP_LCD_SelectLayer(0); BSP_LCD_SetLayerVisible(1, ENABLE); BSP_LCD_SetLayerVisible(0, DISABLE);
        activeLCDLayer = 0;
    }else{
        BSP_LCD_SelectLayer(1); BSP_LCD_SetLayerVisible(0, ENABLE); BSP_LCD_SetLayerVisible(1, DISABLE);
        activeLCDLayer = 1;
    }
}


// ---------------------Flags set by Interrupts-------------------------------------------------------------------------
    // Draw projected model onto the LCD 
void draw3dObject(void){
    draw3dObjectFlag = true;
    rotateTouchFlag = false;
    loadTestCubeFlag = false;
}

    // Take peripheral reading, then move Servo & Stepper to next position
void incrementScan(void){
    progressScanFlag = true;
    spinRenderFlag = false;
    rotateTouchFlag = false;
    loadTestCubeFlag = false;
}

    // Rotate and draw object. Based on ts input.
void manualRotation(void){
    rotateTouchFlag = true;
    spinRenderFlag = false;
    progressScanFlag = false;
    draw3dObjectFlag = false;
}

//--------------------Functions executed via seperate thread-------------------------------------------------------------
    // Check TS button states, and rotate render accordingly
void tsButtonThreadFunction(void){
    while(true){
            // Try to aquire semaphore, wait untill one is available
        SemaphoreTSButtons.acquire();
            // Rotate or reset according to buttons pressed
        if(ButtonIncreaseFov.isPressed()) {Render3D.focalLength+=1;}                 // Note: yes. if-elseif-else is faster here. (stops comparisons when true).
        if(ButtonDecreaseFov.isPressed()) {Render3D.focalLength-=1;}                 // using if-if-if to support multiple simultaneous button presses.
        if(ButtonIncreaseRotationX.isPressed()) {Render3D.rotateVertices(1, 0);}
        if(ButtonDecreaseRotationX.isPressed()) {Render3D.rotateVertices(-1, 0);}
        if(ButtonIncreaseRotationY.isPressed()) {Render3D.rotateVertices(1, 1);}
        if(ButtonDecreaseRotationY.isPressed()) {Render3D.rotateVertices(-1, 1);}
        if(ButtonIncreaseRotationZ.isPressed()) {Render3D.rotateVertices(1, 2);}
        if(ButtonDecreaseRotationZ.isPressed()) {Render3D.rotateVertices(-1, 2);}
            // Lock thread via mutex, load saved vertices, then unlock thread again.
        if(ButtonResetRotation.isPressed()){
            loadTestCubeFlag = false;
            MutexTSButtons.lock();
            Render3D.restoreSave();
            MutexTSButtons.unlock();
        }
            // Choose object colour via slider.
        SliderDrawColour.isPressed();
        if(SliderDrawColour.sliderOut <= 33){
            redArgb = Utils.valMap(SliderDrawColour.sliderOut, 0, 33, 0x00, 0xFF);
            greenArgb = Utils.valMap(SliderDrawColour.sliderOut, 0, 33, 0xFF, 0x00);
            drawColour = Utils.argbToHex(0xFF, redArgb, greenArgb, blueArgb);
        }else if(SliderDrawColour.sliderOut <= 66){
            greenArgb = Utils.valMap(SliderDrawColour.sliderOut, 0, 33, 0x00, 0xFF);
            blueArgb = Utils.valMap(SliderDrawColour.sliderOut, 0, 33, 0xFF, 0x00);
            drawColour = Utils.argbToHex(0xFF, redArgb, greenArgb, blueArgb);
        }else if(SliderDrawColour.sliderOut <= 100){
            blueArgb = Utils.valMap(SliderDrawColour.sliderOut, 0, 33, 0x00, 0xFF);
            redArgb = Utils.valMap(SliderDrawColour.sliderOut, 0, 33, 0xFF, 0x00);
            drawColour = Utils.argbToHex(0xFF, redArgb, greenArgb, blueArgb);
        }
            // Release Control of semaphore, so this thread can be "paused"
        SemaphoreTSButtons.release();
    }
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

        // Setup another LCD layer
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS +(272*480*4));
    BSP_LCD_SelectLayer(0);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)"14074479 MAOUDISA", CENTER_MODE);
    HAL_Delay(1000);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_SetBackColor(LCD_COLOR_TRANSPARENT);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_Clear(LCD_COLOR_BLACK);


        // Initialize touchscreen
    BSP_TS_Init(480, 272);

    BSP_LCD_SelectLayer(0); BSP_LCD_SetLayerVisible(0, ENABLE); BSP_LCD_SetLayerVisible(1, DISABLE);
        // Attatch ticker to this flag. Using this demo as a splash screen
    TickerUpdateScreen.attach(rotatingCubeDemo, 1ms); 

    // Do nothing here until a flag is set
    while(1) {

        while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
        toggleLCDLayer();

            // Manual control over 3D render (Slow)
        if(rotateTouchFlag == true){
                // Relinquish control of Semaphore so TSButton thread can run
            SemaphoreTSButtons.release();
                // Attatch thread to monitor TS Buttons and rotate render accordingly, does nothing once thread is started.
            ThreadTSButtons.start(tsButtonThreadFunction);
                // Generate coordinates, Clear Object, Draw image. (Only clear immidiately before drawing to reduce strobing)
                // Buttons are not redrawn, But also not cleared. Faster. (Exception > Sliders)
            Render3D.generateProjected();
            SliderDrawColour.drawButton();
            if(loadTestCubeFlag == true){
                drawDebugCube();
            }else{
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(80, 50, 320, 155);
                BSP_LCD_SetTextColor(drawColour);
                for(int i = 0; i < constants::MAX_VERTICES -1; i++){
                    BSP_LCD_DrawLine(Render3D.xProjected[i] +constants::OFFSET_3D_X, Render3D.yProjected[i] +constants::OFFSET_3D_Y, Render3D.xProjected[i+1] +constants::OFFSET_3D_X, Render3D.yProjected[i+1] +constants::OFFSET_3D_Y);
                }
            }
        }

            // Scanning Routine, progress one step (causes mutex if in ISR)
        if(progressScanFlag == true){
                // Take semaphore so TSButtons cant run
            SemaphoreTSButtons.try_acquire();
                // Update peripheral data. Clear lcd between layers
            updatePeripherals(IR.getDistance(), desiredScanAngle, depthMapLayer, RangePot.readVoltage());
            (scanningClockwise == true) ? desiredScanAngle++ : desiredScanAngle--;
            if(desiredScanAngle >= 90 || desiredScanAngle < 0){
                for(int i = 0; i < 2; i++){
                    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                    BSP_LCD_FillRect(0, 22, 300, 250);
                    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
                    toggleLCDLayer();
                }
                scanningClockwise = !scanningClockwise;
                lastX = 0, lastY = 0;
                depthMapLayer++;
                depthMapLayer++;
                    // Check for scan completion. Detatch ticker & save Vertices
                if(depthMapLayer > 90){
                    TickerNextStep.detach();
                    depthMapLayer = 0;
                    pixelIndex = 8100;
                    Render3D.saveVertices();
                    draw3dObjectFlag = true;
                }
            }
                // Move Stepper and Servo. Store xyz Coordinates
            if(desiredScanAngle %4 == 0) {Stepper.step(scanningClockwise, 1, 7);} // Temporary? Uni only has crap Steppers. (step angle too large, even with half step)
            Servo.writePos(depthMapLayer);
            Render3D.Vertices.y[pixelIndex] = -45 + depthMapLayer;
            Render3D.Vertices.x[pixelIndex] = -45 + desiredScanAngle;
            Render3D.Vertices.z[pixelIndex] = (int16_t)(rangeCutoff / 2) - round(IR.lastDistance());
            if(IR.lastDistance() >= rangeCutoff) {Render3D.Vertices.z[pixelIndex] = -(int16_t)(rangeCutoff / 2);}
            pixelIndex--;
            draw3dObjectFlag = true; // Draws 3d object as it is scanned
            progressScanFlag = false;
        }

            // Draw object in 3d (Dont want this in ISR, lots of operations)
        if(draw3dObjectFlag == true){
                // Take semaphore so TSButtons cant run
            SemaphoreTSButtons.try_acquire();
            if(spinRenderFlag == true){
                for(int j = 0; j < 360; j++){
                    Render3D.rotateVertices(j, rotationAxis);
                    Render3D.generateProjected();
                    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                    BSP_LCD_FillRect(80, 50, 320, 155);
                    BSP_LCD_SetTextColor(drawColour);
                    for(int i = 0; i < constants::MAX_VERTICES -1; i++){ // Connect each projected vertex to its neighbour
                        BSP_LCD_DrawLine(Render3D.xProjected[i] +constants::OFFSET_3D_X, Render3D.yProjected[i] +constants::OFFSET_3D_Y, Render3D.xProjected[i+1] +constants::OFFSET_3D_X, Render3D.yProjected[i+1] +constants::OFFSET_3D_Y);
                    }
                    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
                    toggleLCDLayer();
                        // Restore vertex data from save
                    Render3D.restoreSave();
                }
                rotationAxis++; // Cycle axis of rotation every full rotation
                if(rotationAxis > 2){
                    rotationAxis = 0;
                    BSP_LCD_Clear(LCD_COLOR_BLACK);
                    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
                    toggleLCDLayer();
                    BSP_LCD_Clear(LCD_COLOR_BLACK);
                    while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS)) {}     // Wait for v-sync. (Magic.)
                    toggleLCDLayer();
                    TickerUpdateScreen.detach();
                }
            }else{
                Render3D.generateProjected();
                BSP_LCD_SetTextColor(drawColour);
                for(int i = 0; i < constants::MAX_VERTICES -1; i++){ // Connect each projected vertex to its neighbour
                    BSP_LCD_DrawLine(Render3D.xProjected[i] +constants::OFFSET_3D_X, Render3D.yProjected[i] +constants::OFFSET_3D_Y, Render3D.xProjected[i+1] +constants::OFFSET_3D_X,Render3D.yProjected[i+1] +constants::OFFSET_3D_Y);
                }
            }
            draw3dObjectFlag = false;
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
            sprintf((char*)text, "Range-Pot Distance: %d    Voltage: %.2f", Utils.valMap(potVal, 0, 3.3, 0, 100), potVal); 
            //BSP_LCD_ClearStringLine(3);
            BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Encoder Clockwise: %d", Encoder.getClockwise()); 
            //BSP_LCD_ClearStringLine(4);
            BSP_LCD_DisplayStringAt(0, LINE(4), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Servo Position: %.2f", Servo.readPos());
            //BSP_LCD_ClearStringLine(5);
            BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Scan Clockwise: %d    Angle: %d   Layer: %d", scanningClockwise, desiredScanAngle, depthMapLayer);
            //BSP_LCD_ClearStringLine(6);
            BSP_LCD_DisplayStringAt(0, LINE(6), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "Radar Offset X: %d   Y: %d", constants::OFFSET_RADAR_X, constants::OFFSET_RADAR_Y); 
            //BSP_LCD_ClearStringLine(7);
            BSP_LCD_DisplayStringAt(0, LINE(7), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "DepthMap Offset X: %d   Y: %d", constants::OFFSET_DEPTHMAP_X, constants::OFFSET_DEPTHMAP_Y); 
            //BSP_LCD_ClearStringLine(8);
            BSP_LCD_DisplayStringAt(0, LINE(8), (uint8_t *)&text, LEFT_MODE);
            sprintf((char*)text, "3D Array Index %d", pixelIndex); 
            //BSP_LCD_ClearStringLine(8);
            BSP_LCD_DisplayStringAt(0, LINE(9), (uint8_t *)&text, LEFT_MODE);
        }
            // Process Uart input (if any)
        UartSerial.processInput();
    }
}

// Stepper angle is 3.6 in half step, 7.2 in full step //ish, cant find data online
// TODO:
// https://github.com/cbm80amiga/ST7735_3d_filled_vector/blob/master/gfx3d.h
