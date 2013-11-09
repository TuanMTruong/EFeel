/*
 * Xmega.c
 *
 * Created: 8/1/2013 7:32:34 PM
 *  Author: Tuan
 */ 

//includes
#include <avr/io.h>
#include "Xmega.h"
#include <util/delay.h>
#include <avr/interrupt.h>

/*
 * This file holds the mini functions used in the transmitter code. It keeps
 * things clean and I will warn thee now that there are some functions that are
 * not used but were created for testing purpose (I call them junk-func). 
 * I also write comments in a weird way because it entertains me, me not you. So
 * apologies in advance to the reader.
 * Also you might find random curly bracket blocks like this:
 * {
 * 
 * }
 * those are solely for my use. It allows me to collapse a chunk of my code in Atmel Studio.
 * Just ignore them...
 */

/************************************************************************/
/* Gloabal Variables                                                    */
/************************************************************************/
//Hold receive and transmit data temporarily (thinking of creating a FIFO but haven't gotten to it)
volatile uint8_t ReceiverData;
volatile unsigned char TransmitterData;
//Un-used, temporary miscellaneous variable if need to track something (if used document (yes i know i could just delete it but i choose not to))
volatile uint8_t dataFlag = 0;
unsigned char dataTable[8];
//16 bits, 1 bit for receiver (0 = off, 1 = on), address of receiver is the bit position
uint16_t ActiveReceiver = 0;
//Max number of channels from vixen PER RECEIER!, set to 38, 8 for EL wire, 30 for LEDs with 3 per strips (RGB) making 10 total strips
//228 channels total for 6 receiver
uint8_t ChannelNum = 38; //i might change this to a #define 

/************************************************************************/
/* Set up 32MHZ Clock                                                   */
/************************************************************************/
void setClockTo32MHz() {
	CCP = CCP_IOREG_gc;              // disable register security for oscillator update
	OSC.CTRL = OSC_RC32MEN_bm;       // enable 32MHz oscillator
	while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator to be ready
	CCP = CCP_IOREG_gc;              // disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // switch to 32MHz clock
}

/************************************************************************/
/* USART Functions                                                      */
/************************************************************************/

	
//Sets the Baudrate 57600 => BSEL=1079, BSCALE=-5
//Sets the Baudrate 9600 => BSEL=12, BSCALE=4

//sets up the Debug USART 8Bit, 1SB, NO parity
void SetupUSART_DEBUG(int BSEL_VALUE, int BSCALE_VALUE){
	USART_DEBUG.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
	USART_DEBUG.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;
	USART_DEBUG.CTRLA |= USART_RXCINTLVL_HI_gc;
		
	USART_Baudrate_Set(USART_DEBUG, BSEL_VALUE, BSCALE_VALUE);
	
	USART_DEBUG_PORT.DIRSET = USART_TXPIN;
	USART_DEBUG_PORT.DIRCLR = USART_RXPIN;
}

//sets up the Xbee USART 8Bit, 1SB, NO parity
void SetupUSART_XBEE(int BSEL_VALUE, int BSCALE_VALUE){
	USART_XBEE.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
	USART_XBEE.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;
	USART_XBEE.CTRLA |= USART_RXCINTLVL_MED_gc | USART_TXCINTLVL_MED_gc; //| USART_DREINTLVL_MED_gc;
	
	USART_Baudrate_Set(USART_XBEE, BSEL_VALUE, BSCALE_VALUE);
	
	USART_XBEE_PORT.DIRSET = USART_TXPIN;
	USART_XBEE_PORT.DIRCLR = USART_RXPIN;
}

//sets up the USB USART 8Bit, 1SB, NO parity
void SetupUSART_USB(int BSEL_VALUE, int BSCALE_VALUE){
	USART_USB.CTRLB |= USART_RXEN_bm | USART_TXEN_bm |USART_CLK2X_bm;
	USART_USB.CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;
	USART_USB.CTRLA |= USART_RXCINTLVL_HI_gc; //| USART_DREINTLVL_MED_gc;
	
	USART_Baudrate_Set(USART_USB, BSEL_VALUE, BSCALE_VALUE);
	
	USART_USB_PORT.DIRSET = USART_TXPIN;
	USART_USB_PORT.DIRCLR = USART_RXPIN;
}

/************************************************************************/
/* Send and Receive Functions for Xbee and Debug (BLOCKING)             */
/************************************************************************/

/***XBEE - Sending byte BLOCKING***/
void USART_SendByte_XBEE_Blocking (uint8_t data){
	USART_WaitForTx(USART_XBEE);
	USART_SENDBYTE(USART_XBEE, data);
	return;
}

