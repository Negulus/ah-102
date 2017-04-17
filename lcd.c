#include <avr/io.h>
#include "main.h"
#include "lcd.h"
#include "digit_s.h"
#include "ascii.h"
#include "logo.h"
#include "twi.h"

void lcd_pause()
{
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
}

void lcd_init()
{
	PORTB = LCD_0;
	lcd_contr = mem_set_tmp[MEM_LCD_C];

	lcd_i = 0;
	lcd_j = 0;
	lcd_k = 0;
	lcd_tmpuc = 0;
	lcd_tmpc = 0;
	lcd_tmp = 0;
	lcd_start = 0;
	lcd_stat = 0;

	//Аппаратный рестарт дисплея
	PORTB &= ~LCD_RES;
	lcd_pause();
	PORTB |= LCD_E;
	lcd_pause();
	lcd_pause();
	lcd_pause();
	lcd_pause();
	lcd_pause();
	lcd_pause();
	lcd_pause();
	lcd_pause();
	PORTB &= ~LCD_E;
	lcd_pause();
	PORTB |= LCD_RES;

	//Конфигурироавние дисплея
	lcd_out_i(0xA0);	//ADC select - reverce

	lcd_out_i(0xC0);	//Common output mode select - normal direction
	lcd_out_i(0xA2);	//SET BIAS=0
	lcd_out_i(0x2F);	//POWER control

	lcd_out_i(0x21);	//Ref. R select 00100---

	lcd_contrast();

	lcd_out_i(0x40);	//Display start line set
	lcd_out_i(0xB0);	//Set Page From 0
	lcd_out_i(0x10);	//Column address set upper bit - 0
	lcd_out_i(0x00);	//Column address set lower bit - 0
	lcd_out_i(0xAF);	//Display ON

	lcd_stat |= LCD_STAT_CLEAR;
}

void lcd_contrast()
{
	lcd_out_i(0x81);	//Set Voltage Ref. Mode
	lcd_out_i(0x80 | lcd_contr);	//Set Voltage Ref. Mode
}

void lcd_wait()
{	
	PORTB = LCD_0|LCD_RW;
	lcd_pause();
	PORTA = 0;
	DDRA = 0;
	PORTA = 0xff;
	lcd_pause();
	PORTB |= LCD_E;
	lcd_pause();
	lcd_tmpuc = PINA & 0x80;
	lcd_pause();
	PORTB &= ~LCD_E;
	lcd_pause();
	while(lcd_tmpuc)
	{
//		break;
		lcd_pause();
		PORTB |= LCD_E;
		lcd_pause();
		lcd_tmpuc = PINA & 0x80;
		lcd_pause();
		PORTB &= ~LCD_E;
		lcd_pause();
	}
	PORTB = LCD_0;
	lcd_pause();
	DDRA = 0xff;
	PORTA = 0;
}

void lcd_out_i(unsigned char data)
{
	lcd_wait();
	lcd_pause();
	PORTB = LCD_0;
	lcd_pause();
	PORTA = data;
	lcd_pause();
	PORTB |= LCD_E;
	lcd_pause();
	PORTB &= ~LCD_E;
	lcd_pause();
}

void lcd_out_d(unsigned char data)
{
	lcd_wait();
	lcd_pause();
	PORTB = LCD_0|LCD_A0;
	lcd_pause();
	PORTA = data;
	lcd_pause();
	PORTB |= LCD_E;
	lcd_pause();
	PORTB &= ~LCD_E;
	lcd_pause();
}

void lcd_setpage(unsigned char page)
{
	lcd_out_i(0xB0|page);
}

void lcd_setadr(unsigned char adr)
{
	lcd_out_i((0x10 | ((adr>>4)&0x0f)));
	lcd_out_i((adr&0x0f));
}

void lcd_clear()
{
	for (lcd_j = 0; lcd_j < 8; lcd_j++)
	{
		lcd_setpage(lcd_j);
		lcd_setadr(0);

		for (lcd_i = 0; lcd_i < 128; lcd_i++)
			lcd_out_d(0x00);
	}
}

