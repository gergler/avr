#define F_CPU 8000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD - 1

#include <avr/io.h>
#include <util/delay.h>

#define slave_addr 0xA0

#define SCL 0
#define SDA 1

void init_twi() {
	PORTC |= (1 << SCL) | (1 << SDA);
	DDRC &= ~(1 << SCL) | (1 << SDA);

	// (F_CPU / F_SCL) - 16) / (2 * pow(4, twps))
	TWBR = 0x80; // register with speed connection
	TWSR = 0x00; // prescaler = 1
}

void init_uart(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr >>8);
	UBRRL = (unsigned char)ubrr;

	UCSRA = 0;
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN);
}

void send_ch_uart(char c) {
	while (!(UCSRA&(1<<UDRE)));
	UDR = c;
}

int write_byte(unsigned char data) {
	TWDR = data;  // data register: Load DATA into TWDR Register
	TWCR = (1 << TWINT) | (1 << TWEN);  // control register: Clear TWINT bit in TWCR to start transmission of data
	while(!(TWCR & (1 << TWINT))); // wait for TWINT Flag set, this indicates that the DATA has been transmitted and ACK/NACK has been received
	if((TWSR & 0xF8) == 0x28) // status register: Data byte has been transmitted; ACK has been received
		return 0;
	return -1;
}


int set_connection() {// set connection with slave
	while((TWSR & 0xF8) != 0x18) {// SLA+W has been transmitted; ACK has been received
		TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // Send START condition
		while(!(TWCR & (1<<TWINT))); // wait for TWINT Flag set, this indicates that the START condition has been transmitted
		if((TWSR & 0xF8) != 0x08) // Check value of TWI Status Register: A START condition has been transmitted
			return -1;

		TWDR = slave_addr; // Load SLA_W into TWDR Register
		TWCR = (1 << TWINT) | (1 << TWEN); // Clear TWINT bit in TWCR to start transmission of address
		while(!(TWCR & (1 << TWINT))); // Wait for TWINT Flag set. This indicates that the SLA+W has been transmitted, and ACK/NACK has been received.
	}
	return 0;
}

unsigned char write_ch(unsigned char addr, unsigned char data) {
	int status = set_connection();
	if (status == -1)
		return '1';

	status = write_byte(addr << 8); // send ADDR
	if(status == -1)
		return '2';

	status = write_byte(addr);
	if(status == -1)
		return '3';

	status = write_byte(data); // send DATA
	if(status == -1)
		return '4';

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Transmit STOP condition
	while(TWCR & (1<<TWSTO));
	return '0';
}

unsigned char read_ch(unsigned char addr) {
	unsigned char data;

	int status = set_connection();
	if (status == -1)
		return '1';

	status = write_byte(addr >> 8); // send ADDR
	if(status == -1)
		return '2';

	status = write_byte(addr);
	if(status == -1)
		return '3';

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // Send START condition
	while(!(TWCR & (1<<TWINT))); // wait for TWINT Flag set, this indicates that the START condition has been transmitted
	if((TWSR & 0xF8) != 0x10) //  A repeated START condition has been transmitted
		return '4';

	TWDR = slave_addr + 1; // Load SLA_W into TWDR Register
	TWCR = (1 << TWINT) | (1 << TWEN); //  Clear TWINT bit in TWCR to start transmission of address
	while(!(TWCR & (1 << TWINT))); // Wait for TWINT Flag set. This indicates that the SLA+W has been transmitted, and ACK/NACK has been received.
	if((TWSR & 0xF8) != 0x40) // SLA+R has been transmitted; ACK has been received
		return '5';

	TWCR = (1<<TWINT) | (1<<TWEN); // READ
	while(!(TWCR & (1<<TWINT)));
	if((TWSR & 0xF8) != 0x58) // Data byte has been received; NOT ACK has been returned
		return '6';

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Transmit STOP condition
	while(TWCR & (1<<TWSTO));

	return TWDR;
}

void write_str(unsigned char data[], unsigned char start_addr) {
	int i = 0;
	while(data[i] != '\0') {
		send_ch_uart(write_ch(start_addr + i, data[i]));
		_delay_ms(500);
		i++;
	}
}

int main() {
	init_twi();
	init_uart(MYUBBR);

	unsigned char data[15] = "HELLO WORLD!!! ";
	unsigned char start_addr = 0;
	char read_data[15];

// 	write_str(data, start_addr);
// 	_delay_ms(1000);

	for (int i = 0; i < 15; i++) {
		read_data[i] = read_ch(start_addr + i);
		_delay_ms(100);
	}

	while (1) {
		for (int i = 0; i < 15; i++) {
			send_ch_uart(read_data[i]);
			_delay_ms(500);
		}
	}
}
