# maoudisa-assignment
maoudisa 14074479 Assignment

An embedded system to do something?

### Goals:

#### Grade:
100% lol\
Satisfy all marking criteria to a maximum\

#### Complexity (Technologies):
the goal of the project will be to develop the aforementioned embedded system using a range of technologies as both a learning excercise and as a method to demonstrate individual ability in this module.\
Purposely include a range of technologies, even in scenarios where it would traditionally make sense to use multiple instances of a single technology, for example: instead of using two stepper motors (one for x movement and one for y movement), this project will make use of a stepper motor for x movement and a servo motor for y movement. or by using a rotary encoder and a potentiometer, where two potentiometers would suffice.\
while this may not be the most practical approach, it affords me the opportunity to write a library for, and implement the use of more technologies which will better aid the achievement of the following excerpt from the marking criteria:\

"a wide range of complex\
input and output devices some of\
which we may not have used in\
the tutorial material, all working\
as expected. "\

#### Version Control (Regular Commits and a Clear Workflow):
Another goal of the project is to provide a meaningfull and well commented/documented update to the repository at an interval of at least 4 times per week (I will aim for daily commits, however: 5 assignemnts + 2 exams due in the next few weeks).\
Each commit will aim to demonstrate a clear direction and progress route through the projects development, i.e. evidence i havent just copied a load of stuff i dont understand from stackoverflow.\

I consider a meaningfull update to be along the lines of, but not limited to, the following :\

 - Clear functional progress in, or the completion of, a library for a component (.h and .cpp files in folder designated: "My-X-Lib").\
 - Well commented, significant progress on the functionality of the project.\
 - Significant progress or beggining of Implementation of a new technology (functional addition to debug screen or similar etc.).\
 - "Housekeeping": formatting, commenting, & encapsulating code to better reflect OOP practices etc.\

TL;DR:
Regular updates of some significance, with an accurate commit message.\
No "added some code" commits.\
Clear workflow and direction should be visibile throughout commit history (once target embedded system has been decided).\

this is with the intention of achieving the following assignment criteria excerpt:\

"Evidence of regular daily\
commits over a longer time\
frame. Demonstrates clear\
incremental progress with logical\
commit messages for features etc."\

#### Code Quality:
Write practical code and follow best practices throughout all aspects of the assignment, including but not limited to things such as:\

 - dont use delays in an interrupt, dont use GOTO: etc.\
 - dont name variables things like: "foo", "bar", and "var1", code should be "self documenting" and readable.\
 - use MEANINGFUL comments as a form as documentation, no functions without comments, but also dont have a useless comment on every single line.\
 - need to call the same bit of code many many times? PUT IT IN A FUNCTION/CLASS/LIBRARY.\

(I will consider myself exempt from this restrictions during development/debugging, however final code should follow all without exception).\
the following assignment criteria is self explanitory, I will attempt to be a good code monkey:\

"Outstanding tidy code. Good\
practices being used e.g.\
encapsulation, meaningful\
identifiers and an attempt at\
documentation. Code contains\
no linting errors or warnings."\

Additional overlap from the following excerpt:\

"Use of custom classes, objects,\
and OOP concepts."\

#### Complexity & Architechture (Embedded Code)

the following criteria is self explanitory, dont have the entire thing in a while loop:

"The embedded architecture used\
likely makes use of a\
combination of hard and soft\
interrupts with polling or perhaps,\
RTOS, multi-threading, event\
loops, etc."\

## 1 - Embedded System Proposal

Description: todo

Inputs: todo

Outputs: todo

Basic operation: todo
## 2 - Hardware Design

PCB Gerber files and Circuit Schematics will be included in repo

Component List:

STM32f74G-DISCO\
todo\

IR Sensor Datasheet: https://www.sparkfun.com/datasheets/Components/GP2Y0A21YK.pdf



## 3 - Embedded Code Implementation and Architechture
todo

## 4 - Evaluation 
todo
