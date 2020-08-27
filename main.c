#include <avr/io.h>
#include "util/delay.h"
#include <avr/interrupt.h>

unsigned char sec_0 = 9 ;
unsigned char sec_1 = 4;
unsigned char min_0 = 9;
unsigned char min_1 = 5;
unsigned char hr_0 = 3;
unsigned char hr_1 = 2;

void INT0_init (void)
{
	DDRD &= ~ (1 << 2);  // input pin (switch)
	PORTD |= (1 << 2);  // pull up

	SREG |= (1 << 7); // I_bit
	GICR |= (1 << 6); //module interrupt enable
	MCUCR |= (1 << 1);  // falling edge
	MCUCR &= ~ (1 << 0);  // falling edge

}

ISR (INT0_vect)
{
	SREG |= (1 << 7); // I_bit
	sec_0 = 0;
	sec_1 = 0;
	min_0 = 0;
	min_1 = 0;
	hr_0 = 0;
	hr_1 = 0;
}

void INT1_init (void)
{
	DDRD &= ~ (1 << 3);  // input pin (switch)
	//PIND  &= ~ (1 << 3);
	SREG |= (1 << 7); // I_bit

	GICR |= (1 << 7); //module interrupt enable
	MCUCR |= (1 << 2) | (1 << 3);  // raising edge
}

ISR (INT1_vect)
{
	SREG |= (1 << 7); // I_bit
	TCCR1B &= ~(1 << CS10) & ~(1 << CS11) & ~(1 << CS12) ;  // no clk
}

void INT2_init (void)
{
	DDRB &= ~ (1 << 2);  // input pin (switch)
	PORTB |=  (1 << 2);  // pull up
	SREG |= (1 << 7); // I_bit

	GICR |= (1 << 5); //module interrupt enable
	MCUCSR &= ~(1 << 6) ;  // falling edge
}

ISR (INT2_vect)
{
	SREG |= (1 << 7); // I_bit
	TCCR1B |= (1 << CS10) | (1 << CS12);
	TCCR1B &= ~(1 << CS11);
}

void Timer1_init (void)
{
	//TCCR1A = (1 << 3) | (1 << 2); //  FOC1A  FOC1B
	TCCR1A = (1 << FOC1A) | (1 << FOC1B);
	//TCCR1B = (1 << 0) | (1 << 2) | (1 << 3) ;  // N=1024  // mode 4(CTC)
	TCCR1B = (1 << CS10) | (1 << CS12) | (1 << WGM12);

	TCNT1 = 0;  // initial value
	OCR1A = 1000;  // 1 second (1000 count) 976
	TIMSK |= (1 << 4);  // module interrupt enable(A)
	SREG |= (1 << 7); // I_bit
}


