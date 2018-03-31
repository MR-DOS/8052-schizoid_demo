
/**************************************************

Simple non-interactive serial terminal which automatically handles
communication with the 8052 "Schizoid" board. It restarts the board,
send it new program, restarts it again and starts the program which
it sent.

It should be compilable both on Linux/Unix and Windows, though I never
tried it on *NIX system.

 **************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs232.h"

#define BAUD_RATE 57600 //define baudrate

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Expected parameter format as follows: \"file.bin -ser COMnumber\"");
        return (1);
    };

    unsigned char binary[65536]; //array for storage of 8051 binary
    FILE *bindata;
    bindata = fopen(argv[1], "r"); //handle to 8051 binary file
    //read binary file
    fread(binary, sizeof (binary[0]), sizeof (binary) / sizeof (binary[0]), bindata);
    fclose(bindata); //close file

    int i, n,
            cport_nr = atoi(argv[3]); //number of COMport
    char mode[] = {'8', 'N', '1', 0}; //mode of operation

    unsigned char buf[4096]; //serial receive buffer


    printf("\nLOCALTERM: Opening COMport.\n");
    if (RS232_OpenComport(cport_nr, BAUD_RATE, mode))
    {
        printf("\nLOCALTERM: Can not open comport\n");
        return (0);
    }

    RS232_disableDTR(cport_nr); //enable bootloader mode
    printf("\nLOCALTERM: Running in bootloader mode.\n");
#ifdef _WIN32
    Sleep(100); //Sleep for 100ms to make sure 8051 resets properly
#else
    usleep(100000);
#endif    
    RS232_disableRTS(cport_nr); //restart 8051
    printf("\nLOCALTERM: Resetting board.\n");

#ifdef _WIN32
    Sleep(100); //Sleep for 100ms to make sure 8051 resets properly
#else
    usleep(100000);
#endif
    RS232_enableRTS(cport_nr); //un-restart 8051    
    RS232_PollComport(cport_nr, buf, 4095); //receive data so that on beginning
    //there is no garbage on terminal output

    int run = 1; //variable used to break main while loop    

    while (run == 1)
    { //main loop, does all the work
        n = RS232_PollComport(cport_nr, buf, 4095); //read received serial data

        if (n > 0)
        {
            buf[n] = 0; // always put a "null" at the end of a string!
            printf((char *) buf);

            //Start programming board
            if (strstr(buf, "processor") != NULL)
            {
                printf("\nLOCALTERM: Sending request for code load.\n\n");
                RS232_SendByte(cport_nr, 'P');
            }

            //Start sending program
            if (strstr(buf, "stuffed") != NULL)
            {
                printf("\nLOCALTERM: Beginning data transfer.\n");
                /*for (i = 0; i<sizeof (binary); i++) {
                    RS232_SendByte(cport_nr, binary[i]);
                }*/
                RS232_SendBuf(cport_nr, binary, sizeof (binary));
                printf("\nLOCALTERM: Data transfer complete. Waiting for reaction.\n");
            }

            //Kill board after programming
            if (strstr(buf, "loaded") != NULL)
            {
                printf("\nLOCALTERM: Loading was successful.\n");
                printf("\nLOCALTERM: Killing board before reset.\n\n");
                RS232_SendByte(cport_nr, 'H');
            }

            //Report successful freeze, reset board
            if (strstr(buf, "freeze.") != NULL)
            {

                printf("\nLOCALTERM: Freezing was successful.\n");
                printf("\nLOCALTERM: Resetting board in user mode.\n");
#ifdef _WIN32
                Sleep(100);
#else
                usleep(100000);
#endif 
                RS232_disableRTS(cport_nr); //reset
#ifdef _WIN32
                Sleep(100);
#else
                usleep(100000);
#endif                    
                RS232_enableDTR(cport_nr); //enable usermode
#ifdef _WIN32
                Sleep(100);
#else
                usleep(100000);
#endif      
                RS232_enableRTS(cport_nr); //unreset
                run = 0;
            }
        }

#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000000); /* sleep for 1000 milliSeconds */
#endif
    }

    char ch;
    printf("\nLOCALTERM: Board running in external code mode. Press q to quit.\n");
    do
    {
        ch = _getch();
    }
    while (ch != 'q');
    return (0);
}

