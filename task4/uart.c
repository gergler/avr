#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#define FOSC 8000000
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

char str = 0;
int flag = 0;
int udre_flag = 0;
int timer_flag = 0;
int timer_counter = 0;

int count = 0;
char val[3];

void send_ch(char data) {
	while((UCSRA&(1<<UDRE)) == 0) {};
	UDR = data;
}

void send_str(char *data) {
	unsigned char i = 0;
	while (data[i] != '\0') {
		send_ch(data[i]);
		i++;
	}
	send_ch('\n');
}

// ISR(USART_UDRE_vect) {
// 	if ((udre_flag = 1)&(timer_flag = 1)) {
// 		UDR = str;
// 		udre_flag = 0;
// 		timer_flag = 0;
// 	}
// }
// 
// ISR(TIMER0_OVF_vect) {
// 	timer_counter ++;
// 	if(timer_counter >= 10) {
// 		timer_counter = 0;
// 		timer_flag = 1;
// 	}
// }

// 
// void send_str(const char *data) {
// 	for(int i = 0; i < strlen(data); i++) {
// 		str = data[i];
// 		udre_flag = 1;
// 	}
// }

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
	//ADCSRA &= ~(1<<ADIE);
}

ISR(USART_RXC_vect) {
	val[count] = UDR;
	count++;
	if (count == 3) {
// 		if ('a' == str[0])
// 			adc();
// 		if ('d' == str[0])
// 			dac();
		if (strcmp("led", val))
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
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN)|(1<<TXCIE)|(1<<UDRIE);
	UCSRC = (1<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

void init_adc() {
	ADMUX = (1<<REFS0)|(1<<ADLAR)|(1<<MUX0);
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADATE);
}

void init_timer() {
	TCCR0 = (1<<CS02)|(1<<CS00);
	TIMSK = (1<<TOIE0);
}

void main() {
	init_uart(MYUBRR);
	//init_adc();
	init_timer();
	
	sei();
	
	// LED
	DDRB = 0x08;
	PORTB = 0x00;
	
	while(1){};
}
