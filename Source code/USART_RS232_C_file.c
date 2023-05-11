

#include "USART_RS232_H_file.h"						/* Include USART header file */

void USART_Init()				/* USART initialize function */
{ 
	UBRR0H = BAUD_PRESCALE >> 8;
	UBRR0L = BAUD_PRESCALE;
	
	UCSR0C = (0<<UMSEL00) | (0<<UPM00) | (0<<USBS0) | (3<<UCSZ00);
	
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
}

char USART_RxChar()									/* Data receiving function */
{
	uint8_t DataByte;
	while (( UCSR0A & (1<<RXC0)) == 0) {}; // Do nothing until data have been received
	DataByte = UDR0 ;
	return DataByte;								/* Get and return received data */ 
}

void USART_TxChar(char DataByte)						/* Data transmitting function */
{
	while (( UCSR0A & (1<<UDRE0)) == 0) ;
	UDR0 = DataByte;				/* Wait until data transmit and buffer get empty */
}

void USART_SendString(char *c)					/* Send string of USART data function */ 
{
	uint8_t u = 0;
	do
	{
		while (( UCSR0A & (1<<UDRE0)) == 0) {}; // Do nothing until UDR is ready
		UDR0 = c[u];
		u++;
	} while (c[u] != '\0');
}