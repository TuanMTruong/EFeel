/*
 * ElectricFeel_Receiver.c
 *
 * Created: 8/2/2013 9:38:56 AM
 *  Author: Tuan
 */ 

//includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Xmega.h"

/*
Plan of attack:
...
*/

uint8_t myAddress;

int main(void)
{
	SystemInitializing(); //system checks (yup)
	uint8_t i =0;
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