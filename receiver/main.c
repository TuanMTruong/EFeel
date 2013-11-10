/*
 * main.c
 *
 * Created: 8/2/2013 9:38:56 AM
 *  Author: Tuan
 */ 

//includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Xmega.h"

/*
 * Plan of attack:
 * When the receiver first turns on it will initialize settings
 *	- 32 Mhz clock
 *	- USART
 *	- SPI
 *	- XBEE baud rate settings
 *	- XBEE network address
 *	- interrupts
 *	
 * After connecting receiver will set up receiver address
 *	read from the address dial
 * Receiver will attempt to connect to transmitter
 * If connection not established receiver will continue to try and connect.
 * 
 * When connected global interrupts will be enable.
 * receiver will continue to collect data and store it in a software FIFO
 * Display data in FIFO 
 *
 */


//global variables
//used to store address on receiver (errata: create functions to access address)
uint8_t myAddress;

//start main
int main(void)
{
	SystemInitializing(); //system checks (yup)
	//counter
	uint8_t i =0;
	//used to store incoming USART data 
	unsigned char temp;

	//Create address
	myAddress = PORTB.IN & 0x0F;
	USART_SendString_Debug("Receiver address = ");
	USART_SendByte_DEBUG_Blocking(myAddress);
	USART_SendByte_DEBUG_Blocking('\n');
	
	USART_WaitForTx(USART_XBEE);
	USART_SENDBYTE(USART_XBEE, (ADDNEW_RECV|myAddress));
	
	
    while(1)
    {
       USART_WaitForRx(USART_XBEE);
	   temp = USART_READBYTE(USART_XBEE);
	   if (temp == (ACK_HOST|myAddress)){
			USART_WaitForTx(USART_XBEE);
			USART_SENDBYTE(USART_XBEE, (ACK_RECV|myAddress));
			LED_PORT.OUT |= 1<<CONNECT_LED;
			break;
	   }
	   if(i>=100){
		   USART_WaitForTx(USART_XBEE);
		   USART_SENDBYTE(USART_XBEE, (ADDNEW_RECV|myAddress));
		   i=0;
	   }
	   i++;
    }
	
	sei();
	
	while (1)
	{
		// USART_WaitForRx(USART_XBEE);
		// temp = USART_READBYTE(USART_XBEE);
		// PORTA.OUT = temp;
		//PrintFIFO();
		if( FIFOverify()){
			SetEL();
			SetLED();
		}
	}
	return 0;
	
}
