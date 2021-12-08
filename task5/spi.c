#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define DIN 3
#define SCK 0
#define CS 1

#define NUM_DEVICES 4
#define DEL 10000

#define SLAVE_SELECT    PORTD &= ~(1 << CS)
#define SLAVE_DESELECT  PORTD |= (1 << CS)

// Array holding arrays of 5 Bytes for each representation of an ASCII character, stored in flash
char const characters[96][5] PROGMEM = {
	{0b00000000,0b00000000,0b00000000,0b00000000,0b00000000}, // space
	{0b00000000,0b00000000,0b01001111,0b00000000,0b00000000}, // !
	{0b00000000,0b00000111,0b00000000,0b00000111,0b00000000}, // "
	{0b00010100,0b01111111,0b00010100,0b01111111,0b00010100}, // #
	{0b00100100,0b00101010,0b01111111,0b00101010,0b00010010}, // $
	{0b00100011,0b00010011,0b00001000,0b01100100,0b01100010}, // %
	{0b00110110,0b01001001,0b01010101,0b00100010,0b01010000}, // &
	{0b00000000,0b00000101,0b00000011,0b00000000,0b00000000}, // '
	{0b00000000,0b00011100,0b00100010,0b01000001,0b00000000}, // (
	{0b00000000,0b01000001,0b00100010,0b00011100,0b00000000}, // )
	{0b00010100,0b00001000,0b00111110,0b00001000,0b00010100}, // *
	{0b00001000,0b00001000,0b00111110,0b00001000,0b00001000}, // +
	{0b00000000,0b01010000,0b00110000,0b00000000,0b00000000}, // ,
	{0b00001000,0b00001000,0b00001000,0b00001000,0b00001000}, // -
	{0b00000000,0b01100000,0b01100000,0b00000000,0b00000000}, // .
	{0b00100000,0b00010000,0b00001000,0b00000100,0b00000010}, // /
	{0b00111110,0b01010001,0b01001001,0b01000101,0b00111110}, // 0
	{0b00000000,0b01000010,0b01111111,0b01000000,0b00000000}, // 1
	{0b01000010,0b01100001,0b01010001,0b01001001,0b01000110}, // 2
	{0b00100001,0b01000001,0b01000101,0b01001011,0b00110001}, // 3
	{0b00011000,0b00010100,0b00010010,0b01111111,0b00010000}, // 4
	{0b00100111,0b01000101,0b01000101,0b01000101,0b00111001}, // 5
	{0b00111100,0b01001010,0b01001001,0b01001001,0b00110000}, // 6
	{0b00000011,0b01110001,0b00001001,0b00000101,0b00000011}, // 7
	{0b00110110,0b01001001,0b01001001,0b01001001,0b00110110}, // 8
	{0b00000110,0b01001001,0b01001001,0b00101001,0b00011110}, // 9
	{0b00000000,0b01101100,0b01101100,0b00000000,0b00000000}, // :
	{0b00000000,0b01010110,0b00110110,0b00000000,0b00000000}, // ;
	{0b00001000,0b00010100,0b00100010,0b01000001,0b00000000}, // <
	{0b00010100,0b00010100,0b00010100,0b00010100,0b00010100}, // =
	{0b00000000,0b01000001,0b00100010,0b00010100,0b00001000}, // >
	{0b00000010,0b00000001,0b01010001,0b00001001,0b00000110}, // ?
	{0b00110010,0b01001001,0b01111001,0b01000001,0b00111110}, // @
	{0b01111110,0b00010001,0b00010001,0b00010001,0b01111110}, // A
	{0b01111111,0b01001001,0b01001001,0b01001001,0b00111110}, // B
	{0b00111110,0b01000001,0b01000001,0b01000001,0b00100010}, // C
	{0b01111111,0b01000001,0b01000001,0b01000001,0b00111110}, // D
	{0b01111111,0b01001001,0b01001001,0b01001001,0b01001001}, // E
	{0b01111111,0b00001001,0b00001001,0b00001001,0b00000001}, // F
	{0b00111110,0b01000001,0b01001001,0b01001001,0b00111010}, // G
	{0b01111111,0b00001000,0b00001000,0b00001000,0b01111111}, // H
	{0b01000001,0b01000001,0b01111111,0b01000001,0b01000001}, // I
	{0b00110000,0b01000001,0b01000001,0b00111111,0b00000001}, // J
	{0b01111111,0b00001000,0b00010100,0b00100010,0b01000001}, // K
	{0b01111111,0b01000000,0b01000000,0b01000000,0b01000000}, // L
	{0b01111111,0b00000010,0b00001100,0b00000010,0b01111111}, // M
	{0b01111111,0b00000100,0b00001000,0b00010000,0b01111111}, // N
	{0b00111110,0b01000001,0b01000001,0b01000001,0b00111110}, // O
	{0b01111111,0b00001001,0b00001001,0b00001001,0b00000110}, // P
	{0b00111110,0b01000001,0b01010001,0b00100001,0b01011110}, // Q
	{0b01111111,0b00001001,0b00001001,0b00011001,0b01100110}, // R
	{0b01000110,0b01001001,0b01001001,0b01001001,0b00110001}, // S
	{0b00000001,0b00000001,0b01111111,0b00000001,0b00000001}, // T
	{0b00111111,0b01000000,0b01000000,0b01000000,0b00111111}, // U
	{0b00001111,0b00110000,0b01000000,0b00110000,0b00001111}, // V
	{0b00111111,0b01000000,0b00111000,0b01000000,0b00111111}, // W
	{0b01100011,0b00010100,0b00001000,0b00010100,0b01100011}, // X
	{0b00000011,0b00000100,0b01111000,0b00000100,0b00000011}, // Y
	{0b01100001,0b01010001,0b01001001,0b01000101,0b01000011}, // Z
	{0b01111111,0b01000001,0b01000001,0b00000000,0b00000000}, // [
	{0b00000010,0b00000100,0b00001000,0b00010000,0b00100000}, // '\'
	{0b00000000,0b00000000,0b01000001,0b01000001,0b01111111}, // ]
	{0b00000100,0b00000010,0b00000001,0b00000010,0b00000100}, // ^
	{0b01000000,0b01000000,0b01000000,0b01000000,0b01000000}, // _
	{0b00000000,0b00000001,0b00000010,0b00000100,0b00000000}, // `
	{0b00100000,0b01010100,0b01010100,0b01010100,0b01111000}, // a
	{0b01111111,0b01001000,0b01000100,0b01000100,0b00111000}, // 0b
	{0b00111000,0b01000100,0b01000100,0b01000100,0b00100000}, // c
	{0b00111000,0b01000100,0b01000100,0b01001000,0b01111111}, // d
	{0b00111000,0b01010100,0b01010100,0b01010100,0b00011000}, // e
	{0b00001000,0b01111110,0b00001001,0b00000001,0b00000010}, // f
	{0b00001100,0b01010010,0b01010010,0b01010010,0b00111110}, // g
	{0b01111111,0b00001000,0b00000100,0b00000100,0b01111000}, // h
	{0b00000000,0b01000100,0b01111101,0b01000000,0b00000000}, // i
	{0b00100000,0b01000000,0b01000100,0b00111101,0b00000000}, // j
	{0b01111111,0b00010000,0b00101000,0b01000100,0b00000000}, // k
	{0b00000000,0b01000001,0b01111111,0b01000000,0b00000000}, // l
	{0b01111000,0b00000100,0b00001000,0b00000100,0b01111000}, // m
	{0b01111100,0b00001000,0b00000100,0b00000100,0b01111000}, // n
	{0b00111000,0b01000100,0b01000100,0b01000100,0b00111000}, // o
	{0b01111100,0b00010100,0b00010100,0b00010100,0b00001000}, // p
	{0b00001000,0b00010100,0b00010100,0b01111100,0b00000000}, // q
	{0b01111100,0b00001000,0b00000100,0b00000100,0b00001000}, // r
	{0b01001000,0b01010100,0b01010100,0b01010100,0b00100000}, // s
	{0b00000100,0b00111111,0b01000100,0b01000000,0b00100000}, // t
	{0b00111100,0b01000000,0b01000000,0b00100000,0b01111100}, // u
	{0b00011100,0b00100000,0b01000000,0b00100000,0b00011100}, // v
	{0b00111100,0b01000000,0b00110000,0b01000000,0b00111100}, // w
	{0b01000100,0b00101000,0b00010000,0b00101000,0b01000100}, // x
	{0b00001100,0b01010000,0b01010000,0b01010000,0b00111100}, // y
	{0b01000100,0b01100100,0b01010100,0b01001100,0b01000100}, // z
	{0b00000000,0b00001000,0b00110110,0b01000001,0b00000000}, // {
	{0b00000000,0b00000000,0b01111111,0b00000000,0b00000000}, // |
	{0b00000000,0b01000001,0b00110110,0b00001000,0b00000000}, // }
	{0b00001000,0b00000100,0b00000100,0b00001000,0b00000100} // ~
}; // characters[95]

