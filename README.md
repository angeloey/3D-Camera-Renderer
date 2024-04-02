//Angelo Maoudis README // 14074479 

# maoudisa-assignment
maoudisa 14074479 Assignment

3D Scanner & Renderer
An embedded system to scan an object, then render the resulting scan in 3D to be inspected by the user.

### Goals & Pre-Project Discussion:
Goals for the Assignment. \
Namely, the objectives defined in the marking criteria, and how i intend to achieve each one.

#### Grade:
100%.
Satisfy all marking criteria to a maximum

#### Complexity (Technologies):
the goal of the project will be to develop the aforementioned embedded system using a range of technologies as to satisfy the assignment criteria.\
The system will purposely include a range of technologies, even in scenarios where it would traditionally make sense to use multiple instances of a
single technology, for example: instead of using two stepper motors (one for x movement and one for y movement), this project will make use of a
stepper motor for x movement and a servo motor for y movement.\
or by using a rotary encoder and a potentiometer, where two potentiometers would suffice.\
While this may not be the most practical approach, it affords me the opportunity to write a library for, and implement the use of more technologies which will better aid the achievement of the following excerpt from the marking criteria:

*"a wide range of complex\
input and output devices some of\
which we may not have used in\
the tutorial material, all working\
as expected."*

#### Version Control (Regular Commits and a Clear Workflow):
Another goal of the project is to provide a meaningful and well commented/documented update to the repository at regular intervals. \
(I will aim for daily commits, however: life waits for no man.) \
Any periods without a commit will be compensated for in magnitude or quantity once updates resume. \
There are 27 days between the start of this project on March 6th, and the due date April 2nd, therefore I will ensure there are at MINIMUM, 27 commits of
substanstial size, quality, or progress before April 2nd. \
Each commit will aim to demonstrate a clear direction and progress route through the projects development, i.e. evidence i havent just copied a load of
stuff i dont understand from stackoverflow.

I consider a meaningful update to be along the lines of, but not limited to, the following :

 - Functional progress in, or the completion of, a library for a component (.h and .cpp files in folder designated: "My-X-Lib"). 
 - Well commented, significant progress on the functionality of the project. 
 - Significant progress in, or beginning of, the Implementation of a new technology (a functional addition to debug screen for example.).
 - "Housekeeping": formatting, commenting, & encapsulating code to better reflect OOP practices etc.
 - Checkoints, functional milestones, or working implementation of a feature, such as a demo (i.e. "50Hz Rotating Cube Demo" on March 18th).

TL;DR:
Regular updates of some significance, with an accurate commit message.\
No "added some code" commits.\
Clear workflow and direction should be visibile throughout commit history (once target embedded system has been decided).\

this is with the intention of achieving the following assignment criteria excerpt:

*"Evidence of regular daily\
commits over a longer time\
frame. Demonstrates clear\
incremental progress with logical\
commit messages for features etc."*


