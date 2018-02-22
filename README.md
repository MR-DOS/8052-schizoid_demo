# 8052-schizoid_demo
My own improved implementation of so-called "Schizofreny" 8051 board.You can find the original version [here](http://www.bezstarosti.cz/jungle/schizofreny/schizofreny.htm)

This version uses 128 kB SRAM to emulate 64 kB of ROM and 64 kB of RAM. The program is sent to the board via virtual COM port (CH340G). Internal bootloader of the 8052 (also included) handles all of the access to the SRAM during programming. One of the RS232 control signals resets the CPU, another switches it between the bootloader and user mode. Each mode has its own crystal, so the board can be used at any frequency.

The virtual RAM portion of the 128 kB SRAM can be disabled so it does not interfere with anything connected to the board. In this mode, ports P1 and P3 are completely free for the user. When the RAM is not disabled, P1 is free, but P3 has P3.6, P3.7 used for RD, WR (access to RAM). The board handles disconnecing of these pins from the header, so it does not interfere with user HW.