// Message to be displayed, stored in flash
const char message[] PROGMEM = "Hello world!!!";

// Buffer array of bytes to store current display data for each column in each cascaded device
uint8_t buffer [NUM_DEVICES*8];	

void init_spi() {
	DDRD = (1 << DIN) | (1 << SCK) | (1 << CS);		// output - SPI 
	PORTD = (0 << DIN) | (0 << SCK) | (1 << CS);	// unselected S
	SPCR = (1 << SPE) | (1 << MSTR);				// enable SPI, MK - M
}


void write_byte(uint8_t byte) {
	SPDR = byte;                      // SPI starts sending immediately
	while(!(SPSR & (1 << SPIF)));     // Loop until complete bit set
}

void write_word(uint8_t addr, uint8_t data) {
	write_byte(addr);	
	write_byte(data);    
}

// Initializes all cascaded devices
void init_matrix() {
	// Set display brightness
	SLAVE_SELECT;
	for(uint8_t i = 0; i < NUM_DEVICES; i++) {   // Loop through number of cascaded devices
		write_byte(0x0A); // Select Intensity register
		write_byte(0x07); // Set brightness
	}
	SLAVE_DESELECT;

	
	// Set display refresh
	SLAVE_SELECT;
	for(uint8_t i = 0; i < NUM_DEVICES; i++) {
		write_byte(0x0B); // Select Scan-Limit register
		write_byte(0x07); // Select columns 0-7
	}
	SLAVE_DESELECT;

	
	// Turn on the display
	SLAVE_SELECT;
	for(uint8_t i = 0; i < NUM_DEVICES; i++) {
		write_byte(0x0C); // Select Shutdown register
		write_byte(0x01); // Select Normal Operation mode
	}
	SLAVE_DESELECT;

	
	// Disable Display-Test
	SLAVE_SELECT;
	for(uint8_t i = 0; i < NUM_DEVICES; i++) {
		write_byte(0x0F); // Select Display-Test register
		write_byte(0x00); // Disable Display-Test
	}
	SLAVE_DESELECT;
}