void lcd_out_string(unsigned char page, unsigned char adr, unsigned char lcd_len)
{
	for (lcd_j = 0; lcd_j < lcd_len; lcd_j++)
	{
		lcd_tmpc = adr + (lcd_j * 7);
		if (lcd_tmpc > 127) return;

		lcd_tmp = ((lcd_num[lcd_j]/* - 32*/) * 14);
		lcd_setpage(page);
		lcd_setadr(lcd_tmpc);

		for (lcd_i = lcd_tmp; lcd_i < lcd_tmp + 7; lcd_i++)
		{
			lcd_out_d(pgm_read_byte(&(ascii[lcd_i])));
			lcd_tmpc++;
			if (lcd_tmpc > 127) return;
		}

		lcd_setpage(page+1);
		lcd_setadr(adr + (lcd_j * 7));

		for (lcd_i = lcd_tmp + 7; lcd_i < lcd_tmp + 14; lcd_i++)
			lcd_out_d(pgm_read_byte(&(ascii[lcd_i])));
	}
}

void lcd_out_string_s(unsigned char page, unsigned char adr, unsigned char lcd_len)
{
	lcd_j = 0;
	lcd_setpage(page);
	lcd_setadr(adr);

	for (lcd_j = 0; lcd_j < lcd_len; lcd_j++)
	{
		if ((adr + (lcd_j * 6)) > 127)
			return;
		for (lcd_i = 0; lcd_i < 5; lcd_i++)
		{
			lcd_tmp = ((lcd_num[lcd_j]/* -32 */) * 5) + lcd_i;
			lcd_out_d(pgm_read_byte(&(ascii_s[lcd_tmp])));
		}
		lcd_out_d(0);
	}
}

void lcd_out_num(signed int lcd_in, char *lcd_out, unsigned char lcd_len, unsigned char lcd_float, unsigned char lcd_type)
{
	//Определение знака
	if (lcd_in < 0)
	{
		lcd_n_tmpc = 1;
		lcd_in *= -1;
	}
	else
		lcd_n_tmpc = 0;

	//Определение длины числа
	if (lcd_in > 9999)
		lcd_n_k = 5;
	else if (lcd_in > 999)
		lcd_n_k = 4;
	else if (lcd_in > 99)
		lcd_n_k = 3;
	else if (lcd_in > 9)
		lcd_n_k = 2;
	else
	{
		if (lcd_float > 0)
			lcd_n_k = 2;
		else
			lcd_n_k = 1;
	}

	lcd_n_z = lcd_n_k - 1;

	if (lcd_float > 0)
		lcd_n_k++;

	if (lcd_n_k > lcd_len)
		lcd_len = lcd_n_k;

	for (lcd_n_i = 0; lcd_n_i < lcd_len; lcd_n_i++)
	{
		if (lcd_n_i < (lcd_len - lcd_n_k - 1))
		{
			if (lcd_type & LCD_NUM_Z)
				*(lcd_out + lcd_n_i) = '0';
			else
				*(lcd_out + lcd_n_i) = ' ';
		}
		else if (lcd_n_i == (lcd_len - lcd_n_k - 1))
		{
			if (lcd_n_tmpc == 1)
				*(lcd_out + lcd_n_i) = '-';
			else if (lcd_type & LCD_NUM_P)
				*(lcd_out + lcd_n_i) = '+';
			else if (lcd_type & LCD_NUM_Z)
				*(lcd_out + lcd_n_i) = '0';
			else
				*(lcd_out + lcd_n_i) = ' ';
		}
		else if (lcd_float > 0 && lcd_n_i == (lcd_len - lcd_float - 1))
			*(lcd_out + lcd_n_i) = '.';
		else
		{
			lcd_n_tmp = 1;
			for (lcd_n_j = 0; lcd_n_j < lcd_n_z; lcd_n_j++)
				lcd_n_tmp *= 10;
			*(lcd_out + lcd_n_i) = (lcd_in/lcd_n_tmp) + 48;
			lcd_in = lcd_in%lcd_n_tmp;
			lcd_n_z--;
		}
	}
}

