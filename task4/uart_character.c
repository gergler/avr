#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define FOSC 8000000
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

uint8_t receive = 0;
uint8_t rx_data = 0;
volatile uint8_t rx_flag = 0;

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	
	// configuration
	UCSRA = 0;
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN)|(1<<UDRIE)|(1<<TXCIE);
	UCSRC = (1<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

void UARTSend(uint8_t data) {
	while(!(UCSRA & (1<<UDRE)));
	UDR = data;
}

unsigned char UARTGet() {
	while(!rx_flag);
	rx_flag = 0;
	return rx_data;
}

int main(void) {
	sei();
	init_uart(MYUBRR);
	while(1) {
		receive = UARTGet();
		UARTSend(receive);
	}
}

ISR(USART_RXC_vect) {
	rx_data = UDR;
	rx_flag = 1;
}