ISR (TIMER1_COMPA_vect)
{

	SREG |= (1 << 7); // I_bit
	TCNT1 = 0;  // initial value

	PORTA |= (1 << 6);
	if (sec_0 == 9)
	{
		sec_0 = 0 ;
		PORTC = (PORTC & 0XF0) | (sec_0 & 0X0F) ;
		sec_1 ++;
	}
	else
	{
		sec_0 ++;
		PORTC = (PORTC & 0XF0) | (sec_0 & 0X0F) ;
	}
	//_delay_ms(2);
	PORTA &= ~ (1 << 6);

	PORTA |= (1 << 5);
	if (sec_1 == 6)
	{
		sec_1 = 0 ;
		sec_0 = 0;
		PORTC = (PORTC & 0XF0) | (sec_1 & 0X0F) ;
		min_0 ++;
	}
	else
	{
		PORTC = (PORTC & 0XF0) | (sec_1 & 0X0F) ;
	}
	//_delay_ms(2);
	PORTA &= ~ (1 << 5);


	PORTA |= (1 << 4) ;
	if (min_0 == 10)
	{
		min_0 = 0;
		PORTC = (PORTC & 0XF0) | (min_0 & 0X0F) ;
		min_1 ++;
	}
	else
	{
		PORTC = (PORTC & 0XF0) | (min_0 & 0X0F) ;
	}
	//_delay_ms(2);
	PORTA &= ~ (1 << 4);


	PORTA |= (1 << 3) ;
	if (min_1 == 6) //&& min_0 == 0)
	{
		sec_0 = 0;
		sec_1 = 0;
		min_0 = 0;
		min_1 = 0;
		PORTC = (PORTC & 0XF0) | (min_1 & 0X0F) ;
		hr_0 ++;
	}
	else
	{
		PORTC = (PORTC & 0XF0) | (min_1 & 0X0F) ;
	}
	//_delay_ms(2);
	PORTA &= ~ (1 << 3);


	PORTA |= (1 << 2) ;
	if (hr_0 == 10)
	{
		hr_0 = 0;
		PORTC = (PORTC & 0XF0) | (hr_0 & 0X0F) ;
		hr_1 ++;
	}
	else
	{
		PORTC = (PORTC & 0XF0) | (hr_0 & 0X0F) ;
	}
	//_delay_ms(2);
	PORTA &= ~ (1 << 2);


	PORTA |= (1 << 1) ;

	if (hr_1 == 2 && hr_0 == 4)
	{
		sec_0 = 0;
		sec_1 = 0;
		min_0 = 0;
		min_1 = 0;
		hr_0 = 0;
		hr_1 = 0;
		PORTC = (PORTC & 0XF0) | (hr_1 & 0X0F) ;
	}
	else
	{
		PORTC = (PORTC & 0XF0) | (hr_1 & 0X0F) ;
	}
	//_delay_ms(2);
	PORTA &= ~ (1 << 1);

}

int main(void)
{

	DDRA |= 0X7E;  // PortA (1:6 bits) output to enable the 6 (7 segments)
	PORTA &= 0X81;  // initialize PortA (1:6 bits) with 0 (all 7 segments disabled)

	DDRC |= 0X0F;  // PortC (first 4 pins) output to decoder
	PORTC &= 0XF0; // initialize PortC (first 4 pins) with 0

	//SREG = (1 << 7); // I_bit
	//TCCR1B |= (1 << CS10) | (1 << CS12);
	//TCCR1B &= ~(1 << CS11);
	//GIFR &= ~(1 << 7);

	Timer1_init();
	INT0_init();
	INT1_init();
	INT2_init();

	while (1)
	{

		/*
		PORTA |= (1 << 6);
		PORTC = (PORTC & 0XF0) | (sec_0 & 0X0F) ;
		_delay_ms(2);
		PORTA &= ~ (1 << 6);

		PORTA |= (1 << 5);
		PORTC = (PORTC & 0XF0) | (sec_1 & 0X0F) ;
		_delay_ms(2);
		PORTA &= ~ (1 << 5);

		PORTA |= (1 << 4);
		PORTC = (PORTC & 0XF0) | (min_0 & 0X0F) ;
		_delay_ms(2);
		PORTA &= ~ (1 << 4);

		PORTA |= (1 << 3);
		PORTC = (PORTC & 0XF0) | (min_1 & 0X0F) ;
		_delay_ms(2);
		PORTA &= ~ (1 << 3);

		PORTA |= (1 << 2);
		PORTC = (PORTC & 0XF0) | (hr_0 & 0X0F) ;
		_delay_ms(2);
		PORTA &= ~ (1 << 2);

		PORTA |= (1 << 1);
		PORTC = (PORTC & 0XF0) | (hr_1 & 0X0F) ;
		_delay_ms(2);
		PORTA &= ~ (1 << 1);
		*/
		unsigned char list [6] = {hr_1, hr_0, min_1, min_0, sec_1, sec_0};
		unsigned char i = 1;
		for (i = 6 ; i > 0; i --)
		{
			PORTA |= (1 << i);
			PORTC = (PORTC & 0XF0) | (list[i-1] & 0X0F) ;
			_delay_ms(2);
			PORTA &= ~ (1 << i);
		}
	}


}
