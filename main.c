#include <avr/io.h>
#include <avr/interrupt.h>
#include "main.h"
#include "lcd.h"
#include "modbus.h"
#include "twi.h"

unsigned char mem_set[6] EEMEM;

int main (void)
{
	status = 0;

	DDRA = 0xff;
	PORTA = 0;
	DDRB = 0b00011111;
	PORTB = 0;
	DDRC = 0b11001100;
	PORTC = 0b00000000;
	DDRD = 0;
	PORTD = 0b11;

	but = 0;

	t_ms = 0;
	t_s = 0;
	t_m = 0;
	t_button = 0;

	lcd_ac = 0;

	init_mem();
	init_sys();
	init_menu();
	init_reg();
	lcd_init();

	mb_init(mem_set_tmp[MEM_MB_ADR], mem_set_tmp[MEM_MB_BR], mem_set_tmp[MEM_MB_PR], mem_set_tmp[MEM_MB_SB], mem_set_tmp[MEM_MB_BIT]);
	twi_init();

	if (lcd_start == 0)
	{
		lcd_clear();
		lcd_out_logo();
		lcd_start = 1;
	}

//------------------------------------------------------------------------
//--------------------Основная инициализация закончена--------------------
//------------------------------------------------------------------------

	OCR0 = 124;
	TCNT0 = 0;
	TIMSK |= (1<<OCIE0);
	TCCR0 = 0b11;

	sei();

	PORTC |= LED_G;

	while (1)
    {
		asm volatile ("nop");
		if (lcd_ac == 1)
		{
			if (lcd_start == 11)
			{
				lcd_ac = 0;
				lcd_draw();
			}
			else if (lcd_start == 10)
			{
				lcd_clear();
				lcd_start = 11;
			}
		}
		asm volatile ("nop");
	}
} 

ISR (TIMER0_COMP_vect)
{
	TCNT0 = 0;
	//1 милисекунда
	t_ms++;
	t_button++;

	//Таймеры ModBus
	timer_mb();
	timer_mb_next();
	timer_mb_check();

	if (status & STAT_MENU)
	{
		if ((status & STAT_MENU_READ) && (status & STAT_MENU_WRITE))
		{
			status &= ~(STAT_SYNC|STAT_MENU_READ|STAT_MENU_WRITE);
			lcd_stat |= LCD_STAT_CLEAR;
		}
	}

	if (twi.state == 0)
		t_twi++;

	if (t_twi > t_twi_set)
	{
		twi.accept = 1;
		twi_queue();
		t_twi = 0;
	}

	for (j = 0; j < 8; j++)
		if (but_timer & (1<<j))
			but_t[j]++;

	if (t_button >= 50)
	{
		lcd_ac = 1;
		t_button = 0;
		but_upd(PIND & (1<<7), 5);
		but_upd(PIND & (1<<6), 4);
		but_upd(PIND & (1<<5), 3);
		but_upd(PIND & (1<<4), 2);
		but_upd(PIND & (1<<3), 1);
		but_upd(PIND & (1<<2), 0);

		if (but & (1<<BUT0_S))
		{
			but0call_s();
			but &= ~(1<<BUT0_S);
		}
		else if (but & (1<<BUT0_L))
		{
			but0call_l();
			but &= ~(1<<BUT0_L);
		}
		if (but & (1<<BUT1_S))
		{
			but1call_s();
			but &= ~(1<<BUT1_S);
		}
		else if (but & (1<<BUT1_L))
		{
			but1call_l();
			but &= ~(1<<BUT1_L);
		}
		if (but & (1<<BUT2_S))
		{
			but2call_s();
			but &= ~(1<<BUT2_S);
		}
		else if (but & (1<<BUT2_L))
		{
			but2call_l();
			but &= ~(1<<BUT2_L);
		}
		if (but & (1<<BUT3_S))
		{
			but3call_s();
			but &= ~(1<<BUT3_S);
		}
		else if (but & (1<<BUT3_L))
		{
			but3call_l();
			but &= ~(1<<BUT3_L);
		}
		if (but & (1<<BUT4_S))
		{
			but4call_s();
			but &= ~(1<<BUT4_S);
		}
		else if (but & (1<<BUT4_L))
		{
			but4call_l();
			but &= ~(1<<BUT4_L);
		}
		if (but & (1<<BUT5_S))
		{
			but5call_s();
			but &= ~(1<<BUT5_S);
		}
		else if (but & (1<<BUT5_L))
		{
			but5call_l();
			but &= ~(1<<BUT5_L);
		}

		if (status & STAT_LED0)
			PORTC |= LED_R;
		else
			PORTC &= ~LED_R;

//		if (status & STAT_LED1)
//			PORTC |= LED_G;
//		else
//			PORTC &= ~LED_G;
	}

	if (t_ms >= 1000)
	{
		t_ms = 0;
		//1 секунда
		t_s++;

		if (lcd_start < 3)
			lcd_start++;
		else if (lcd_start == 3)
		{
			lcd_start = 10;
		}

		if (status & STAT_BLOCK)
			status &= ~STAT_RUN;

		lcd_ac = 1;

		PORTC &= ~LED_G;

		if (t_s > 59)
		{
			t_s = 0;
			//1 минута
			t_m++;

			if (t_m > 59)
				t_m = 0;
		}
	}

	return;
}

