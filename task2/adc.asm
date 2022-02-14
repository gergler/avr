initial:
	ldi r16, low(RAMEND)
	out SPL, r16
	ldi r16, high(RAMEND)
	out SPH, r17

	sbi DDRB, PB3

	ldi r16, 1<<ADEN|1<<ADSC|1<<ADATE|3<<ADPS0 // on, start, avto, flag, prescaling 8
	out ADCSRA, r16
	ldi r16, 1<<REFS0|1<<ADLAR|1<<MUX0 // AVCC with C on AREF, left adjust the result, PA1 - ADC1
	out ADMUX, r16

	ldi r16, 1<<WGM00|1<<WGM01|1<<COM01|1<<COM00|1<<WGM01|1<<CS00 // fpwm, no prescaling
	out TCCR0, r16 // config register 

start:
	in r16, ADCH
	out OCR0, r16
	rjmp start
ret
