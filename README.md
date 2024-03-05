# maoudisa-assignment
maoudisa 14074479 Assignment

An embedded system to draw an interpretation of a provided image with a pen. an art robot.

## 1 - Embedded System Proposal

Description:
a differential drive robot utilizing stepper motors to guide a pen around a peice of paper, with the end goal of creating a "hand drawn" drawing of a provided image. "hand-drawn" as in, drawn by a robot using a pen.

Inputs: a PNG Image

Outputs: a "hand-drawn" picture :). stepper motors, solenoid, pen

Basic operation: successful completion of the systems task can be considered achieved, if the robot completes it's routine and results in what can reasonably be considered a 'drawing' of the original image

## 2 - Hardware Design

PCB Gerber files and Circuit Schematics will be included in repo

Component List:

STM32f74G-DISCO
OK42FD34-0316MB-F1 Stepper motor x2
Adafruit 6v Solenoid (they lie, i will be driving this at 12V or else it doesnt have enough oomphf to lift a pen)
LM7805CV 5v regulator
ULN2803A transistor array x2
L297 stepper motor driver x2
IRL540N MosFET
12V Battery

Bunch of 3D printed crap, i will include STL files in the repo

maybe i dont know this whole thing is speculation at this point
big fat todo

## 3 - Embedded Code Implementation and Architechture
todo

## 4 - Evaluation 
todo
