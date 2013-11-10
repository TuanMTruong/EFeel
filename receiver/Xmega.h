/*
 * Xmega.h
 *
 * Created: 8/1/2013 7:32:50 PM
 *  Author: Tuan
 */ 


#ifndef XMEGA_H_
#define XMEGA_H_


/************************************************************************/
/* F_CPU CLOCK                                                          */
/************************************************************************/
#define F_CPU 32000000UL

/************************************************************************/
/* Physical PORT and PINS defined                                        */
/************************************************************************/

#define USART_DEBUG USARTC0
#define USART_DEBUG_PORT PORTC

#define USART_XBEE USARTE0
#define USART_XBEE_PORT PORTE
#define USART_XBEE_RXVECT USARTE0_RXC_vect

//#define USART_USB USARTC0
//#define USART_USB_PORT PORTC
//#define USART_USB_RECVECT USARTE0_RXC_vect

#define USART_RXPIN PIN2_bm
#define USART_TXPIN PIN3_bm

#define LED_PORT PORTC
#define EL_PORT PORTA
#define CONNECT_LED 0

#define SPI_LED1 SPIC
#define SPI_LED2 SPID

#define SPI_LED1_PORT PORTC
#define SPI_LED2_PORT PORTD

#define MOSI_bm PIN5_bm
#define SCK_bm PIN7_bm 

/************************************************************************/
/* USART and Interrupt settings								            */
/************************************************************************/
#define ENABLE_PMIC() PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

#define USART_WaitForTx(_usart) while(!(_usart.STATUS & USART_DREIF_bm)){}
#define USART_SENDBYTE(_usart, _data) _usart.DATA = _data
#define USART_WaitForRx(_usart) while(!(_usart.STATUS & USART_RXCIF_bm)){}
#define USART_READBYTE(_usart) _usart.DATA

#define USART_Baudrate_Set(_usart, _bselValue, _bScaleFactor)            \
	(_usart).BAUDCTRLA =(uint8_t)_bselValue;                             \
	(_usart).BAUDCTRLB =(_bScaleFactor << USART_BSCALE0_bp)|(_bselValue >> 8)

/************************************************************************/
/* Data Packet settings                                                 */
/************************************************************************/
#define START_HEADER 'S'
#define END_HEADER 'E'
#define CheckReceiverTable '+'

#define RECV_NUM 6

#define NET_ID "8352"
#define ADDNEW_RECV	0x20
#define CHANGE_RECV	0x90
#define ACK_HOST	0xA0
#define ACK_RECV	0x60
#define ReadyData	0xF0
#define OPEN_RECV	0xB0
#define CLOSE_RECV	0xC0

/**
0x20 - Add new channel to Host
0x90 - Change current channel
0xa0 - ACK Host sends out
0x60 - ACK Receiver sends out
0xF0 - Ready for data

**/


/************************************************************************/
/* Function Prototypes                                                  */
/************************************************************************/
void setClockTo32MHz();
void SetupUSART_DEBUG(int BSEL_VALUE, int BSCALE_VALUE);
void SetupUSART_XBEE(int BSEL_VALUE, int BSCALE_VALUE);
void USART_SendString_XBEE(char* str);
void USART_SendString_USB(char* str);
void SendDatapack(unsigned char addr, unsigned char data);
void PrintActiveReceivers();
void SetupDDR();
void BootupLEDs();
int CalculateChannelNum();
int CheckXbeeBaud(uint32_t CHECKBAUD);
int UpdateXbeeBaud();
int XbeeOK();
int CheckXbeeNetID(char *ID_data);
int UpdateXbeeID();
int SystemInitializing();

/************************************************************************/
/* External gloabl variable                                             */
/************************************************************************/
extern uint16_t ActiveReceiver;
extern uint8_t ChannelNum;
extern uint8_t myAddress;

#endif /* XMEGA_H_ */
