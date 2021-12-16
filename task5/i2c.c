#define F_CPU 8000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD - 1

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define EEPROM_Write_Addess	0xA0
#define EEPROM_Read_Addess	0xA1

#define SCL 0
#define SDA 1

void init_twi() {
	PORTC |= (1 << SCL) | (1 << SDA);
	DDRC &= ~(1 << SCL) | (1 << SDA);
	
	// (F_CPU / F_SCL) - 16) / (2 * pow(4, twps))
	TWBR = 0x64; // register with speed connection 0x80
	TWSR = 0x00; // prescaler = 1
	TWCR = (1 << TWEN);
}

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr >>8);
	UBRRL = (unsigned char)ubrr;
	
	UCSRA = 0;
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN);
}

void send_ch_UART(char c) {
	while (!(UCSRA&(1<<UDRE)));
	UDR = c;
}

void start_twi() {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // Send START condition
	while(!(TWCR & (1<<TWINT))); // wait for TWINT Flag set, this indicates that the START condition has been transmitted
}

void stop_twi() {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Transmit STOP condition
	while(TWCR&(1<<TWSTO));	// Wait until stop condition execution
}

void write_twi(unsigned char data) {
	TWDR = data;  // data register: Load DATA into TWDR Register
	TWCR = (1 << TWINT) | (1 << TWEN);  // control register: Clear TWINT bit in TWCR to start transmission of data
	while(!(TWCR & (1 << TWINT))); // wait for TWINT Flag set, this indicates that the DATA has been transmitted and ACK/NACK has been received
	if((TWSR & 0xF8) == 0x28) // ACK has been received
		return 0;
// 	if((TWSR & 0xF8) == 0x30) // NACK has been received
// 		return 0;
	else 
		return -1;
}

unsigned char read_twi() {
	TWCR = (1 << TWINT) | (1 << TWEN);  // control register: Clear TWINT bit in TWCR to start transmission of data
	while(!(TWCR & (1 << TWINT))); // wait for TWINT Flag set, this indicates that the DATA has been transmitted and ACK/NACK has been received
	return TWDR;
}

int send_byte_twi(unsigned char data) {
	TWDR = data;  // data register: Load DATA into TWDR Register
	TWCR = (1 << TWINT) | (1 << TWEN);  // control register: Clear TWINT bit in TWCR to start transmission of data
	while(!(TWCR & (1 << TWINT))); // wait for TWINT Flag set, this indicates that the DATA has been transmitted and ACK/NACK has been received
	if((TWSR & 0xF8) == 0x28) // status register: Data byte has been transmitted; ACK has been received
		return 0;
	return -1;
}

int start() { // set connection with slave
	start_twi();
	if((TWSR & 0xF8) != 0x08) // Check value of TWI Status Register: A START condition has been transmitted
		return -1;
	write_twi(EEPROM_Write_Addess);
		
	if ((TWSR & 0xF8) == 0x18) // ack received
		return 0;
// 	if ((TWSR & 0xF8) == 0x20) // nack received
// 		return 0;
	else 
		return -1;
	return 0;
}

int repeated_start() { 
	start_twi();
	if((TWSR & 0xF8) != 0x10) //  Check for repeated start transmitted
		return -1;
	write_twi(EEPROM_Write_Addess);
		
	if ((TWSR & 0xF8) == 0x40) // ack received
		return 0;
// 	if ((TWSR & 0xF8) == 0x48) // nack received
// 		return 0;
	else 
		return -1;
}

unsigned char write_word_twi(unsigned char addr, unsigned char data)
{
	int ch = set_connection();
	if (ch == -1)
		return '1';
	
	ch = send_byte_twi(addr << 8); // send ADDR
	if(ch == -1)
		return '2';
	
	ch = send_byte_twi(addr);
	if(ch == -1)
		return '3';
	
	ch = send_byte_twi(data); // send DATA
	if(ch == -1)
		return '4';
	
	stop_twi();
	return 0;
}

unsigned char read_word_twi(unsigned char addr)
{
	unsigned char data;
	
	int ch = set_connection();
	if (ch == -1)
		return '1';

	ch = send_byte_twi(addr >> 8); // send ADDR
	if(ch == -1)
		return '2';
	
	ch = send_byte_twi(addr);
	if(ch == -1)
		return '3';

	start_twi();
	if((TWSR & 0xF8) != 0x10) //  A repeated START condition has been transmitted
		return '4';
	
	write_twi(slave_addr + 1);
	if((TWSR & 0xF8) != 0x40) // SLA+R has been transmitted; ACK has been received
		return '5';

	data =read_twi();
	if((TWSR & 0xF8) != 0x58) // Data byte has been received; NOT ACK has been returned
		return '6';

	stop_twi();
	return data;
}

void write_str(unsigned char data[], unsigned char start_addr) {
	int i = 0;
	unsigned char output_write;
	while(data[i] != '\0') {
		output_write = write_word_twi(start_addr + i, data[i]);
		_delay_ms(100);
		i++;
		if (output_write != '0') {
			send_ch_UART('W');
			send_ch_UART(output_write);
		}
	}
}

int main() {
	unsigned char data[5] = "HELLO";
	init_twi();
	init_uart(MYUBBR);
	start();
	write_twi(0x00); // Write start memory address for data write
	unsigned char start_addr = 0;
	
	for (int i = 0; i < strlen(data); i++)
		write_twi(data[i]);
		
	stop_twi();
	_delay_ms(10)
	
	start();
	write_twi(0x00); 
	repeated_start(EEPROM_Read_Addess); // Repeat start I2C SLA+R 

	while (1) {
		for (int i = 0; i < 5; i++) {
			send_ch_UART(read_twi(start_addr + i));
			_delay_ms(500);
		}
	read_NACK();
	stop_twi();
}

