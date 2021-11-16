.equ Delay = 10

init:
	ldi r16, low(RAMEND) // init stack
	out SPL, r16
	ldi r16, high(RAMEND)
	out SPH, r17

	ldi r16, 0<<FOC0|1<<WGM00|1<<COM01|1<<COM00|1<<WGM01|1<<CS00 // fpwm, compare match, no prescaling
	out TCCR0, r16

	ldi r16, 255
	out OCR0, r16 // compare register

	sbi DDRB, 3
	rjmp start

start:
	rcall decrement
	rcall increment
	ldi r16, 255
	rjmp start

decrement:
	out OCR0, r16 // for fpwm
	rcall wait
	dec r16
	brne decrement // branch if not equal (zero)
	ret // return to the addr loc on the stack

increment:
	out OCR0, r16 
	rcall wait 
	inc r16 
	brne increment 
	ret 

wait:
	ldi r17, 0xFF
	ldi r18, 0x0F

delay_loop:
	subi r17, 1 
	sbci r18, 0 
	brne delay_loop	
ret
