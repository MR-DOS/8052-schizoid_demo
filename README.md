# 8052-schizoid_demo
My own improved implementation of so-called "Schizofreny" 8051 board.You can find the original version [here](http://www.bezstarosti.cz/jungle/schizofreny/schizofreny.htm). 

## Disclaimer
I would like to admit this is a very old project back when I was rather beginner at programming. It somehow survived (or, better to say, did not) being moved across several generations of my computers (about 6 notebooks, 3 desktops, one dead HDD and one HDD encrypted due to ransomware) ranging from PIII-M to 3rd gen i5. I had to visit my parents and search my old computer to get the sources of the PC part of the SW, which I thought have been destroyed long ago.

Since this is an old project, it will receive only very limited support from me.
On the other hand, the project worked and was *usable*. Well, if you call uC with no debug at all usable, that is.

## Brief info
### What this thingumabob consists of
Hardware: AT89C52 with 256B of internal RAM and bootloader (you can in fact use any 8051, just not 80C31), 64 kB of external ROM and 64 kB of external RAM (can be disabled).
Bootloader: Handles all of the programming part and communication with PC via virtual COM port over USB.

Terminal on PC: Communicates with the bootloader over USB. You just have to supply it with the HEX file to be programmed and number of the COM port. Originally, this was used to be integrated into MIDE (8051-centric dead simple IDE).

### How to use it
Hardware: Just build it, no calibration needed.

Bootloader: Program the 80C52 with the bootloader (you need to have a programmer).

Terminal: Use the batch script program.bat. It converts a HEX file into BIN file, sends it to the board and starts the program. It has two arguments, first is the name of the HEX file, second is number of COM port (just a number, like "5", not "COM5").

### MIDE-51 integration
I originally used MIDE-51 for development of the bootloader. Now, I would not recommend the IDE to anyone since it has no autocompletion and next to no connection with the toolchain. However if you wish to use it, here is how to set it up.
![MIDE Setting](https://github.com/MR-DOS/8052-schizoid_demo/blob/master/PICS/mide_setting.png)
* The line with name "execute file" should point to program.bat (8052term.exe and hex2bin.exe have to be in the same directory).
* Parameter 2 specifies number of COM port to which the demoboard is connected.
* MIDE-51 can be downloaded from [here](http://www.opcube.com/home.html#MIDE51).

## Technical info
This "Schizofreny" inspired board uses 128 kB SRAM to emulate 64 kB of ROM and 64 kB of RAM. The program is sent to the board via virtual COM port (CH340G) over USB. Internal bootloader and dedicated circuitry around the 8052 (AT89C52, specifically) handles all of the access to the SRAM during programming. One of the RS232 control signals resets the CPU, another switches it between the bootloader and user mode. Each mode has its own crystal, so the board can be used at any frequency.

The virtual 64 kB RAM portion of the 128 kB SRAM can be disabled so it does not interfere with anything connected to the board. In this mode, ports P1 and P3 are completely free for the user. When the RAM is not disabled, P1 is free, but P3 has P3.6, P3.7 used for RD, WR (access to RAM). The board handles disconnecting of these pins from the header, so it does not interfere with user HW.
To sum it up, you have either 14 or 16 GPIO accessible dependng on the mode (set by jumper).

Sadly, the virtual ROM sits on ports P0 and P2, so it is not possible to use them for anything else. Since the 8052 has 256 bytes of RAM and Flash cannot be reflashed using bootloader, there is no other way to load the program. This is a limitation of the 8051 architecture.

### What was used for the project
The board was made in EAGLE. For those not willing to pay for this shitware to make this board, vector graphics with the board TOP and BOTTOM layers, as well as the schematic, BOM and drawing of placement of parts, are included.

The bootloader was written in C and can be compiled using SDCC. Do not even try with GCC (it does not even have a useable port for 8051, at least not one I'm aware of).

The terminal was written using C (GCC / MinGW) for Windows. It uses RS-232 library from Teunis van Beelen, licensed under GNU GPL, can be found [here](https://www.teuniz.net/RS-232/index.html). If you would like to compile the terminal yourself, you should first check the author's website whether there is a new version of the library.

### Where to find the resources
* The Eagle project is in **HW_eagle**
* The preview of the board, BOM, top and bottom copper layers in PostScript, PDF and SVG and schematics are in **HW_makedata**
* The bootloader is located in **SW_8052_bootloader**. The BIN and HEX files are located in **SW_8052_bootloader/bin/**, sources are in **SW_8052_bootloader/src/**
* The terminal is located in **8052-schizoid_demo/SW_PC_terminal**. The binaries are in **8052-schizoid_demo/SW_PC_terminal/bin**, the sources are in **8052-schizoid_demo/SW_PC_terminal/src**

## Known bugs
* Upon sending the binary to the board, the board responds with some messages saying that the transfer completed. However, prior to that, the board sends one seemingly random character. This is a bug in the main while(1) loop where it waits upon completion of the code loading and then it parses the last character in the buffer as unknown control character and thus is sent back. It has no effect on functionality, but it looks stupid. This should be corrected later.
* There is no image of the finished board. I will add some later.
* It looks like the SVG files are shown correctly in Inkscape, but not in LXDE default image viewer. This should be checked.