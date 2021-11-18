#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define FOSC 8000000
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1
#define DELAY 500

char data[3] = {0};
uint8_t flag = 0;
uint8_t adc_flag = 0;
int count = 0;

void send_ch(char data) {
	while(!(UCSRA&(1<<UDRE))) {}
	UDR = data;
}

void send_str(char *data) {
	unsigned char i = 0;
	while (data[i] != '\0') {
		send_ch(data[i]);
		i++;
	}
	send_ch(13);
}

void light() {
	send_str("LIGHT");
	if (flag == 0) {
		PORTB |= (1 << 3);
		flag = 1;
	}
	else {
		PORTB &= ~(1 << 3);
		flag = 0;
	}
}

void adc() {
	send_str("ADC");
	ADMUX = (1<<REFS0)|(1<<ADLAR)|(1<<MUX0); // AVCC with C on AREF, left adjust the result, PA1 - ADC1
	ADCSRA |= (1<<ADIE);
}

void dac() {
	send_str("DAC");
	ADMUX = (1<<REFS0)|(1<<ADLAR)|(0<MUX0); // AVCC with C on AREF, left adjust the result, PA0 - ADC0
	ADCSRA |= (1<<ADIE);
}

ISR(ADC_vect) {
	uint8_t low_adc = ADCL;
	uint8_t high_adc = ADCH;
	unsigned int adc_value = (high_adc<<2)|(low_adc>>6);
	char* str = "00000";
	sprintf(str, "%d", adc_value);
	send_str(str);
	ADCSRA &= ~(1<<ADIE);
}

ISR(USART_RXC_vect) {
	data[count] = UDR;
	count++;
	if (count == 3) {
		if (strncmp("adc", data, 3) == 0)
			adc();
		else if (strncmp("dac", data, 3) == 0)
			dac();
		else if (strncmp("led", data, 3) == 0)
			light();
		else
			send_str("incorrect command");
		count = 0;
	}
}

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	
	// configuration
	UCSRA = 0;
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN)|(1<<UDRIE)|(1<<TXCIE);
	UCSRC = (1<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

void init_adc() {
	ADMUX = (1<<REFS0)|(1<<ADLAR)|(1<<MUX0);
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADATE);
}

void main() {
	init_uart(MYUBRR);
	init_adc();
	
	sei();
	
	// LED
	DDRB = 0x08;
	PORTB = 0x00;
	
	while(1);
}

