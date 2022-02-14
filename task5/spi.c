#include <avr/io.h>
#include <util/delay.h>

#define DIN 5
#define SCK 7
#define CS 4
#define SPI_PORT PORTB
#define SPI_DDR DDRB

#define TST_D 3
#define TST_CL 2
#define TST_CS 0

#define NUM_DEVICES 4

#define SLAVE_SELECT    SPI_PORT &= ~(1 << CS)
#define SLAVE_DESELECT  SPI_PORT |= (1 << CS)

// char message[5][8] = {{0x00, 0x09, 0x09, 0x0f, 0x09, 0x09,0x09, 0x00}, {0x00, 0x0f, 0x08, 0x0f, 0x08, 0x08,0x0f, 0x00}, {0x00, 0x08, 0x08, 0x08, 0x08, 0x08,0x0f, 0x00},
// {0x00, 0x08, 0x08, 0x08, 0x08, 0x08,0x0f, 0x00}, {0x00, 0x0f, 0x09, 0x09, 0x09, 0x09,0x0f, 0x00}};

char message[10][4] = {{0x7e, 0x81, 0x81, 0x7e}, {0x04, 0x82, 0xff, 0x80}, {0xe6, 0x91, 0x89, 0x86},
{0x42, 0x89, 0x89, 0x76}, {0x1f, 0x10, 0x10, 0xff}, {0x8f, 0x89, 0x89, 0x71},
{0x7e, 0x89, 0x89, 0x72}, {0xc1, 0x31, 0x0d, 0x03}, {0x76, 0x89, 0x89, 0x76}, {0x4e, 0x91, 0x91, 0x7e}};


#define Intensity 0x0a
#define ScanLimit 0x0b // column quantity
#define ShutDown 0x0c
#define DecodeMode 0x09


void init_spi() {
	SPI_DDR = (1 << DIN) | (1 << SCK) | (1 << CS);	// output - SPI
	SPI_PORT = (0 << DIN) | (0 << SCK) | (1 << CS);	// unselected S
	SPCR = (1 << SPE) | (1 << MSTR) | (0 << CPOL) | (0 << CPHA);			// enable SPI, MK - M
}

void write_byte(uint8_t byte) {
	while(!(SPSR & (1 << SPIF)));     // Loop until complete bit set
	SPDR = byte;                      // SPI starts sending immediately
}

void write_word(uint8_t addr, uint8_t data) {
	write_byte(addr);
	write_byte(data);
}

void write_all_word(uint8_t addr, uint8_t data) {
	SLAVE_SELECT;
	for(uint8_t i = 0; i < NUM_DEVICES; i++)
	write_word(addr, data);
	SLAVE_DESELECT;
}

// Initializes all cascaded devices
void init_matrix() {
	write_all_word(Intensity, 0x07);
	write_all_word(ScanLimit, 0x07);
	write_all_word(ShutDown, 0x01);
	write_all_word(DecodeMode, 0x00);
}

void clear_matrix() {
	for(uint8_t i = 1; i <= 8; i++) {
		write_all_word(i, 0x00);
		_delay_ms(500);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

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
	_delay_ms(3);
}

void TST_clear_matrix() {
	for(uint8_t i = 1; i <= 8; i++) {
		TST_write_all_word(i, 0x00);
		_delay_ms(500);
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

int main() {
	TST_init_spi();
	TST_init_matrix();
	TST_clear_matrix();

// 	TST_write_four_word(1, 0x00, 0x00, 0xff, 0x00);
// 	_delay_ms(1000);
// 	TST_write_four_word(1, 0x05, 0xa0, 0x10, 0xff);
	
	char number = 0; // number - char number in string
	char k = 0; // k - number of column (0..3)
	char array[9]; // array of send char
	
	for (uint8_t i = 0; i <= 8; i++)
		array[i] = 0x00;
	

	while (1) {
		number = 0;
		while (number < 10) {
			for (uint8_t i = 0; i <= 8; i++) // current array
				TST_write_four_word(i, 0x00, 0x00,array[i], 0x00);
			_delay_ms(500);
			for (uint8_t i = 0; i < 8; i++) // logical shift left, empty 7 column
				array[i] = array[i+1];
			array[8] = message[number][k++]; // in current symbol set next column
			
			if (k > 4) { // if every symbol of current symbol was out
				array[8] = 0x00; // empty column between characters
				number++; // next symbol at string
				k = 0; // start with first column
			}
		}
		
	}
	return (0);
}
