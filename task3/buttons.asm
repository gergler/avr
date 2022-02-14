.def col=r18
.def flag_next=r16

init:
	ldi r16, low(RAMEND) // init stack
	out SPL, r16
	ldi r16, high(RAMEND)
	out SPH, r16

	ldi r16, 0xff // ����������� ����� �� �����
	out DDRD, r16 
	ldi r16, 0x00
	out PORTD, r16 // ����� ����������

	ldi r16, 0x00 //  ������ �� ����
	out DDRA, r16
	ldi r16, 0x03 // ������������� ��������, pull-up �� �������
	out PORTA, r16
	
	ldi r16, 0x03 //  ������� �� �����
	out DDRB, r16
	ldi r16, 0x00 
	out PORTB, r16

	ldi flag_next, 0x01

first_col:
	ldi col, 0x01

button:
	//rcall delay

	mov r22, col
	ldi r17, 0xff
	eor r22, r17
	andi r22, 0x03
	out PORTB, r22 // ������������� �������� �� ������� 
	out DDRB, col // ����������� �� ����� �������

	in r17, PINA // ��������� ��������� ����� �� ��������
	andi r17, 0x03 // �������� �������� �����
	cpi r17, 0x03
	breq flag

	cpi flag_next, 0x00 // ������ ������ ����� ������
	breq button

	cpi r17, 0x03
	brne number

	rjmp button

flag:
	ldi flag_next, 0x01
	rcall clear
	rjmp next_col

number:
    mov r21, col  // ��������� ����� ������, ��� N = 2*column + row
	lsl r21
	add r17, r21
	ldi r21, 0x01

shift:
	lsl r21
	dec r17
	breq light
	rjmp shift

light:
	out PORTD, r21
	ldi flag_next, 0x00 // ������ ������ �������
	rjmp button

next_col:	
	lsl col
	cpi col, 4
	breq first_col
	rjmp button

clear:
	ldi r19, 0x00
	out PORTD, r19
	ret

delay:
	ldi r21, 0xFF
	ldi r22, 0x0F
	ldi r23, 0
	
delayloop:
	subi r21, 1 
	sbci r22, 0 
	sbci r23, 0 
	brcc delayloop	
ret
