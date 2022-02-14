#define F_CPU 8000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD - 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

int adc_value = 0;
int dac_value = 0;
int flag = 0;

int timer_flag = 0;
int timer_counter = 0;

int rxc_counter = 0;
char rxc_data[3] = {0};
	
int adc_flag = 0;

int led_flag = 0;

void send_ch(char c) {
	while (!(UCSRA&(1<<UDRE)));
	UDR = c;
}

void send_str(char *str) {
	while (*str != 0) {
		send_ch(*str++);
	}
	send_ch('\n');
}

void send_adc(int val) {
	char str[4];
	sprintf(str, "%d", val);
	send_str(str);
}

void light() {
	send_str("LIGHT");
	if (led_flag == 0) {
		PORTB |= (1 << 3);
		led_flag = 1;
	}
	else {
		PORTB &= ~(1 << 3);
		led_flag = 0;
	}
}

ISR(USART_RXC_vect) {
	rxc_data[rxc_counter] = UDR;
	rxc_counter++;
	if (rxc_counter == 3) {
		send_str(rxc_data);
		if (('a' == rxc_data[0])&&(('d' == rxc_data[1])&&('c' == rxc_data[2]))) {
			ADMUX = (1<<REFS0)|(0<<MUX0);
			adc_flag = 0;
			flag = 0;
		}
		if (('d' == rxc_data[0])&&(('a' == rxc_data[1])&&('c' == rxc_data[2]))) {
			ADMUX = (1<<REFS0)|(1<<MUX0);
			adc_flag = 0;
			flag = 1;
		}
		if (('l' == rxc_data[0])&&(('e' == rxc_data[1])&&('d' == rxc_data[2]))) {
			light();
		}
		memset(rxc_data, 0, sizeof(rxc_data));
		rxc_counter = 0;
	}
}

ISR(TIMER0_OVF_vect) {
	timer_flag = 1;
}

ISR(ADC_vect) {
	if (adc_flag == 0) {
		if (flag == 0) {
			adc_value = ADCL;
			adc_value |= (ADCH << 8);
			send_str("ADC");
			send_adc(adc_value);
		}
		else {
			dac_value = ADCL;
			dac_value |= (ADCH << 8);
			send_str("DAC");
			send_adc(dac_value);
		}
		adc_flag = 1;
	}
}

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr >>8);
	UBRRL = (unsigned char)ubrr;
	
	UCSRA = 0;
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1<<RXCIE);
}

void init_adc() {
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADIE)|(1<<ADATE);
}

void init_timer() {
	TCCR0 = (0<<CS02)|(1<<CS00);
	TIMSK = (1<<TOIE0);
}

void init_led() {
	DDRB = 0x08;
	PORTB = 0x00;
}

int main() {
	init_uart(MYUBBR);
	init_adc();
	init_timer();
	init_led();
	
	sei();
	
	while (1) {};
}