void but_upd(unsigned char bit, unsigned char ch)
{
	if (bit == 0)
	{
		if (but_tmp & (1<<ch))
		{
			//Нажата и была нажата
			if (but_t[ch] >= 1000)
			{
				if (but_t[ch] < 1500)
				{
					but |= (1<<((ch*2)+1));
					but_t[ch] = 2250;
				}
				else if (but_t[ch] >= 3150)
				{
					but |= (1<<((ch*2)+1));
					but_t[ch] = 3000;
				}
			}
		}
		else
		{
			but_timer |= (1<<ch);
			//Нажата и не была нажата
		}
		but_tmp |= (1<<ch);
	}
	else
	{
		if (but_tmp & (1<<ch))
		{
			//Не нажата и была нажата
			if (but_t[ch] < 1000)
				but |= (1<<(ch*2));
			but_timer |= (1<<ch);
			but_t[ch] = 0;
		}
		else
		{
			//Не нажата и не была нажата
			if (but & ((1<<((ch*2)+1))|(1<<(ch*2))))
			{
				if (but_t[ch] > 2000)
					but &= ~((1<<((ch*2)+1))|(1<<(ch*2)));
			}
			else
			{
				but_timer &= ~(1<<ch);
				but_t[ch] = 0;
			}
		}
		but_tmp &= ~(1<<ch);
	}
}

//*********************************************
//                   Таймеры
//*********************************************
void timer_mb_set(unsigned int set)
{
	mb.t = 0;
	mb.t_set = set;
}

void timer_mb_stoptx_set(unsigned int set)
{
	mb.t_stoptx = 0;
	mb.t_stoptx_set = set;
}

void timer_mb()
{
	if (mb.t_set > 0)
	{
		mb.t++;
		if (mb.t >= mb.t_set)
		{
			mb.t = 0;
			mb.t_set = 0;
			mb_stop_rx();
		}
	}

	if (mb.t_stoptx_set > 0)
	{
		mb.t_stoptx++;
		if (mb.t_stoptx >= mb.t_stoptx_set)
		{
			mb.t_stoptx = 0;
			mb.t_stoptx_set = 0;
			mb_stop_tx();
		}
	}
}

void timer_mb_check()
{
	tmpc = 0;
	if ((mb.state & MB_RX) == 0)
		tmpc = 1;

	if (((UCSRB & (1<<4)) == 0) || ((UCSRB & (1<<7)) == 0))
		tmpc = 1;

	if (tmpc == 1)
		mb.t_check++;
	else
		mb.t_check = 0;

	if (mb.t_check >= T_MB_CHECK)
	{
		mb.state = 0;
		mb_start_rx();
		mb.t_check = 0;
	}
}

void timer_mb_next()
{
	if (mb.t_next_set > 0)
	{
		if (mb.t_next >= mb.t_next_set)
		{
			mb.t_next = 0;
			mb.t_next_set = 0;

			if ((mb.state & MB_PROC) > 0)
			{
				mb_start_tx();
			}
		}
		else
		{
			mb.t_next++;
		}
	}
}

void timer_mb_next_set(unsigned int set)
{
	mb.t_next = 0;
	mb.t_next_set = set;
}

//*********************************************