/***XBEE - Reading byte BLOCKING***/
uint8_t USART_ReadByte_XBEE_Blocking (){
	USART_WaitForRx(USART_XBEE);
	return USART_READBYTE(USART_XBEE);
}

/***XBEE - Sending String BLOCKING***/
void USART_SendString_XBEE(char* str){
	while (*str)
	{
		USART_WaitForTx(USART_XBEE);
		USART_SENDBYTE(USART_XBEE, *str);
		str++;
	}
}

/***USB - Sending byte BLOCKING***/
void USART_SendByte_DEBUG_Blocking (uint8_t data){
	USART_WaitForTx(USART_DEBUG);
	USART_SENDBYTE(USART_DEBUG, data);
	return;
}

/***USB - Reading byte BLOCKING***/
uint8_t USART_ReadByte_DEBUG_Blocking (){
	USART_WaitForRx(USART_DEBUG);
	return USART_READBYTE(USART_DEBUG);
}

/***USB - Sending String BLOCKING***/
void USART_SendString_Debug(char* str){
	while (*str)
	{
		USART_WaitForTx(USART_DEBUG);
		USART_SENDBYTE(USART_DEBUG, *str);
		str++;
	}
}



/************************************************************************/
/* Send data to receiver									            */
/************************************************************************/
void SendDatapack(unsigned char addr, unsigned char data){
	//sends a packet of data to a receiver with the specified address, 8bit address and 8bit data
	//combine address of receiver with the command to get ready to receive data
	addr = addr | ReadyData;
	USART_SendByte_XBEE_Blocking(addr);		//send address and command
	USART_SendByte_XBEE_Blocking(data);		//send data
}

/************************************************************************/
/* Debug commands											            */
/************************************************************************/
void PrintActiveReceivers(){
	//Loops through active receiver variable letting know if each receiver is active
	int i = 0;
	//LOOOOPP
	for(i=0; i<8 ; i++){ //only accounts for 8 receiver, easily changeable with a #define
		USART_SendString_Debug("Receiver ");
		USART_SendByte_DEBUG_Blocking(i+48);
		USART_SendString_Debug(": ");
		if (ActiveReceiver & ( 1<<i))
		{
			USART_SendString_Debug("Active\n");
		}
		else {
			USART_SendString_Debug("InActive\n");
		}
	}
	return;
}

/************************************************************************/
/* Set up DDR (not Dance Dance Revolution)                              */
/************************************************************************/
void SetupDDR(){
	LED_PORT.DIRSET = 0xff;
}

/************************************************************************/
/* Channel calculator via Vixen                                         */
/************************************************************************/
//Un-used function that was created during prototyping to count the number of channels
//Channel numbers are set
int CalculateChannelNum(){
	int count = 0;
	while (!(ReceiverData == START_HEADER))
	{
		ReceiverData = USART_ReadByte_DEBUG_Blocking();
	}

	while(!(ReceiverData == END_HEADER)){
		ReceiverData = USART_ReadByte_DEBUG_Blocking();
		count++;
	}
	return count;
	
}


/************************************************************************/
/* USART interrupt routines                                             */
/************************************************************************/


ISR(USART_XBEE_RXVECT)
{
	ReceiverData = USART_READBYTE(USART_XBEE);
	if ((ReceiverData & 0xF0)==ADDNEW_RECV){
		USART_SendByte_XBEE_Blocking(ACK_HOST| (ReceiverData& 0x0f));
		USART_SendString_Debug("CH Request\n");
	}
	if ((ReceiverData & 0xF0)==ACK_RECV)
	{
		ActiveReceiver |= (1<< (ReceiverData& 0x0f));
		LED_PORT.OUT = ActiveReceiver;
		
		USART_SendString_Debug("Request Confirmed\n");
		USART_SendString_Debug("Receiver: ");
		USART_SendByte_DEBUG_Blocking((ReceiverData& 0x0f)+48);
		USART_SendByte_DEBUG_Blocking('\n');
	}
}

ISR(USART_XBEE_TXVECT){
//	USART_SendByte_XBEE_Blocking(0xFF);
}

ISR(USART_DEBUG_RXVECT){
	/*
	USART_SendString_Debug("\nReceivers:\n");
	ReceiverData = USART_READBYTE(USART_DEBUG);
	if(ReceiverData == CheckReceiverTable){
		PrintActiveReceivers();
	}
	*/
	ReceiverData = USART_READBYTE(USART_DEBUG);
	USART_WaitForTx(USART_DEBUG);
	USART_SENDBYTE(USART_DEBUG, ReceiverData);
	USART_SendByte_XBEE_Blocking(ReceiverData);
	LED_PORT.OUT++;
}

