#define F_CPU 8000000
#define BAUD 9600
#define MYUBBR F_CPU/16/BAUD - 1

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

/*#define BITRATE(TWSR) ((F_CPU/SCL_CLK)-16)/(2*pow(4,(TWSR&((1<<TWPS0)|(1<<TWPS1)))))*/

#define EEPROM_Write_Addess	0xA0
#define EEPROM_Read_Addess	0xA1

#define SCL 0
#define SDA 1

void I2C_Init() {
	PORTC |= (1 << SCL) | (1 << SDA);
	DDRC |= (1 << SCL) | (1 << SDA);
	
	// (F_CPU / F_SCL) - 16) / (2 * pow(4, twps))
	TWBR = 32; // register with speed connection 0x80
	TWSR = 0x00; // prescaler = 1
	TWCR = (1 << TWEN);
}

void UART_Init(unsigned int ubrr) {
	UBRRH = (unsigned char)(ubrr >>8);
	UBRRL = (unsigned char)ubrr;
	
	UCSRA = 0;
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN);
}

void UART_Send_Char(char c) {
	while (!(UCSRA&(1<<UDRE)));
	UDR = c;
	_delay_ms(500);
}

uint8_t I2C_Start(char write_address) {/* I2C start function */
	uint8_t status;		/* Declare variable */
	TWCR = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT); /* Enable TWI, generate START */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	status = TWSR&0xF8;		/* Read TWI status register */
	if(status!=0x08)		/* Check weather START transmitted or not? */
		return '0';			/* Return 0 to indicate start condition fail */
	TWDR = write_address;		/* Write SLA+W in TWI data register */
	TWCR = (1<<TWEN)|(1<<TWINT);	/* Enable TWI & clear interrupt flag */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	status = TWSR&0xF8;		/* Read TWI status register */
	if(status == 0x18)		/* Check for SLA+W transmitted &ack received */
		return '1';			/* Return 1 to indicate ack received */
	if(status == 0x20)		/* Check for SLA+W transmitted &nack received */
		return '2';			/* Return 2 to indicate nack received */
	else
		return '3';			/* Else return 3 to indicate SLA+W failed */
}

uint8_t I2C_Repeated_Start(char read_address) {/* I2C repeated start function */
	uint8_t status;		/* Declare variable */
	TWCR = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);/* Enable TWI, generate start */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	status = TWSR&0xF8;		/* Read TWI status register */
	if(status != 0x10)		/* Check for repeated start transmitted */
		return '0';			/* Return 0 for repeated start condition fail */
	TWDR = read_address;		/* Write SLA+R in TWI data register */
	TWCR = (1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	status = TWSR&0xF8;		/* Read TWI status register */
	if(status == 0x40)		/* Check for SLA+R transmitted &ack received */
		return '1';			/* Return 1 to indicate ack received */
	if(status == 0x48)		/* Check for SLA+R transmitted &nack received */
		return '2';			/* Return 2 to indicate nack received */
	else
		return '3';			/* Else return 3 to indicate SLA+W failed */
}

uint8_t I2C_Write(char data) {	/* I2C write function */
	uint8_t status;		/* Declare variable */
	TWDR = data;			/* Copy data in TWI data register */
	TWCR = (1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	status = TWSR&0xF8;		/* Read TWI status register */
	if(status == 0x28)		/* Check for data transmitted &ack received */
		return '0';			/* Return 0 to indicate ack received */
	if(status == 0x30)		/* Check for data transmitted &nack received */
		return '1';			/* Return 1 to indicate nack received */
	else
		return '2';			/* Else return 2 for data transmission failure */
}

char I2C_Read_Ack()	{	/* I2C read ack function */
	TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWEA); /* Enable TWI, generation of ack */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	return TWDR;			/* Return received data */
}

char I2C_Read_Nack() {		/* I2C read nack function */
	TWCR = (1<<TWEN)|(1<<TWINT);	/* Enable TWI and clear interrupt flag */
	while(!(TWCR&(1<<TWINT)));	/* Wait until TWI finish its current job */
	return TWDR;		/* Return received data */
}

void I2C_Stop() {	/* I2C stop function */
	TWCR = (1<<TWSTO)|(1<<TWINT)|(1<<TWEN);/* Enable TWI, generate stop */
	while(TWCR&(1<<TWSTO));	/* Wait until stop condition execution */
}

int main() {
	char data[5] = "Hi!!!";
	char read_data[5];
	UART_Init(MYUBBR);
	I2C_Init();
	
	UART_Send_Char('S');
	UART_Send_Char(I2C_Start(EEPROM_Write_Addess));
	
	UART_Send_Char('W');
	UART_Send_Char(I2C_Write(0x00)); // Start I2C with device write address
	
	for (int i = 0; i < strlen(data); i++) {
		UART_Send_Char('W');
		UART_Send_Char(I2C_Write(data[i]));
	}
		
	I2C_Stop();
	_delay_ms(10);
	
	UART_Send_Char('S');
	UART_Send_Char(I2C_Start(EEPROM_Write_Addess));
	
	UART_Send_Char('W');
	UART_Send_Char(I2C_Write(0x00)); // Start I2C with device write address
	
	for (int i = 0; i < strlen(data); i++) {
		read_data[i] = I2C_Read_Ack(i);
		_delay_ms(100);
	}
	I2C_Read_Nack();
	I2C_Stop();
	
	while(1) {
		for (int i = 0; i < strlen(data); i++) {
			UART_Send_Char(read_data[i]);
			_delay_ms(500);
		}
	}
	
	return 0;
}

