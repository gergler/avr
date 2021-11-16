.equ ch1=0x06
.equ ch2=0x5B
.equ ch3=0x4F
.equ ch4=0x66

.dseg // сегмент ОЗУ
Visible: // метка Visible
.byte 4 // в ОЗУ зараезервируем 4 байта

.cseg // Программный сегмент

.org 0 // Адрес вектора Reset
jmp Reset // Переходим на Reset

.org $012 // Адрес вектора таймера 0 по переполнению
jmp TIM0_OVF ; // Переходим на TIM0_OVF

Reset: // Метка Reset
	ldi r16, ch1 // записываем в ОЗУ ch1
	sts Visible, r16
	ldi r16, ch2 // записываем в ОЗУ ch2
	sts Visible+1, r16
	ldi r16, ch3 // записываем в ОЗУ ch3
	sts Visible+2, r16
	ldi r16, ch4 // записываем в ОЗУ ch4
	sts Visible+3, r16

	ldi r16, high(RAMEND) // Инициализируем стек
	out sph, r16
	ldi r16, low(RAMEND)
	out spl, r16

	ldi r16, 0xff 
	out DDRC, r16 // разряд
	out DDRA, r16 // число

	ldi r16, 0b00000010 // Ставим предделитель CS = 010 
	out TCCR0, r16 
	ldi r16, 0b00000001 // TOV0 = 1 бит прерывания по переполнению таймера 0
	out TIFR, r16
	out TIMSK, r16
	ldi r16, 0xfe 
	out TCNT0, r16 // в тикающий регистр засовываем FE (254), чтобы после метки sei попасть в прерывание, тк таймер по переполнению доходит до 0xFF
	ldi r20, 0b0001000 // будем складывать значение порта в регистр r20, ставим начальное положение r20 = PC3
	sei // Разрешаем прерывания

main: // Основная программа
	ldi r21, ch1 // Пихаем единицу в регистр
	sts Visible, r21 // сохраняем в ОЗУ, 1 ячейка
	rcall Delay // Вызываем подпрограмму задержки
	rcall Delay
	ldi r21, ch2 // Дальше по аналогии, только меняем выводимые цифры
	sts Visible, r21
	rcall Delay
	rcall Delay
	ldi r21, ch3
	sts Visible, r21
	rcall Delay
	rcall Delay
	ldi r21, ch4
	sts Visible, r21
	rcall Delay
	rcall Delay
	rjmp main

Delay: // Задержка
	ldi r17, 255
	ldi r18, 255
	ldi r19, 10
Pdelay:
	dec r17
	brne Pdelay
	dec r18
	brne Pdelay
	dec r19
	brne Pdelay
	ret

TIM0_OVF: // Прерывание по переполнению
	cli // Запрещаем прерывания
	lsr r20 // Производим логический сдвиг вправо
	cpi r20, 0b00000000 // Проверяем, не ушли ли за пределы сегментов отображения на индикаторе (PD7)
	breq went_beyond // Если ушли, переходим на обнуление т.е. interrupt
	out PORTC, r20 // Если нет, выводим значение r20 в порт
	ld r16, X+ // Увеличиваем адрес регистра косвенной адресации (Visible+)
	ld r16, X // Загружаем по данному адресу 
	out PORTA, r16 // И выводим в порт A

interrupt: // прерывание
	ldi r16, 0x00 // Взводим тикающий регистр
	out TCNT0, r16
	sei // Разрешаем прерывания
	reti // Выходим из прерывания

went_beyond: 
	ldi r20, 0b00001000 // В r20 значение
	out PORTC, r20 // Выводим на порт
	ldi XH, high(Visible) // Тут произведем зарядку 1 адреса нашего Visible в регистр косвенной
	ldi XL, low(Visible) // адресации, сперва старший, потом младший
	ld r16, X // загрузим значение из X(Visible) в r16
	out PORTA, r16 // И выкинем на порт 
	rjmp interrupt // Перейдем на interrupt
