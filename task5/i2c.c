#define F_CPU 8000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD - 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <time.h>
#include <stdlib.h>

int adc_value = 0;
int dac_value = 0;
int flag = 0;

int timer_flag = 0;
int timer_counter = 0;

int rxc_counter = 0;
char rxc_data[7] = {0};

int adc_flag = 0;

int flag_start = 0;

void send_ch(char c) {
	while (!(UCSRA&(1<<UDRE)));
	UDR = c;
}

void send_str(char *str) {
	uint8_t i = 0;
	while (str[i] != '\0') {
		send_ch(str[i]);
		i++;
	}
	send_ch('\n');
}

void send_adc(int val) {
	char str[4];
	sprintf(str, "%d", val);
	send_str(str);
}

ISR(USART_RXC_vect) {
	rxc_data[rxc_counter] = UDR;
	rxc_counter++;
	if (rxc_data[rxc_counter - 1] == ';') {
		send_str(rxc_data);
		if (!strcmp(rxc_data, "speed;")) {
			ADCSRA |= (1 << ADSC);
		}
		if (!strcmp(rxc_data, "game;")) {
			send_str("Start GAME!");
			flag_start = 1;
		}
		if (!strcmp(rxc_data, "stop;")) {
			send_str("Stop GAME!");
			flag_start = 0;
		}
		memset(rxc_data, 0, sizeof(rxc_data));
		rxc_counter = 0;
	}
}

ISR(TIMER0_OVF_vect) {
	timer_flag = 1;
}

ISR(ADC_vect) {
	adc_value = ADCL;
	adc_value |= (ADCH << 8);
	send_str("Your current speed: ");
	send_adc(adc_value);
}

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr >>8);
	UBRRL = (unsigned char)ubrr;
	
	UCSRA = 0;
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1<<RXCIE);
}

void init_adc() {
	ADMUX = (1<<REFS0)|(0<<MUX0);
	ADCSRA = (1<<ADEN)|(1<<ADIE);
}

void init_timer() {
	TCCR0 = (0<<CS02)|(1<<CS00);
	TIMSK = (1<<TOIE0);
}

/////////////////SPI/////////////////////

#define SPI_PORT PORTB
#define SPI_DDR DDRB

#define TST_D 3
#define TST_CL 2
#define TST_CS 0

#define NUM_DEVICES 4

char game[4][8] = {{0x00, 0x38, 0x44, 0x40, 0x4c, 0x44, 0x3c, 0x00},
					{0x00, 0x38, 0x44, 0x44, 0x7c, 0x44, 0x44, 0x00},
					{0x00, 0x44, 0x6c, 0x54, 0x44, 0x44, 0x44, 0x00},
					{0x00, 0x7c, 0x40, 0x7c, 0x40, 0x40, 0x7c, 0x00}};

char message[4][8] = {{0x00, 0x10, 0x10, 0x10, 0x54, 0x38, 0x10, 0x00}, 
					{0x00, 0x10, 0x20, 0x7e, 0x20, 0x10, 0x00, 0x00}, 
				   {0x00, 0x08, 0x04, 0x7e, 0x04, 0x08, 0x00, 0x00}, 
				   {0x00, 0x10, 0x38, 0x54, 0x10, 0x10, 0x10, 0x00}};

#define Intensity 0x0a
#define ScanLimit 0x0b // column quantity
#define ShutDown 0x0c
#define DecodeMode 0x09


void TST_init_spi() {
	SPI_DDR = (1 << TST_CS) | (1 << TST_CL) | (1 << TST_D);
	SPI_PORT = (1 << TST_CS) | (0 << TST_D) | (0 << TST_CL);
}

void TST_write_byte(uint8_t byte){
	for(int i = 0; i < 8; i++){
		if(byte & 0x80){
			SPI_PORT |= (1 << TST_D);
			} else {
			SPI_PORT &= ~(1 << TST_D);
		}
		_delay_us(50);
		SPI_PORT |= (1 << TST_CL);
		_delay_us(50);
		SPI_PORT &= ~(1 << TST_CL);
		byte <<= 1;
	}
}


void TST_write_word(uint8_t addr, uint8_t data) {
	/*	SPI_PORT &= ~(1 << TST_CS);*/
	TST_write_byte(addr);
	TST_write_byte(data);
	// 	SPI_PORT |= (1 << TST_CS);
	// 	_delay_ms(3);
}

void TST_write_all_word(uint8_t addr, uint8_t data) {
	SPI_PORT &= ~(1 << TST_CS);
	for(uint8_t i = 0; i < NUM_DEVICES; i++)
		TST_write_word(addr, data);
	SPI_PORT |= (1 << TST_CS);
	_delay_ms(1);
}

void TST_clear_matrix() {
	for(uint8_t i = 1; i <= 8; i++) {
		TST_write_all_word(i, 0x00);
	}
}

void TST_init_matrix() {
	TST_write_all_word(Intensity, 0x07);
	TST_write_all_word(ScanLimit, 0x07);
	TST_write_all_word(ShutDown, 0x01);
	TST_write_all_word(DecodeMode, 0x00);
}

void TST_write_four_word(uint8_t addr, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4) {
	SPI_PORT &= ~(1 << TST_CS);
	for (int i = 0; i < 9; i++) {
		TST_write_word(addr, data1);
		TST_write_word(addr, data2);
		TST_write_word(addr, data3);
		TST_write_word(addr, data4);
	}
	SPI_PORT |= (1 << TST_CS);
}

///////////////////END SPI/////////////////////

int main() {
	init_uart(MYUBBR);
	init_adc();
	init_timer();
	
	TST_init_spi();
	TST_init_matrix();
	TST_clear_matrix();

	sei();
	
	srand(time(NULL));

	while (1) {
		if (flag_start == 0) {
				for (uint8_t i = 1; i <= 8; i++)
				TST_write_four_word(i, game[0][i-1], game[1][i-1], game[2][i-1], game[3][i-1]);
				while (!flag_start);
				TST_clear_matrix();
		}
		else {
			int r[4] ={rand() % 4, rand() % 4, rand() % 4, rand() % 4};
			for (uint8_t i = 1; i <= 8; i++) {
				TST_write_four_word(i, message[r[0]][i-1], message[r[1]][i-1], message[r[2]][i-1], message[r[3]][i-1]);
			}
			TCNT0 = adc_value*1000;
			while(!timer_flag);
			timer_flag = 0;
			TST_clear_matrix();
		}
	}
}




