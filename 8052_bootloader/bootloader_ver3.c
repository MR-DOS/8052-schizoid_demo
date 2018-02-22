//--------------------------------Definice preprocesoru-------------------------------------------//

#include <at89x52.h>
#include <stdint.h>
#include <string.h>
#define L 0
#define H 1
#define LOW 0
#define HIGH 1
#define TRUE 1
#define FALSE 0

//--------------------------------Globalni promenne-----------------------------------------------//

volatile __data unsigned char RXbuffer, TXbuffer;
volatile __bit newRX = 0, pend_char = 0, pend_string = 0, code_load=0;
volatile unsigned char const * sendstring_p = NULL;
volatile __data uint8_t sendstring_length;

volatile __xdata uint8_t * __data RAMptr = 0;		//ukazatel na xDATA ulozeny v iDATA

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

//---------------------------------Definice funkci------------------------------------------------//

void ser_init(void)
{
	T2CON=0x30;	//pripoji RX a TX hodiny seriaku na T2
	RCAP2H = 0xFF;	//nastavi seriak na 57600 Bd pri 11,0592 MHz
	RCAP2L = 0xFA;
	TH2 = 0xFF;	//prednastavi pocatecni hodnoty
	TL2 = 0xFA;
	SCON=0x50;	//seriak v rezimu 1 (8-bit UART), zapnuty RX
	
	if (TI) {TI=0;}	//pro jistotu vycisti flagy
	if (RI) {RI=0;}
	
	ES=1;		//zapne serial interrupt
	EA=1;		//zapne globalne interrupty
	
	TR2 = 1;	//zapne T2
}

//------------------------------------------------------------------------------------------------//

void ser_int_handler(void) __interrupt 4 __using 1
//obsluha preruseni seriaku (int4), pouziva druhou banku R registru
//stara se o vysilani jednotlivych znaku nebo celych stringu na pozadi
//lze zneuzit i na vysilani libovolnych jinych bajtove orientovanych typu a jejich poli
{
	ES=0;	//vypne serial interrupt po dobu obsluhy interruptu
	
	if (RI)	//ulozi prijaty znak a nahodi flag
	{
 		RI = 0;	//resetuje hard flag o prijeti
 		RXbuffer = SBUF;	//ulozi prijaty znak
 		newRX = 1;	//nahodi soft flag
 		if (code_load)	//pokud je zapnute nacitani kodu, udela to
 		{
 			*RAMptr = RXbuffer; //bajt z RXbufferu nacpe na soucasny bajt 
 			RAMptr++;
 			if (RAMptr!=0)	//pokracuje dokud se nenaplni cela pamet
 			{
 				newRX = 0;
 			} else
 			{
 				code_load = 0;	//vypne download
 				sendstring_p = &loaded[0];	//ulozi adresu stringu do globalni promenne
				sendstring_length = strlen(loaded)-1;	//ulozi delku stringu do globalni promenne , POKUS s -1!!
				pend_string = 1;	//oznami zacatek prenosu stringu
				TI = 1;		//spusti prenos
 				
 			}
 		}
	}
	
	if (TI)	//odesle znak pripraveny k odeslani
	{
 		TI = 0;	//resetuje hard flag o odeslani
 		
 		if (pend_char)	//obslouzi pozadavek na odeslani znaku
 		{
 			pend_char=0;	//odstrani pozadavek na odeslani znaku
 			SBUF = TXbuffer;	//odesle znak
 		}
 		
 		if (pend_string)	//obslouzi pozadavek na odeslani stringu
 		{
 			SBUF = *(sendstring_p++);	//odesle aktualni znak a postinkrementuje ukazatel
 			sendstring_length--;	//dekrementuje pocet znaku k vyslani
 			if (sendstring_length == 0)	//pokud neni co vyslat, odstrani pozadavek a pointer
 				{
 					pend_string = 0;	//odstrani pozadavek
 					sendstring_p = NULL;	//odstrani pointer
 				}
 		}
	}
  	
	ES=1;	//obnovi preruseni seriaku
}

//------------------------------------------------------------------------------------------------//


void ser_send_char(unsigned const char TXchar)
//Procedura slouzi k odeslani znaku po seriaku.
{
	while (pend_string|pend_char) {}	//pocka na dokonceni predchozich prenosu
	TXbuffer = TXchar;	//nacpe znak do jednobajtoveho bufferu
	pend_char = 1;	//oznami zacatek prenosu znaku
	TI = 1;		//spusti prenos
}

//------------------------------------------------------------------------------------------------//

