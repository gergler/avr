.def flag = r17

.org 0x001c
	rjmp isr

initial:
	ldi r16, low(RAMEND)
	out SPL, r16
	ldi r16, high(RAMEND)
	out SPH, r17

	sbi DDRB, 3 // led
	cbi PORTB, 3

	ldi r16, 0xff
	out DDRD, r16
	ldi r16, 0x00
	out PORTD, r16

	ldi flag, 0x01

	ldi r16, 1<<ADEN|1<<ADSC|1<<ADATE|1<<ADIE|3<<ADPS0// on, start, avto, flag, prescaling, interupt on
	out ADCSRA, r16
	ldi r16, 1<<REFS0|1<<ADLAR|1<<MUX0 // AVCC with C on AREF, left adjust the result, PA1 - ADC1
	out ADMUX, r16

	ldi r16, 1<<WGM00|1<<WGM01|1<<COM01|1<<COM00|1<<WGM01|1<<CS00 // fpwm, no prescaling
	out TCCR0, r16 // config register 

	sei // sreg i = 1, isr on

main_loop:
	nop
	rjmp main_loop 

isr:
	cli
	in r16, ADCH
	cpi flag, 0x00
	breq potentiometer 

button:
	out OCR0, r16
	ldi r16, 1<<REFS0|1<<ADLAR|1<<MUX0 // AVCC with C on AREF, left adjust the result, PA1 - ADC1
	out ADMUX, r16
	ldi flag, 0x00
	reti

potentiometer:
	out PORTD, r16
	ldi r16, 1<<REFS0|1<<ADLAR|0<<MUX0 // AVCC with C on AREF, left adjust the result, PA0 - ADC0
	out ADMUX, r16
	ldi flag, 0x01
	reti
