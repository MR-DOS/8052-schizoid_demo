//--------------------------------Preprocessor definitions----------------------------------------//

#include <at89x52.h>
#include <stdint.h>
#include <string.h>
#define L 0
#define H 1
#define LOW 0
#define HIGH 1
#define TRUE 1
#define FALSE 0

//--------------------------------Global variables/-----------------------------------------------//

volatile __data unsigned char RXbuffer, TXbuffer;
volatile __bit newRX = 0, pend_char = 0, pend_string = 0, code_load=0;
volatile unsigned char const * sendstring_p = NULL;
volatile __data uint8_t sendstring_length;

volatile __xdata uint8_t * __data RAMptr = 0;		//pointer do xDATA area stored in iDATA area

const __code char *messages[2] = {message1, message2};
const __code char *intro[6] = {intro1, intro2, intro3, intro4, intro5, intro6};
const __code char intro1[] = "\r\n\r\n___  ___ _____       ______  _____ _____ _____    _____ _____ _____  _____ \r\n";
const __code char intro2[] = "|  \\/  |/  ___|      | ___ \\|  _  /  ___/  ___|  |  _  |  _  |  ___|/ __  \\\r\n";
const __code char intro3[] = "| .  . |\\ `--. ______| |_/ /| | | \\ `--.\\ `--.    \\ V /| |/' |___ \\ `' / /'\r\n";
const __code char intro4[] = "| |\\/| | `--. \\______| ___ \\| | | |`--. \\`--. \\   / _ \\|  /| |   \\ \\  / /  \r\n";
const __code char intro5[] = "| |  | |/\\__/ /      | |_/ /\\ \\_/ /\\__/ /\\__/ /  | |_| \\ |_/ /\\__/ /./ /___\r\n";
const __code char intro6[] = "\\_|  |_/\\____/       \\____/  \\___/\\____/\\____/   \\_____/\\___/\\____/ \\_____/\r\n\r\n";
const __code char message1[] = "MS-BOSS's 8052 dev board v 1.0, build 30/05/2016\r\n";
const __code char message2[] = "Send P to load code, send H to hang processor.\r\n";
const __code char loaded[] = "\r\n65535 bytes succesfully loaded\r\n\r\n";
const __code char freezemsg[] = "Going to freeze...\r\n";
const __code char loadmsg[] = "Ready to be stuffed!\r\n";

//---------------------------------Function definitions------------------------------------------------//

void ser_init(void)
{
	T2CON=0x30;	//use T2 as RS232 clock
	RCAP2H = 0xFF;	//set serial clock to 57600 Bd @ 11,0592 MHz
	RCAP2L = 0xFA;
	TH2 = 0xFF;	//set initial conditions
	TL2 = 0xFA;
	SCON=0x50;	//serial in mode 1 (8-bit UART), RX on
	
	if (TI) {TI=0;}	//clear the flags
	if (RI) {RI=0;}
	
	ES=1;		//enable serial interupt
	EA=1;		//globally enable interrupts
	
	TR2 = 1;	//enable T2
}

//------------------------------------------------------------------------------------------------//

void ser_int_handler(void) __interrupt 4 __using 1
//serial interrupt handler (int4), uses second R register bank
//handles sending and receiving individual characters or whole strings on background
//can be misused for sending other byte oriented types and their arrays
{
	ES=0;	//disable serial interrupt during this handler
	
	if (RI)	//if received a byte
	{
 		RI = 0;	//reset HW RX flag
 		RXbuffer = SBUF;//save the byte
 		newRX = 1;	//set the SW flag
 		if (code_load)	//if code loading is ON, save as program byte
 		{
 			*RAMptr = RXbuffer; //save the byte from RXbuffer to the place pointed to by the pointer 
 			RAMptr++;	//increase the pointer

 			if (RAMptr!=0)	//continue until the memory is fully loaded
 			{
 				newRX = 0;
 			} else //if fully loaded
 			{
 				code_load = 0;	//turn off code downloading
 				sendstring_p = &loaded[0];	//save the string address to the global variable
				sendstring_length = strlen(loaded)-1;	//save string length, THE -1 IS EXPERIMENTAL !!
				pend_string = 1;	//acknowledge the start of sending a string
				TI = 1;		//start the sending process
 				
 			}
 		}
	}
	
	if (TI)	//send a character
	{
 		TI = 0;	//reset the HW flag
 		
 		if (pend_char)	//if waiting to send a character
 		{
 			pend_char=0;	//delete the request
 			SBUF = TXbuffer;//send the character
 		}
 		
 		if (pend_string)	//if waiting to send a string
 		{
 			SBUF = *(sendstring_p++);	//send the character and increment pointer
 			sendstring_length--;		//decrement number of chars to be sent
 			if (sendstring_length == 0)	//if nothing else to send, delete request
 				{
 					pend_string = 0;	//delete request
 					sendstring_p = NULL;	//delete pointer
 				}
 		}
	}
  	
	ES=1;	//enable serial interrupt and exit
}