ISR(USART_USB_RXVECT){
	//USART_SendString_Debug("\nReceivers:\n");
	
	//ReceiverData = USART_READBYTE(USART_USB);
	//if(ReceiverData == START_HEADER){
		//USART_WaitForTx(USART_USB);
		//USART_SENDBYTE(USART_USB, ReceiverData);
	//	USART_SendByte_XBEE_Blocking(ReceiverData);
	//	LED_PORT.OUT++;
	//}
}



ISR(__vector_default){
	//reti;
}



/************************************************************************/
/* Xbee set up functions                                                */
/************************************************************************/


int CheckXbeeBaud(uint32_t CHECKBAUD){
	
	if (CHECKBAUD == 57600){
		USART_Baudrate_Set(USART_XBEE,1079, -5);
		USART_SendString_Debug("Check if Xbee BD is 57600\n");
	}
	else if (CHECKBAUD == 9600){
		USART_Baudrate_Set(USART_XBEE, 12, 4);
		USART_SendString_Debug("Check if Xbee BD is 9600\n");
	}
	else {
		USART_SendString_Debug("No matching Baud rate found\n");
		return 0;
	}
	
	USART_SendString_Debug("Opening Xbee \n");
	USART_SendString_XBEE("+++");
	

	if (XbeeOK())
	{
		USART_SendString_Debug("Xbee now Open\n");
		if (CHECKBAUD == 9600)
		{
			USART_SendString_Debug("Xbee BD = 9600\n");
		}
		if (CHECKBAUD == 57600)
		{
			USART_SendString_Debug("Xbee BD = 57600\n");
		}
		USART_SendString_Debug("Closing Xbee settngs\n");
		USART_SendString_XBEE("ATCN\r");
		XbeeOK();
			
		return 1;
	}
	USART_SendString_Debug("Unable to open\n");

	return 0;

}

int UpdateXbeeBaud(){
	
	USART_SendString_Debug("\nPreparing Xbee BD update to 57600\n");
	USART_Baudrate_Set(USART_XBEE, 12, 4);
	USART_SendString_Debug("Opening Xbee\n");
	USART_SendString_XBEE("+++");
	if (!XbeeOK())
		return 0;
	
	USART_SendString_Debug("Xbee BD change to 115200\n");	
	_delay_ms(5);
	USART_SendString_XBEE("ATBD6\r");

	
	if(!XbeeOK())
		return 0;
	USART_SendString_Debug("Write new BD to Xbee\n");
	USART_SendString_XBEE("ATWR\r");
	
	if(!XbeeOK())
		return 0;
	
	USART_SendString_Debug("Closing Xbee settings\n");
	USART_SendString_XBEE("ATCN\r");
	
	if(XbeeOK()){
		USART_Baudrate_Set(USART_XBEE, 1047, -6);	
	return 1;

	}
	else {
		return 0;
	}
}

int XbeeOK(){
	
	char TempRev[5];

	uint32_t i = 0;
	int j = 0;
	uint8_t k =0;
	TempRev[0] = 0;
	TempRev[1] = 0;
	char TheOK[2];
	
	while (k < 25)
	{
		if (USART_XBEE.STATUS & USART_RXCIF_bm)
		{
			TempRev[j] = USART_XBEE.DATA;
			j++;
			if (j>=5)
			{
				break;
			}
			
		}
		else{
			i++;
		}
		if (i>115200){
			i = 0;
			k++;
		}
	}
	
	for(i=0; i<5; i++){
		if(TempRev[i] == 'O'){
			TheOK[0] = 'O';
		}
		if (TempRev[i] == 'K'){
			TheOK[1] = 'K';
		}
		
	}
	if (TheOK[0] == 'O' && TheOK[1]== 'K')
	{
		USART_SendString_Debug("Xbee: OK\n");
		
		
		return 1;
		
	}
	USART_SendString_Debug("Xbee: ERROR\n");
	

	return 0;
}


