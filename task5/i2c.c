#define F_CPU 8000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD - 1

#include <avr/io.h>
//#include <avr/interrupt.h>
#include <util/delay.h>

#define slave_addr 0xAE

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

void send_ch_UART(char c) {
	while (!(UCSRA&(1<<UDRE)));
	UDR = c;
}

int send_byte_twi(unsigned char data) {
	TWDR = data;  // data register: Load DATA into TWDR Register
	TWCR = (1 << TWINT) | (1 << TWEN);  // control register: Clear TWINT bit in TWCR to start transmission of data
	while(!(TWCR & (1 << TWINT))); // wait for TWINT Flag set, this indicates that the DATA has been transmitted and ACK/NACK has been received
	if((TWSR & 0xF8) == 0x28) // status register: Data byte has been transmitted; ACK has been received
		return 0;
	return -1;
}


int set_connection() // set connection with slave
{
	while((TWSR & 0xF8) != 0x18) {// SLA+W has been transmitted;	ACK has been received
		TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // Send START condition
		while(!(TWCR & (1<<TWINT))); // wait for TWINT Flag set, this indicates that the START condition has been transmitted
		if((TWSR & 0xF8) != 0x08) // Check value of TWI Status Register: A START condition has been transmitted
			return -1;
			
		TWDR = slave_addr; // Load SLA_W into TWDR Register
		TWCR=(1 << TWINT) | (1 << TWEN); // Clear TWINT bit in TWCR to start transmission of address
		while(!(TWCR & (1 << TWINT))); // Wait for TWINT Flag set. This indicates that the SLA+W has been transmitted, and ACK/NACK has been received.
		// SLA+W has been transmitted/Arbitration lost in SLA+W or data bytes/ SLA+R has been transmitted/ SLA+R has been transmitted  and	ACK has been received/NOT ACK has been received
		if (((TWSR & 0xF8) != 0x18) || ((TWSR & 0xF8) != 0x38) || ((TWSR & 0xF8) != 0x40) || ((TWSR & 0xF8) != 0x48))
			return -1;
	}
	return 0;
}

unsigned char write_ch(unsigned char addr, unsigned char data)
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
	
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Transmit STOP condition
	while(TWCR & (1<<TWSTO));
	return 0;
}

unsigned char read_ch(unsigned char addr)
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
	
	data = TWDR;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); // Transmit STOP condition
	while(TWCR & (1<<TWSTO));
	
	return data;
}

void write_str(unsigned char data[], unsigned char start_addr) {
	int i = 0;
	unsigned char output_write;
	while(data[i] != '\0') {
		output_write = write_ch(start_addr + i, data[i]);
		_delay_ms(100);
		i++;
		if (output_write != '0') 
			send_ch_UART('W');
			send_ch_UART(output_write);
	}
}

int main() {
	init_twi();
	init_uart(MYUBBR);
	
	unsigned char data[5] = "HELLO";
	unsigned char start_addr = 0;
	
	write_str(data, start_addr);
	_delay_ms(1000);

	while (1) {
		for (int i = 0; i < 5; i++) {
			send_ch_UART(read_ch(start_addr + i));
			_delay_ms(500);
		}
	}
}

