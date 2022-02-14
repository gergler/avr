init: 
	ldi r16, low(RAMEND) // init stack
	out SPL, r16
	ldi r16, high(RAMEND)
	out SPH, r16

	ldi r16, 0x08 // ����������� ��������� �� �����
	out DDRB, r16 // ��������� b3
	ldi r16, 0x00
	out PORTB, r16 // ����� ���������

	ldi r16, 0x01 // ����������� ��������� �� �����
	out DDRD, r16 // ��������� b3
	ldi r16, 0x00
	out PORTD, r16 // ����� ���������

	ldi r16, 0x00 // ����������� ������ �� ����
	out DDRA, r16
	ldi r16, 0x01 // ������������� ��������, pull-up
	out PORTA, r16

	ldi r17, 0x01
	ldi r18, 0x00

/*start:
    in r16, PIND // ������ ��������� ������
	ori r16, 0xfe // ���������� ��� �������������� ���� � ���. 1
	cpi r16, 0xff 
	breq noKey
	cpi r17, 0x00
	breq light

noKey:
	cpi r17, 0x01
	brne start
	ldi r16, 0x00 // ����� ���������
	out PORTB, r16
	ldi r17, 0x00 // ���� ��� ������ �� ���������
	rjmp start

light:
	ldi r17, 0x01
	ldi r16, 0x08 // �������� ���������
	out PORTB, r16
	rjmp start*/


start:
	rcall wait
    in r16, PINA // ������ ��������� ������
	out PORTD, r16
	andi r16, 0x01
	cpi r16, 0x01
	brne button
	mov r17, r16
	rjmp start

button:
	andi r17, 0x01
	cpi r17, 0x01
	breq check
	rjmp start

check:
	andi r16, 0x01
	andi r17, 0x01
	eor r16, r17
	cpi r16, 0x01
	brne start

light:
	cpi r18, 0x00
	breq on_light
	cpi r18, 0x08
	breq off_light
	rjmp start

on_light:
	ldi r17, 0x00
	ldi r18, 0x08 
	out PORTB, r18
	rjmp start

off_light:
	ldi r17, 0x00
	ldi r18, 0x00
	out PORTB, r18
	rjmp start


wait:
	ldi r21, 0xFF
	ldi r22, 0x0F
	ldi r23, 0
	
delayloop:
	subi r21, 1 
	sbci r22, 0 
	sbci r23, 0 
	brcc delayloop	
ret

	
