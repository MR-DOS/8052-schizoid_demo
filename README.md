# 8052-schizoid_demo
My own improved implementation of so-called "Schizofreny" 8051 board.You can find the original version [here](http://www.bezstarosti.cz/jungle/schizofreny/schizofreny.htm). 

## Disclaimer
I would like to apologize to anyone not using Windows - this was quite an old project of mine, back when I used mostly Windows. It somehow survived being moved across several generations of my computers (and one dead HDD and one HDD encrypted due to ransomware). The sources of the terminal are either destroyed or at not-precisely-known location. I may find it someday, maybe not. It uses someone else's RS232 lib for Win/Unix, however i cannot give credit to the author due to the source files missing.
Since this is an old project, it will receive only very limited support from me.

## Brief info
### What this thingumabob consists of
Hardware: AT89C52 with 256B of internal RAM and bootloader, 64 kB of external ROM and 64 kB of external RAM (can be disabled).
Bootloader: Handles all of the programming part and communication with PC.
Terminal on PC: Communicates with the bootloader over USB. You just have to supply it with the HEX file to be programmed and number of the COM port. Originally, this was used to be integrated into MIDE (8051-centric dead simple IDE). Sorry for the Windows-only executable.

### How to use it
Hardware: Just build it and program the AT89C52 with bootloader (yeah, sucks if you have no programmer).
Bootloader: As said, flash it into the MCU.
Terminal: Use the batch script program.bat. It converts a HEX file into BIN file, sends it to the board and starts the program. It has two arguments, first is the name of the HEX file, second is number of COM port (just a number, like "5", not "COM5").

## Technical info
This "Schizofreny" inspired board uses 128 kB SRAM to emulate 64 kB of ROM and 64 kB of RAM. The program is sent to the board via virtual COM port (CH340G) over USB. Internal bootloader and dedicated circuitry around the 8052 (AT89C52, specifically) handles all of the access to the SRAM during programming. One of the RS232 control signals resets the CPU, another switches it between the bootloader and user mode. Each mode has its own crystal, so the board can be used at any frequency.

The virtual 64 kB RAM portion of the 128 kB SRAM can be disabled so it does not interfere with anything connected to the board. In this mode, ports P1 and P3 are completely free for the user. When the RAM is not disabled, P1 is free, but P3 has P3.6, P3.7 used for RD, WR (access to RAM). The board handles disconnecting of these pins from the header, so it does not interfere with user HW.
To sum it up, you have either 14 or 16 GPIO accessible dependng on the mode (set by jumper).

Sadly, the virtual ROM sits on ports P0 and P2, so it is not possible to use them for anything else. Since the 8052 has 256 bytes of RAM and Flash cannot be reflashed using bootloader, there is no other way to load the program. This is a limitation of the 8051 architecture.

### What was used for the project
The board was made in EAGLE. For those not willing to pay for this shitware to make this board, vector graphics with the board TOP and BOTTOM layers, as well as the schematic, are included.

The bootloader was written in C and can be compiled using SDCC. Do not even try with GCC (it does not even have a useable port for 8051, at least not one I'm aware of).

The terminal was written using C (GCC / MinGW) for Windows. I used some library for COM port, but since the sources are now lost, i cannot give credit to the author, sorry.