void ser_send_const_string(unsigned const char TXstring[])
//Procedura slouzi k vyvolani prenosu stringu po seriaku na pozadi.
//String MUSI EXISTOVAT po celou dobu prenosu na pozadi. Jinak poleze ven bordel.
{
	while (pend_string|pend_char) {}	//pocka na dokonceni predchozich prenosu
	sendstring_p = &TXstring[0];	//ulozi adresu stringu do globalni promenne
	sendstring_length = strlen(TXstring);	//ulozi delku stringu do globalni promenne
	pend_string = 1;	//oznami zacatek prenosu stringu
	TI = 1;		//spusti prenos
}

//------------------------------------------------------------------------------------------------//

void ser_send_string(unsigned const char TXstring[])
//procedura slouzici k preneseni stringu jako rady znaku. Vhodne pro pripad kretenskych stringu,
//ktere zaniknou hned nebo kratce po navratu z odesilaci funkce.
{
	uint8_t length = strlen(TXstring);	//zjisti delku stringu
	uint8_t i=0;	//zacne posilat od prvniho znaku
	for (; i <= length ; i++)
	{
		ser_send_char(TXstring[i]);	//posle postupne cely string
	}
}

//------------------------------------------------------------------------------------------------//

void clear_xdata(void)
//kompletne vynuluje externi RAM, neni to povinnost, ale ze slusnosti vuci lidem, kteri
//nectou datasheety a maji v hlave kulicky a ocekavaji, ze SRAMka po zapnuti bude cista
{
	register __xdata uint8_t * clearpoint;	//pointer na xdata ulozeny v registru
	clearpoint=0;	//zacne se v 0
	do
	{
		*(clearpoint++)=0;	//vynuluje aktualni bajt a postinkrementuje ukazatel
	}
	while (clearpoint!=0);	//az projde celou RAM
}

//------------------------------------------------------------------------------------------------//

void prereset_die(void)
//Procedura slouzi k vynulovani idata a zacykleni procesoru, musi predchazet restartu.
//restart u 8051 totiz resetuje SFR, ale ne idata RAM
//Neni dobry to psat v Cecku, protoze nasilne hrabeme do cely idata RAMky vcetne R registru.
{
	__asm
		clr EA		//vypneme interrupty, aby nemohl program nekam jeste skakat
		mov A,0			//vynulujem cely prostor idata
		back:
			mov R0,A
			mov @R0,0
			inc A
		jnz back
		
		diepoint:
			jz diepoint	//zacykleni, nekonecne
			jnz diepoint	//vono preci jen, posrat se muze ledacos, ze jo
	__endasm;
}

//------------------------------------------------------------------------------------------------//

void main(void)
{
	ser_init();	//inicializuje seriak na 19200 Bd 8N1
	
	P3_5 = 1;	//vycistime oblast xDATA
	clear_xdata;
	P3_5 = 0;	//vycistime oblast xCODE
	clear_xdata;
	
	{		//posle na seriak uvitaci logo
		uint8_t i;
		for (i = 0; i<6; i++)
		{
			ser_send_const_string(intro[i]);
		}
	}
	
	{		//posle na seriak uvodni zvasty
		uint8_t i;
		for (i = 0; i<2; i++)
		{
			ser_send_const_string(messages[i]);
		}
	}
	
	
	while (1)	//nekonecna smycka
	{
		while(newRX==0) {}	//ceka na znak
		newRX = 0;	//smaze soft flag
		while(code_load){};	//pro jistotu pocka na dokonceni nacitani xCODE
		switch (RXbuffer) {	//rozhodne se, co s tim znakem
			case 'P':	//iniciuje nacitani xCODE
				ser_send_const_string(loadmsg);	//potvrdi pripravenost na data
				while (pend_string){};	//pocka na dokonceni predchozich prenosu
				code_load = 1;	//oznami zacatek nacitani souboru
				break;
			case 'H':
				ser_send_const_string(freezemsg);	//potvrdi zatuhnuti
				while (pend_string){};	//pocka na dokonceni predchozich prenosu
				prereset_die();	//zatuhne
				break;
			default:
				ser_send_char(RXbuffer);	//vrati neznamy znak
				if (RXbuffer=='\n') {		//dokonci nekompletni newline
					ser_send_char('\r');
				}
				if (RXbuffer=='\r') {		//dokonci nekompletni newline
					ser_send_char('\n');
				}
				break;				//kompletni newline se tim zdvoji, to nas ale nesere
		}
	}
	
	while(1){}	//zacykli program na konci. Jestli nekdo tenhle radek smaze, necht
			//je povesen za obnazene gonady na rezavy hak do pruvanu, kde na nej bude
			//kapat voda! Pak by totiz program bezel dal pameti a mohl se chovat
			//chaoticky nebo se vratit zpet na zacatek.
			//i kdyz... ted uz neni potreba, jenze kdyz tu nebude nic, tak SDCC
			//nehodi warning...
}