//------------------------------------------------------------------------------------------------//


void ser_send_char(unsigned const char TXchar)
//invokes a request to send a character on the background
{
	while (pend_string|pend_char) {}	//wait for previous requests to finish
	TXbuffer = TXchar;	//put the char into buffer
	pend_char = 1;		//acknowledge the request
	TI = 1;			//start the transfer - this starts the int4
}

//------------------------------------------------------------------------------------------------//

void ser_send_const_string(unsigned const char TXstring[])
//invokes a request for sending a string on the background
//the string HAS TO EXIST during the WHOLE time of the background operation
//Otherwise, garbage will be sent out.
{
	while (pend_string|pend_char) {}	//wait for previous requests to finish
	sendstring_p = &TXstring[0];		//save string address to global var
	sendstring_length = strlen(TXstring);	//save string length  to global var
	pend_string = 1;	//acknowledge the request
	TI = 1;		//start the transfer - this starts the int4
}

//------------------------------------------------------------------------------------------------//

void ser_send_string(unsigned const char TXstring[])
//this sends a string which does not exist after leaving the function
//use this for fucked-up strings which cease to exist after leaving the sending function
{
	uint8_t length = strlen(TXstring);	//get the length of the string
	uint8_t i=0;	//start sending from the first char
	for (; i <= length ; i++)
	{
		ser_send_char(TXstring[i]);	//iterate over the string
	}
}

//------------------------------------------------------------------------------------------------//

void clear_xdata(void)
//clear the external RAM. This is not neccessary, but it's good to do because there are
//certain dickheads who never readdatasheets and expect a SRAM to be filled with zeroes
//upon power-up. On the other hand, it could be changed fill the RAM with complete shit to make
//them scratch their heads.
{
	register __xdata uint8_t * clearpoint;	//pointer to xDATA stored in a register
	clearpoint=0;	//start at addr 0
	do
	{
		*(clearpoint++)=0;	//zero the byte and increment pointer
	}
	while (clearpoint!=0);	//iterate over the whole SRAM
}

//------------------------------------------------------------------------------------------------//

void prereset_die(void)
//this clears the whole iDATA region and then halts the uC until reset
//This is because resetting a 8051 causes it to reset the SFRs, but not iDATA RAM
//It is written in assembler since it deletes the whole iDATA and pDATA region including R registers
//which could make the C program run haywire. The asm prog needs no RAM for it to function, only
//the A register and R0 register.
{
	__asm
		clr EA		//disable interrupt to prevent a major fuckup
		mov A,0			//start with the first byte
		back:
			mov R0,A	//store the address in R0
			mov @R0,0	//write 0 at the address
			inc A		//increment addr
		jnz back
		
		diepoint:
			sjmp diepoint	//halt here
	__endasm;
}

//------------------------------------------------------------------------------------------------//

void main(void)
{
	ser_init();	//initialize serial at 57600 Bd 8N1
	
	P3_5 = 1;	//clear xDATA (virtual SRAM)
	clear_xdata();
	P3_5 = 0;	//clear xCODE (virtual ROM)
	clear_xdata();
	
	{		//send greetings to the serial port
		uint8_t i;
		for (i = 0; i<6; i++)
		{
			ser_send_const_string(intro[i]);
		}
	}
	
	{		//send initial messages
		uint8_t i;
		for (i = 0; i<2; i++)
		{
			ser_send_const_string(messages[i]);
		}
	}
	
	
	while (1)
	{
		while(newRX==0) {}	//wait for char
		newRX = 0;		//clear SW flag
		while(code_load){};	//wait for the code loading to stop if it has been enabled
		switch (RXbuffer) {	//determine what to do with it
			case 'P':	//initiate xCODE loading
				ser_send_const_string(loadmsg);	//acknowledge being ready for the transfer
				while (pend_string){};		//wait for any pending transfers to complete
				code_load = 1;			//acknowledge the beginning of code loading
				break;
			case 'H':
				ser_send_const_string(freezemsg);	//acknowledge the halting
				while (pend_string){};		//wait for any pending transfers to complete
				prereset_die();			//halt and don't catch fire
				break;
			default:
				ser_send_char(RXbuffer);	//send back unknown char
				if (RXbuffer=='\n') {		//complete incomplete newline
					ser_send_char('\r');
				}
				if (RXbuffer=='\r') {		//complete incomplete newline
					ser_send_char('\n');
				}
				break;				//this doubles a complete newline (only sent by Windows/DOS), doesn't matter
		}
	}
	
	while(1){}	//infinite loop, just for the sake
}