// Clears all columns on all devices
void clear_matrix(void)
{
	for(uint8_t x = 1; x < 9; x++) {	// for all columns
		SLAVE_SELECT;
		for(uint8_t i = 0; i < NUM_DEVICES; i++) {
			write_byte(x);    // Select column x
			write_byte(0x00); // Set column to 0
		}
		SLAVE_DESELECT;
	}
}

// Initializes buffer empty
void init_buffer(void) {
	for(uint8_t i = 0; i < NUM_DEVICES*8; i++)
		buffer[i] = 0x00;
}

// Moves each byte forward in the buffer and adds next byte in at the end
void push_buffer(uint8_t x) {
	for(uint8_t i = 0; i < NUM_DEVICES*8 - 1; i++)
		buffer[i] = buffer[i+1];
	buffer[NUM_DEVICES*8 - 1] = x;
}

// Displays current buffer on the cascaded devices
void display_buffer() {
	for(uint8_t i = 0; i < NUM_DEVICES; i++) {// For each cascaded device
		for(uint8_t j = 1; j < 9; j++) {// For each column
			SLAVE_SELECT;
			
			for(uint8_t k = 0; k < i; k++) // Write No-Op code
				write_word(0x00, 0x00);
			
			write_word(j, buffer[j + i*8 - 1]); // Write column data from buffer
			
			for(uint8_t k = NUM_DEVICES-1; k > i; k--) // Write Post No-Op code
				write_word(0x00, 0x00);
			
			SLAVE_DESELECT;
		}
	}
}

// Pushes in 5 characters columns into the buffer.
void push_character(uint8_t c) {
	for(uint8_t j = 0; j < 5; j++) {				// For 5 bytes representing each character
		push_buffer(pgm_read_byte(&characters[c][j]));   // Push the byte from the characters array to the display buffer
		display_buffer();				// Display the current buffer on the devices
		_delay_us(DEL);					// and delay
	}
}

// Takes a pointer to the beginning of a char array holding message, and array size, scrolls message.
void display_message(const char *arrayPointer, uint16_t arraySize) {
	for(uint16_t i = 0; i < arraySize; i++) {
		push_character(pgm_read_byte_near(arrayPointer + i) - 32);	// Send converted ASCII value of character in message to index in characters array (-32 sends current index to characters array)
		push_buffer(0x00);						// Add empty column after character for letter spacing
		display_buffer();						// Display &
		_delay_us(DEL); 						// Delay
	}
	
}

int main(void) {
	init_spi();
	init_matrix();
	clear_matrix();
	init_buffer();
	
	const char *message_pointer = &message[0];
	uint16_t messageSize = sizeof(message);

	while (1) {
		display_message(message_pointer, messageSize);
	}
	return (0);
}
