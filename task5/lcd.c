#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define RS 1
#define RW 2
#define E 3

void strob() {
	PORTB |= (1 << E);
	_delay_ms(200); 
	PORTB &= ~(1 << E);
}

void reset() {
	PORTA = 0b00000010;
	strob();
}

void clear() {
	PORTA = 0b00000001; 
	strob();
	reset();
}

void init_display() {
	PORTA = 0b00111000; // DL = 1 (8 bit), N = 1 (2 row), F = 0 (5x8 dots)
	strob();
	
	PORTA = 0b00000110; // I/D = 1 (increment address counter), S = 0 (display shift OFF) 
	strob();
	
	PORTA = 0b00001111; // D = 1 (display ON), C = 1 (cursor ON), B = 1 (blink ON)
	strob(); 
}

void ascii(char c) {
	PORTB = (1 << RS);
	PORTA = c;
	strob();
	PORTB &= ~(1 << RS);
}

void character() {
	PORTB = (1 << RS);
	PORTA = 0b00000001;
	strob();
	PORTB &= ~(1 << RS);
}

void my_char() {
	PORTB = (0 << RS);
	PORTA = 0b01001000; // CGRAM, ADDR
	strob();
	PORTB = (1 << RS);
	
	PORTA = 0b00001010;	
	strob();
	PORTA = 0b00001010;
	strob();
	PORTA = 0b00000000;
	strob();
	PORTA = 0b00010001;		
	strob();
	PORTA = 0b00011011;	
	strob();
	PORTA = 0b00001010;	
	strob();
	PORTA = 0b00001110;	
	strob();
	PORTA = 0b00000000;	
	strob();
	
	PORTB &= ~(1 << RS);
	PORTA = 0b10000000; 
	strob();
}

void init_ports() {
	// PORT A = data port
	// PORT B = 0b0000|E|RW|RS|0
	DDRB = 0x0f;
	DDRA = 0xff;
	PORTB = 0x00;
	PORTA = 0x00;
}


int main() {
	unsigned char data[15] = {'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', '!', '!', ' '};
	
	init_ports();
	init_display();
	my_char();
	
    while (1) {
		for (int i = 0; i < 15; i++) {
			ascii(data[i]);
		}
		_delay_ms(500);
		
		PORTB = (1 << RS);
		PORTA = 0b00000001;
		strob();
		PORTB &= ~(1 << RS);
		_delay_ms(2000);
		clear();
    }
}

