/*
 * functions.c
 *
 * Created: 29/01/2016 10:35:32 PM
 *  Author: Steve Wurst
 */
#include	"app_config_s6.h"

void USART_Init( unsigned int ubrr , unsigned char usart_no)
{
	if (usart_no == 0)
	{
		UBRR0H = (unsigned char)(ubrr>>8);
		UBRR0L = (unsigned char)ubrr;
		UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
		UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
		} else {
		UBRR1H = (unsigned char)(ubrr>>8);
		UBRR1L = (unsigned char)ubrr;
		UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE0);
		UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);
	}
}

void USART_Transmit(uint8_t ByteToSend)
{
	while ((UCSR0A & (1 << UDRE0)) == 0) {};		// Do nothing until UDR is ready for more data to be written to it
	UDR0 = ByteToSend;								// Send out the byte value in the variable "ByteToSend"
}