void str_cpy(char lcd_in[21], char *lcd_out, unsigned char lcd_len)
{
	for (lcd_n_i = 0; lcd_n_i < lcd_len; lcd_n_i++)
		*(lcd_out + lcd_n_i) = lcd_in[lcd_n_i];
}

void lcd_out_speed()
{
	if ((status & STAT_TIMER) && (status & STAT_TIMER_BLOCK))
		str_cpy("   блок", &lcd_num[0], 7);
	else if (status & STAT_RUN)
	{
		str_cpy("       ", &lcd_num[0], 7);
		if (speed == SPEED_AUTO)
			str_cpy("авто", &lcd_num[3], 4);
		else
			lcd_out_num(speed, &lcd_num[4], 3, 0, 0);
	}
	else
		str_cpy("   выкл", &lcd_num[0], 7);

	for (lcd_j = 0; lcd_j < 7; lcd_j++)
	{
		lcd_tmp = ((lcd_num[lcd_j]) * 14);
		lcd_setpage(6);
		lcd_setadr(76 + (lcd_j * 7));

		for (lcd_i = lcd_tmp; lcd_i < lcd_tmp + 7; lcd_i++)
			lcd_out_d(pgm_read_byte(&(ascii[lcd_i])) | 0x01);

		lcd_setpage(7);
		lcd_setadr(76 + (lcd_j * 7));

		for (lcd_i = lcd_tmp + 7; lcd_i < lcd_tmp + 14; lcd_i++)
			lcd_out_d(pgm_read_byte(&(ascii[lcd_i])));
	}
}

void lcd_out_mode_1()
{
	lcd_setpage(6);
	lcd_setadr(0);

	switch (mode)
	{
	case MODE_WINTER:
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(pgm_read_byte(&(winter[lcd_i])));

		lcd_setpage(7);
		lcd_setadr(0);

		for (lcd_i = 16; lcd_i < 32; lcd_i++)
			lcd_out_d(pgm_read_byte(&(winter[lcd_i])));
		break;
	case MODE_SUMMER:
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(pgm_read_byte(&(summer[lcd_i])));

		lcd_setpage(7);
		lcd_setadr(0);

		for (lcd_i = 16; lcd_i < 32; lcd_i++)
			lcd_out_d(pgm_read_byte(&(summer[lcd_i])));
		break;
	case MODE_AUTO:
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(pgm_read_byte(&(avto[lcd_i])));

		lcd_setpage(7);
		lcd_setadr(0);

		for (lcd_i = 16; lcd_i < 32; lcd_i++)
			lcd_out_d(pgm_read_byte(&(avto[lcd_i])));
		break;
	default:
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x01);

		lcd_setpage(7);
		lcd_setadr(0);

		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x00);
	}
}

void lcd_out_mode_2()
{
	lcd_setpage(6);
	lcd_setadr(19);

	for (lcd_i = 0; lcd_i < 16; lcd_i++)
		lcd_out_d(0x01);

	lcd_setpage(7);
	lcd_setadr(19);

	for (lcd_i = 0; lcd_i < 16; lcd_i++)
		lcd_out_d(0x00);
}

void lcd_out_mode_3()
{
	lcd_setpage(6);
	lcd_setadr(38);

	if (status & STAT_BLOCK)
	{
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(pgm_read_byte(&(block[lcd_i])));

		lcd_setpage(7);
		lcd_setadr(38);

		for (lcd_i = 16; lcd_i < 32; lcd_i++)
			lcd_out_d(pgm_read_byte(&(block[lcd_i])));
	}
	else
	{
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x01);

		lcd_setpage(7);
		lcd_setadr(38);

		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x00);
	}
};

void lcd_out_mode_4()
{
	lcd_setpage(6);
	lcd_setadr(57);

	if (alarm > 0)
	{
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(pgm_read_byte(&(error[lcd_i])));

		lcd_setpage(7);
		lcd_setadr(57);

		for (lcd_i = 16; lcd_i < 32; lcd_i++)
			lcd_out_d(pgm_read_byte(&(error[lcd_i])));
	}
	else
	{
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x01);

		lcd_setpage(7);
		lcd_setadr(57);

		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x00);
	}
}

