.equ num0 = 0b00111111
.equ num1 = 0b00000110
.equ num2 = 0b01011011
.equ num3 = 0b01001111
.equ num4 = 0b01100110
.equ num5 = 0b01101101
.equ num6 = 0b01111101
.equ num7 = 0b00000111
.equ num8 = 0b01111111
.equ num9 = 0b01101111

init:
	ldi R16, low(RAMEND)
	out SPL, R16
	ldi R16, high(RAMEND)
	out SPH, R16

	ldi R16, 0xFF // выход все порты семисегментника
	out DDRA, R16
	rjmp start

start:
	ldi R16, num1
	out PORTA, R16
	rcall wait

	ldi R16, num2
	out PORTA, R16
	rcall wait

	ldi R16, num3
	out PORTA, R16
	rcall wait

	ldi R16, num4
	out PORTA, R16
	rcall wait

	ldi R16, num5
	out PORTA, R16
	rcall wait

	ldi R16, num6
	out PORTA, R16
	rcall wait

	ldi R16, num7
	out PORTA, R16
	rcall wait

	ldi R16, num8
	out PORTA, R16
	rcall wait

	ldi R16, num9
	out PORTA, R16
	rcall wait

	ldi R16, num0
	out PORTA, R16
	rcall wait

    rjmp start

wait:
	ldi r17, 0xFF
	ldi r18, 0xFF
	ldi r19, 0x05

delay_loop:
	subi r17, 1 
	sbci r18, 0 
	sbci r19, 0 
	brcc delay_loop
ret