int CheckXbeeNetID(char *ID_data){
	char TempRev[5];
	USART_SendString_Debug("Checking Xbee Network ID\n");
	USART_SendString_Debug("Opening Xbee\n");
	USART_SendString_XBEE("+++");
	if (!XbeeOK())
		return 0;
	USART_SendString_XBEE("ATID\r");
	
	TempRev[0] = USART_ReadByte_XBEE_Blocking();
	TempRev[1] = USART_ReadByte_XBEE_Blocking();
	TempRev[2] = USART_ReadByte_XBEE_Blocking();
	TempRev[3] = USART_ReadByte_XBEE_Blocking();
	TempRev[4] = USART_ReadByte_XBEE_Blocking();
	
	if (TempRev[0] == *ID_data && TempRev[1] == *(ID_data+1) && TempRev[2] == *(ID_data+2) && TempRev[3] == *(ID_data+3)){
		USART_SendString_Debug("Correct Network ID\n");
		USART_SendString_Debug("Closing Xbee\n");
		USART_SendString_XBEE("ATCN\r");
		if (XbeeOK())
		{
			return 1;
		}
		else {
			USART_SendString_Debug("Error with Xbee communication =( \n");
			while(1){}
		}
		
	}
	USART_SendString_Debug("Incorrect Network ID\n");
	USART_SendString_Debug("Closing Xbee\n");
	USART_SendString_XBEE("ATCN\r");
	if (XbeeOK())
	{
		return 0;
	}
	USART_SendString_Debug("Something went horribly wrong... Sorry.\n");
	while(1){}
	
}

int UpdateXbeeID(){
	USART_SendString_Debug("Updating Xbee Network ID\n");
	
	USART_SendString_Debug("Opening Xbee\n");
	USART_SendString_XBEE("+++");
	if (!XbeeOK())
		return 0;
	USART_SendString_Debug("Setting new Network ID\n");
	USART_SendString_XBEE("ATID");
	USART_SendString_XBEE(NET_ID);
	USART_SendByte_XBEE_Blocking('\r');
	if (!XbeeOK())
		return 0;
	USART_SendString_Debug("Writing ID to Xbee\n");
	USART_SendString_XBEE("ATWR\r");
	
	if(!XbeeOK())
		return 0;
	USART_SendString_Debug("Closing Xbee settings\n");
	USART_SendString_XBEE("ATCN\r");
	if(XbeeOK()){
		USART_SendString_Debug("Xbee ID Updated\n");
		return 1;
	}
	return 0;
}


/************************************************************************/
/* Initial system check and setup                                       */
/************************************************************************/

int SystemInitializing(){
	int i = 0;
	SetupDDR();
	LED_PORT.OUT = 0x00;
	LED_PORT.OUT |= (1<<i);
	i++;
	
	setClockTo32MHz();
	LED_PORT.OUT |= (1<<i);
	
	i++;
	
	SetupUSART_DEBUG(1047,-6);
	SetupUSART_XBEE(1079,-5);
	SetupUSART_USB(3301,-5);
	
	LED_PORT.OUT |= (1<<i);
	
	i++;
	USART_SendString_Debug("***SYSTEM INITIALIZING***\n");
	USART_SendString_Debug("DDR Initialized\n");
	USART_SendString_Debug("32MHZ Clock Initialized\n");
	USART_SendString_Debug("USART Initialized\n");
	
	ENABLE_PMIC();
	LED_PORT.OUT |= (1<<i);
	i++;
	
	USART_SendString_Debug("PMIC Initialized\n");
	
	USART_SendString_Debug("Starting Xbee setup\n");
	if (CheckXbeeBaud(57600))
		USART_SendString_Debug("Xbee at correct BaudRate\n\n");
	else if(CheckXbeeBaud(9600))
		UpdateXbeeBaud();
	else {
		USART_SendString_Debug("Xbee at unknown BaudRate\n");
		USART_SendString_Debug("ERROR\n");
		while (1){};
	}
	LED_PORT.OUT |= (1<<i);
	i++;
	
	if (!CheckXbeeNetID(NET_ID))
	{
		UpdateXbeeID();
	}
	LED_PORT.OUT |= (1<<i);
	i++;
	//sei();
	USART_SendString_Debug("Xbee setup = Finish\n");
	LED_PORT.OUT |= (1<<i);
	i++;

	LED_PORT.OUT |= (1<<i);
	_delay_ms(500);
	i++;
	LED_PORT.OUT = 0x00;
	_delay_ms(200);
	LED_PORT.OUTTGL = 0xff;
	_delay_ms(200);
	LED_PORT.OUTTGL = 0xff;
	_delay_ms(200);
	LED_PORT.OUTTGL = 0xff;
	_delay_ms(200);
	LED_PORT.OUTTGL = 0xff;
	LED_PORT.OUT = 0x00;
	USART_SendString_Debug("***SYSTEM SETUP COMPLETE***\n");
	return 1;
}