void lcd_out_logo()
{
	lcd_setpage(0);
	lcd_setadr(0);
	for (lcd_j = 0; lcd_j < 6; lcd_j++)
	{
		lcd_setpage(lcd_j+1);
		lcd_setadr(0);
		for (lcd_i = 0; lcd_i < 128; lcd_i++)
		{
			lcd_out_d(pgm_read_byte(&(logo[(lcd_j*128 + lcd_i)])));
		}
	}
}

void lcd_out_time()
{

	switch (time[3])
	{
	case 0:
		lcd_num[0] = 'П';
		lcd_num[1] = 'н';
	break;
	case 1:
		lcd_num[0] = 'В';
		lcd_num[1] = 'т';
	break;
	case 2:
		lcd_num[0] = 'С';
		lcd_num[1] = 'р';
	break;
	case 3:
		lcd_num[0] = 'Ч';
		lcd_num[1] = 'т';
	break;
	case 4:
		lcd_num[0] = 'П';
		lcd_num[1] = 'т';
	break;
	case 5:
		lcd_num[0] = 'С';
		lcd_num[1] = 'б';
	break;
	case 6:
		lcd_num[0] = 'В';
		lcd_num[1] = 'с';
	break;
	}
	lcd_out_string_s(0,2,2);

	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_out_num(time[0], &lcd_num[0], 2, 0, LCD_NUM_Z);
	lcd_num[2] = '/';
	lcd_out_num(time[1], &lcd_num[3], 2, 0, LCD_NUM_Z);
	lcd_num[5] = '/';
	lcd_out_num(time[2], &lcd_num[6], 4, 0, LCD_NUM_Z);
	lcd_out_string_s(0,17,10);

	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_out_num(time[4], &lcd_num[0], 2, 0, LCD_NUM_Z);
	lcd_num[2] = ':';
	lcd_out_num(time[5], &lcd_num[3], 2, 0, LCD_NUM_Z);
	lcd_out_string_s(0,80,5);

	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_setpage(0);
	lcd_setadr(112);
	if (status & STAT_TIMER)
	{
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(pgm_read_byte(&(timer[lcd_i])));

		lcd_setpage(1);
		lcd_setadr(112);

		for (lcd_i = 16; lcd_i < 32; lcd_i++)
			lcd_out_d(pgm_read_byte(&(timer[lcd_i])));
	}
	else
	{
		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x00);

		lcd_setpage(1);
		lcd_setadr(112);

		for (lcd_i = 0; lcd_i < 16; lcd_i++)
			lcd_out_d(0x10);
	}
}

void lcd_out_line_d()
{
	lcd_setpage(1);
	lcd_setadr(0);

	for (lcd_i = 0; lcd_i < 15; lcd_i++)
		lcd_out_d(0x10);

	lcd_out_d(0x1f);

	for (lcd_i = 0; lcd_i < 62; lcd_i++)
		lcd_out_d(0x10);

	lcd_out_d(0x1f);

	for (lcd_i = 0; lcd_i < 32; lcd_i++)
		lcd_out_d(0x10);

	lcd_out_d(0x1f);
}

void lcd_out_line_u()
{
	lcd_setpage(6);

	lcd_setadr(16);
	lcd_out_d(0x01);
	lcd_out_d(0xff);
	lcd_out_d(0x01);

	lcd_setadr(35);
	lcd_out_d(0x01);
	lcd_out_d(0xff);
	lcd_out_d(0x01);

	lcd_setadr(54);
	lcd_out_d(0x01);
	lcd_out_d(0xff);
	lcd_out_d(0x01);

	lcd_setadr(73);
	lcd_out_d(0x01);
	lcd_out_d(0xff);
	lcd_out_d(0x01);

	lcd_setadr(125);
	lcd_out_d(0x01);
	lcd_out_d(0x01);
	lcd_out_d(0x01);

	lcd_setpage(7);

	lcd_setadr(16);
	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_setadr(35);
	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_setadr(54);
	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_setadr(73);
	lcd_out_d(0x00);
	lcd_out_d(0xff);
	lcd_out_d(0x00);

	lcd_setadr(125);
	lcd_out_d(0x00);
	lcd_out_d(0x00);
	lcd_out_d(0x00);
}
