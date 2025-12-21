# virtual-machine

so it started out as a virtual machine but then it took progress into a register based one and then finally became a chip 8 emulator with a parser in it 
my learnings so far

byte code interpreter 
first step of the puzzle something which translates my high level C langauge to a portable machine code type language using components like an instruction fetcher, decoder, and execution engine to read, decode, and execute each instruction, often utilizing a stack-based model for managing data 
works on a linear instruction flow increases predictability bettere for cpu caching
Also, with such type of programs, we have only two things we can do:
execute current instruction, jump to the next one ;
jump to an arbitrary position in the bytecode (useful for loops and conditions)


Virtual  Machine

use stack based or register based depending on the needs of the virtual machine, most emulators though use registers to store variables with data in separate unique named registers while using a large memory stack to set routines or call back pointers for different functionalities that might be present in the software that is coded using the given VM's framework. This is the common skeleton used for my chip based virtual machine.

CHIP 8 emulator

this emulator has 16 registries which can each be used to store and load data, the 16th registry in this emulator is known as the flag registry which is used in various other functions to flag whether a carry over in case of addition or a negative sign in case of subtraction or many other things which can't generally be expressed by the system's limitation. it uses SDL for both input and graphics and audio buffering mechanisms. The emulator has its own CPU cycle which ranges from 50-100 mhz and a timer clock which runs on 60hz mimicking a wall clock using the chrono library.

there is I variable which is used as a memory address pointer in the given emulator.
each instruction in the emulator is of a combined size of 16 bits which includes the functions indexing and the other operands required to utilise the function.

CHIP-8 has a single 4 KB memory space.
The interpreter occupies the lowest memory.
The built-in fontset is stored at address 0x50.
Programs begin at address 0x200 and may store both instructions and sprite data in program memory.
The index register I is used to point to sprite data when drawing.
Stack and registers are separate from main memory.

The I variable is used to store "sprite" memory, sprite is used to display graphics in early systems "A small bitmap (pixel pattern) stored in memory that can be drawn onto the screen. " the sprite in Chip 8 is bound to 8 bits due to hardware complexities, each bit of the 8 bit wide sprite has separate data for each column/bit and each bit represents a separate pixel.

sprites or the draw function in CHIP 8 uses XOR drawing (important) used due to monocrhome nature of the system
0 0 -> 0
1 0 -> 1
0 1  -> 1
1 1 -> 0
same then 0 different then 1, function of XOR its a bit level function
Short answer (corrected & precise)
gfx[] is the screen’s bit-mapped framebuffer.
 Each sprite is also a bit-mapped pattern.
 During Dxyn, each sprite bit is XORed with the corresponding gfx[] bit, which toggles the pixel ON or OFF and allows collision detection.
That is exactly how CHIP-8 graphics work.

SDL takes this input from the draw function which is stored in gfx[] and outputs it
SDL uses polling to pull input data from the OS to bring it on to the SDL cycle and ask for wether user input via input devices has occured or not

CHIP-8 VM
├── registers
├── memory
├── stack
├── timers
├── gfx[64×32]
└── keys[16]
        ↓
SDL (Host)
├── window
├── texture
├── keyboard events
└── audio callback


