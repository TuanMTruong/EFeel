/*
 * xmega32aa4u_testing.c
 *
 * Created: 7/24/2013 11:08:12 AM
 *  Author: Tuan
 */ 

//includes
#include <avr/io.h>
#include "Xmega.h"
#include <util/delay.h>
#include <avr/interrupt.h>


/* 
Plan of Attack:
OK, so most of the functions that would make this sensible is in the transmitter.c and h file.
So be sure to check those files one. Ok on to the plan. After the program runs through and 
does the system checks (sounds cool huh). The software initializes the USART, interrupts, Xbee,
and anything else that is need. The LEDs on the transmitter will show the setup progress (freak out if one of the lights freeze).
After that the program will wait for a START_HEADER character from the computer, if so then read in the data and send it out 
with the address of the correct receiver. 

38 channels, 8 for EL wires, 30 for LEDs, 10 LEDs, 3 channel per LED (RGB)

Data format for each receiver [1 opening bye | 8 byte EL wire data | 30 LED data | 1 closing byte ]

*/

int main(void)
{

	SystemInitializing();		//system checks
	//counter to keep track of which receiver is currently being talked to 
	int CurrentRECV = 0;
	//a counter that is used to keep track of which channel of data packet is being sent out
	int counter = 0;
	
	//secondary counter if needed
	//int anothercounter = 0;
	
	unsigned char Tempdata = 0;
	
	sei();
	_delay_ms(200);
    while(1)
    {	
		
		
		//wait for START_HEADER from Vixen
		USART_WaitForRx(USART_USB)
		//save read data temporary 
		Tempdata = USART_READBYTE(USART_USB); 
		
		
		
		//check is data receive was starting command
		if (Tempdata == START_HEADER){
			
			//If there is Start header the transmitter will start checking for active receivers
			//This look will cycle through maximum receiver number
			for(CurrentRECV = 0; CurrentRECV< RECV_NUM; CurrentRECV++){
				
				//if the receiver is active ie. it is connected with the transmitter
				if (ActiveReceiver & ( 1<<CurrentRECV)){
					//Send opening byte and the address of receiver to tell receiver to prepare for data
					USART_SendByte_XBEE_Blocking(OPEN_RECV | CurrentRECV);
					
					//Read in and send out data packets [8 byte El wire | 30 byte LED wire ]
					for(counter=0; counter<ChannelNum; counter++){
						USART_WaitForRx(USART_USB)
						Tempdata = USART_READBYTE(USART_USB);
						//USART_SendByte_DEBUG_Blocking(Tempdata);
						USART_SendByte_XBEE_Blocking(Tempdata);
					}
					USART_SendByte_XBEE_Blocking(CLOSE_RECV | CurrentRECV);
				}
				else{
					for(counter=0; counter<ChannelNum; counter++){
						USART_WaitForRx(USART_USB)
						Tempdata = USART_READBYTE(USART_USB);
						
					}
				}
			}
			
		}
		

	}
}
