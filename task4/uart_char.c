#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define FOSC 8000000
#define BAUD 9600 
#define MYUBRR FOSC/16/BAUD-1

uint16_t led_ch = 0;
int counter = 0;
int pcounter = 0;
int flag = 0;

ISR(USART_UDRE_vect) {
	if (flag == 1) {
		UDR = counter;
		flag = 0;
	}
}

ISR(TIMER0_OVF_vect) {
	pcounter ++;
	if(pcounter >= 10){
		pcounter = 0;
		flag = 1;
		if (counter != 8)
			counter++;
		else
			counter = 0;
		PORTA = 1 << counter;
	}
}

ISR(USART_RXC_vect) {
	led_ch = UDR;
	if (led_ch == '1')
		PORTB |= (1 << 3);
	else if (led_ch == '0')
		PORTB &= ~(1 << 3);
}

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	
	// configuration 
	UCSRA = 0;
	UCSRB = (1 << RXCIE)|(1 << RXEN)|(1 << TXEN)|(1 << UDRIE)|(1 << TXCIE); 
	UCSRC = (1 << URSEL)|(0 << USBS)|(1<<UCSZ1)|(1<<UCSZ0); 
}

void init_timer() {
	TCCR0 = (1<<CS02)|(1<<CS00);
	TIMSK = (1<<TOIE0);
}

void main() {
	init_uart(MYUBRR);
	init_timer();
	
	// LED 
	DDRB = 0x08;
	PORTB = 0x00;
	DDRA = 0xff;
	PORTA = 0x00;
	
	sei();
	
    while(1);
}


