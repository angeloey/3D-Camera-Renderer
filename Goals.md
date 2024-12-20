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