#### Code Quality:
Write practical code and follow best practices throughout all aspects of the assignment, including but not limited to things such as:

 - dont use delays in an interrupt, dont use GOTO: etc.
 - dont name variables things like: "foo", "bar", and "var1", code should be "self documenting" and readable.
 - use MEANINGFUL comments as a form as documentation, no functions without comments, but also dont have a useless comment on every single line.
 - need to call the same bit of code many many times? PUT IT IN A FUNCTION/CLASS/LIBRARY.
 - camelCase is king.
 - dont use uint32_t where uint8_t would suffice. not exclusive to integers. *(Exception: Chris said it's ok to "throw floats at it")*

(I will consider myself exempt from these restrictions during development/debugging, however final code should follow all without exception). \
the following assignment criteria is self explanitory, I will attempt to be a good code monkey:


*Outstanding tidy code. Good\
practices being used e.g.\
encapsulation, meaningful\
identifiers and an attempt at\
documentation. Code contains\
no linting errors or warnings."*

Additional overlap from the following excerpt:

*"Use of custom classes, objects,\
and OOP concepts."*

### Naming Convention adhered to:
- Variables: camelCase
- User-Defined/Custom Types: PascalCase
- Constants: CAPS_SEPARATED_BY_UNDERSCORES
- Private Member Variables: /_underscorePrefixedCamelCase
- Descriptive Self-Documenting identifiers are mandatory. 

#### Complexity & Architechture (Embedded Code)

the following criteria is self explanitory, dont have the entire thing in a while loop:

*"The embedded architecture used\
likely makes use of a\
combination of hard and soft\
interrupts with polling or perhaps,\
RTOS, multi-threading, event\
loops, etc."*





## 1 - Embedded System Proposal

Description: A 3D Scanner.
A device which scans a scene along the X and Y axis, while recording Z axis data about the scene. A 2D representation of the scene, alongside a 3D render is generated from the recoded data, which the user can interact with to inspect the object/scene post scan. Colour is used to represent depth in the 2d Image.


Basic operation:
The device uses a stepper motor and a servo to take range measurements in increments of 1 degrees, in a 90x90 degree FOV.\
The range measurements are made using an IR Sensor, the maximum range of which can be configured using an potentiometer.\
During and after completion of the scan, the data recorded is used to represent the scanned scene on the LCD Screen. The user can interact with a 3D render of the scanned scene via the touchscreen, or by selecting an option to automatically rotate the render via the rotary encoder menu.\
Uart Commands can also be used to start a scan remotely.

The system also contains customization options such a colour slider, and fov adjustment, which are modified in their corresponding menu via the touchscreen interface.

Verification of Successful Operation:
The fundamental operation of the system can be considered successful if a scene/object can be scanned by the device, producing a render which can be interacted with by user. I.e. can be rotate around all 3 Axis at will. A wide array of diferent technologies and peripherals will be used by the system to achieve this, and will work together without error or conflict.





## 2 - Hardware Design

Component List:

STM32f74NG-DISCO

Inputs:
 - Uart via Serial Port - STM32F74NG-DISCO
 - Potentiometer - NHP22
 - Rotary Encoder - EC11
 - IR Range Sensor - GP2Y0A21YK
 - Touchscreen - STM32F74NG-DISCO

Outputs:
 - Uart via Serial Port - STM32F74NG-DISCO
 - LCD Screen - STM32F74NG-DISCO
 - Servo Motor - SG90
 - Stepper Motor - PJJ051ZA-P

STM32F74NG-DISCO Datasheet: https://www.farnell.com/datasheets/1944312.pdf \
IR Sensor Datasheet: https://www.sparkfun.com/datasheets/Components/GP2Y0A21YK.pdf \
Potentiometer Datasheet: https://p3america.com/content/pdf/nhp22.pdf \
Rotary Encoder Datasheet: https://www.farnell.com/datasheets/1837001.pdf \
Servo Datsheet: https://static.rapidonline.com/pdf/37-1330t.pdf \
Stepper Motor Datsheet: University Provided, cannot find information online, generic unipolar stepper operation. \
ULN2803 Darlington Array Datasheet: https://www.st.com/resource/en/datasheet/uln2801a.pdf

Connection Diagram: \
![image](https://github.coventry.ac.uk/storage/user/6796/files/ee9ec413-ffa5-4819-95da-15ec06ea4bdf)


System Runs on MbedOS 6 \
MbedOS Docs: https://os.mbed.com/docs/mbed-os/v6.16/introduction/index.html 

To stay within the "spirit" of the Embedded Systems Programming module, the System opts for software control over hardware control, via the GPIO, ADC, & DAC pins built in to the STM32f746NG-DISCO, with the only exception being the use of a ULN2803 Darlington Array IC which is used as a *driver* for the stepper motor. Stepper motor *control* is achieved via software implementation however.





## 3 - Embedded Code Implementation and Architechture

### Structure Overview

The system uses a combination of hard and soft interrupts, polling, and multithreading. \
This was an intentional design choice in order to demonstrate/implement a wider range of techniques. \
Hardware interrupts are used to control the physical IO of the device, such as the rotary encoder & menu navigation. \
Software interrupts (Tickers) are used to progress through the scanning process, as well as to refresh the LCD display. \
The touchscreen interface is handled by its own dedicated thread, running in paralell to the main thread of the device. Semaphores are used to manage the individual threads, and to ensure a thread only uses system resources when necessary. \
A polling scheme is used to manage the UART interface.  \
A flag polling system is also used to manage routines triggered by interrupts without executing them inside the ISR, both to avoid unwanted mutex locks, 
and to adhere to best practices (i.e. no delays or lengthy functions inside the ISR). 

Functionality of peripheral devices is contained and managed by individual Objects & corresponding libraries. 

All source files are commented where apporopriate to provide a reasonable level of documentation, "Pseudo Diagrams" have been drawn using comments in files where a greater level of explanation was felt necessary, example below from My-Stepper-Lib:

![image](https://github.coventry.ac.uk/storage/user/6796/files/95734af4-9ebb-4856-bf2c-dd8f5a78eafe)


### Tasks Handled by Software

### Scene/Object Scanning:
__Description__: The scanning routine is responsible for triggering & reading peripheral devices such as the stepper motor, servo, IR sensor,
and potentiometer. And displaying the progress of the scan to the user.
        
__Real-time Constraints__: During scanning data is collected from these peipherals, and output output to the motors in realtime as the scan progresses.
A 2D representation of the scene, as well as a 3D render of the scanned object is updated in realtime to indicate scan progress to the user.
        
__Responsiveness__: The system only allocates resources to areas required by this routine as and when they are needed. These resources are managed by a series of flags set and unset by the Scanning routine as the scan progresses.

### Hardware Interface Interaction:
__Description__: A rotary encoder is used to cycle through a menu, which provides primary method of user interaction with the system.
        
__Real-time Constraints__: This is handled using hardware interrupts, and will take priority over all tasks so that the user can control the device               without hinderence, with the exception of the scanning routine. If the scanning routine is currently running, the vertices from the last successfull scan are loaded into the vertex buffer before executing any instructions, to avoid any unfinished scans being loaded by the renderer.
        
__Responsiveness__: As mentioned above, this is handled by hardware interrupts to provide immediate response to user interaction.

### Touch Interface:
__Description__: Detecting touch on the LCD in specific areas, and executing tasks accordingly. The touch interface consists of 9 touch buttons used to rotate a scanned object in 3D, as well as to alter the camera focal length of the renderer. Additionally a slider is also used to change the draw colour of the system.
        
__Real-time Constraints__: Handled by its own dedicated thread that runs alongside the main thread of the system in order to respond to user interaction in realtime. \ This was intentional in order to demonstrate a wider range of technologies. Semaphores are used to ensure that the touch interface thread only consumes system resources when touchscreen buttons are visible on the screen.
        
__Responsiveness__: Since it is handled by its own thread, responsiveness to touch inputs is immediate, and has support for multiple simultaneous touches

### Uart Interface:
__Description__: A barebones uart interface is used to trigger the scanning routine remotely, upon recieveing a newline (\n) character via the serial port, the device compares the characters stored in its character buffer to a list of known commands, and executes hte corresponding command accordingly. If none are found, it notfies the user that it does not recognise the recived command. This was included primarily to better satisfy the marking criteria and demonstrate the use of serial communication, but finds itself analagous to existing embedded systems where remote activation is a desired feature.

__Real-time Constraints__: Characters are added to a buffer as and when they are recieved by the device, not particularly time-critical and is polled once any set flags have been acknowledged.

__Responsiveness__: No noticeable delay by the user. Effectively immediate from a humans POV.

### 3D Rendering:
__Description__: The device is capable of representing the data recorded by the scanning routine in the form of a weak perspective projection. The files inside folder My-3D-Lib are heavily commented to explain how the 3D rendering of the system is achieved, below is an attempt to illustrate how the "Projected" pixel coordinates are calculated from the vertices recorded by the Scanning routine, for the X axis. This process is repeated for each coordinate recorded (90 x 90 = 8100).

![illustration](https://github.coventry.ac.uk/storage/user/6796/files/8975266f-458d-43b4-8f1d-a7de5b1724ef) 
                           

 Triangles (X, Adjacent, C) and (Xp, Focallength, C) are similar, \
 therefore: Xp / X  ==  FocalLength / Adjacent \ 
 thus Xp  ==  (Focallength * X) / Adjacent \
 Xp (xProjected) is the X coordinate of the pixel intersected when casting a ray from the camera to the object. \
 Repeat for Y. Process is analagous. \
 Repeating this process for all scanned coordinates results in a list of XY pixel coordinate which when drawn on the screen \
 form a weak perspective projection of the scanned Scene/Object. 

__Real-time Constraints__: During scanning, a 3D render of the scene is constructed in realtime as the system records more data. However this render does not need to rotate, and given that the scan operates at a frequency of 20Hz, and each pixel is calculated as its corresponding vertex is obtained in realtime, ample time is available to render the scene in realtime. \
Once the scan has completed, the object can be rendered, rotated, and inspected by the user. This involves significantly more computational overhead and the frame-rate of the system is limited by the speed at which the FPU can complete these calculations.

__Responsiveness__: When rotating and inspecting the render, the system becomes significantly less responsive, and "Double-Buffering", along with V-Sync was implemented in order to eliminate screen tearing/flickering, as the device cannot calculate and draw all of the projected vertices fast enough to be imperceptible without this.


### Source Code Authors
Files written by me: 
 - Everything in a folder labelled "My-X"
 - main.cpp
 - README.md
 - Unit tests inside folder TESTS (Made from boilerplate provided in mbed docs, link to boilerplate provided as comments inside each file).
 All files are labelled with Name and SID as top comments.

Third-Party Code: 
 - BSP_DISCO_F746NG.lib
 - Mbed-OS.lib

### Practical Embedded Considerations

The scan system was originally made using an ultrasonic sensor, and a library for this sensor sourced from mbed.org, however the output was noisy and I intend to write all libraries for components myself. To solve this issue replaced with IR range sensor (much less noisy output) and wrote library from scratch (My-IR-Lib).

In order to target the IR Range sensor at a specific point, a Mount was designed and 3D-Printed, the latest version of this model in STL format is included in the repository under folder: 3D Print Files

The image below shows from left to right, Versions 1, 2, 3, & 4, of the aforementioned mount:

![Mounts_Cropped_2](https://github.coventry.ac.uk/storage/user/6796/files/c0bbbd6b-cb62-418b-baf3-95458bf07fb2)


Rotating a perspective projection render in 3 dimensions is computationally expensive, and somewhat out of reach for a device such as the STM32F7 to do in any reasonable amount of time. However this system takes advantage of a "workaround". 
Provided you only rotate around one axis at a time, a perspective projection can be rotated using the 2D rotation matrix, which is much less computationally expensive.
To provide rotation around all 3 axis, this system rotates around one axis, saves the result to a buffer, and then rotates the resulting buffer around another axis sequentially. this workaround means that the system can appear to rotate a render around 3 axis simultaneously, albeit while taking 3 times as long to do so.

Functionality was written into the stepper library to enable microstepping, however the STM32f7 Board being used for this system only has 2 onboard ADC's and the implementation (the only way i could think of to do it in software) required 4 ADC's to operate. 
As the step size of the provided stepper motor is too large for the degree of accuracy desired, the system only rotates the stepper every 4 degrees instead of every 1 degree, and takes 4 measurements during its travel time to acquire the closest approximation I could get. 
I am unable to find any information on this specific stepper online, but measuring its step size by hand, it appears to have a step size of ~7.5 Degrees, so I am using half stepping to get this down to ~4, and using the aforementioned method to compensate for it.






## 4 - Evaluation 
### Implementation Successfulness
The system achieves the criteria outlined in the proposal, it is capable of scanning scenes or objects and generating a 3D render which the user can freely rotate along all three axis. It utilises a wide range of technologies and peripherals, which operate without conflict. 
The system can be considered a sucessful prototype.

Upon reflection, some shortcomings of the device or things I would add given more time include:
- Ability to export renders to another device (maybe as .STL or similar).
- The stepper motor used for the project works fine as a proof of concept/prototype however in a future iteration i would substitute it for a stepper with a smaller minimum step angle, or use a MCU with multiple DAC's as to utilize microstepping
- Create an enclosure or housing for the device (Maybe a battery powered handheld iteration)
- Create a PCB implementation instead of relying on a breadboard
- Add greater functionality to the uart interface. a HC-05 module could be used along side a terminal app, to control the device from a phone

### Group Members:
Angelo Maoudis 14074479 \
Commits as: maoudisa(University Github) & angeloey (Home Github). 

## Link to Video Demonstration:
https://youtu.be/fXoxNYmKON4
