#include "main.h"
#include "lcd.h"
#include "twi.h"
#include "menu.h"
#include "modbus.h"

void lcd_draw()
{
	if (lcd_contr & 0x80)
	{
		lcd_contr &= 0x3f;
		lcd_contrast();
	}

	if (lcd_stat & LCD_STAT_CLEAR)
	{
		lcd_clear();
		lcd_stat &= ~LCD_STAT_CLEAR;
	}

	if (status & STAT_MENU)
	{
		lcd_menu(scr_menu);
	}
	else
	{
		lcd_out_line_u();
		lcd_out_mode_1();
		lcd_out_mode_2();
		lcd_out_mode_3();
		lcd_out_mode_4();
		lcd_out_speed();

		switch (scr_sel)
		{
		case 0:
			lcd_scr_0();
		break;
		case 1:
			lcd_scr_1(1, (inout_en>>9));
		break;
		case 2:
			lcd_scr_1(2, alarm);
		break;
		default:
			scr_sel = 0;
			lcd_scr_0();
		}
	}
}

void lcd_scr_0()
{
	lcd_out_line_d();
	lcd_out_time();

	str_cpy("Уставка:      ", &lcd_num[0], 14);
	switch (inout)
	{
	case SENS_OUT:
	case SENS_AIR:
	case SENS_WATER:
	case SENS_EX:
	case SENS_ROOM:
	case SENS_LOC:
	default:
		str_cpy("°С", &lcd_num[16], 2);
		lcd_out_num(set_t, &lcd_num[11], 5, 0, LCD_NUM_P);
	break;
	case SENS_OUT_H:
	case SENS_AIR_H:
	case SENS_ROOM_H:
		str_cpy("%", &lcd_num[17], 1);
		lcd_out_num(set_h, &lcd_num[14], 3, 0, 0);
	break;
	}
	lcd_out_string(2,0,18);

	switch (inout)
	{
	case SENS_OUT:
		str_cpy("t° наруж.:      °С", &lcd_num[0], 18);
		lcd_out_num(in[0], &lcd_num[10], 6, 1, LCD_NUM_P);
	break;
	case SENS_AIR:
		str_cpy("t° в канале:    °С", &lcd_num[0], 18);
		lcd_out_num(in[1], &lcd_num[10], 6, 1, LCD_NUM_P);
	break;
	case SENS_WATER:
		str_cpy("t° обр.воды:    °С", &lcd_num[0], 18);
		lcd_out_num(in[2], &lcd_num[10], 6, 1, LCD_NUM_P);
	break;
	case SENS_EX:
		str_cpy("t° вытяжки:     °С", &lcd_num[0], 18);
		lcd_out_num(in[3], &lcd_num[10], 6, 1, LCD_NUM_P);
	break;
	case SENS_ROOM:
		str_cpy("t° помещ.:      °С", &lcd_num[0], 18);
		lcd_out_num(in[4], &lcd_num[10], 6, 1, LCD_NUM_P);
	break;
	case SENS_LOC:
		str_cpy("t° панели:      °С", &lcd_num[0], 18);
		lcd_out_num(in[5], &lcd_num[10], 6, 1, LCD_NUM_P);
	break;
	case SENS_OUT_H:
		str_cpy("h наруж.:        %", &lcd_num[0], 18);
		lcd_out_num(in[6], &lcd_num[14], 3, 0, 0);
	break;
	case SENS_AIR_H:
		str_cpy("h в канале:      %", &lcd_num[0], 18);
		lcd_out_num(in[7], &lcd_num[14], 3, 0, 0);
	break;
	case SENS_ROOM_H:
		str_cpy("h помещ.:        %", &lcd_num[0], 18);
		lcd_out_num(in[8], &lcd_num[14], 3, 0, 0);
	break;
	default:
		str_cpy("                  ", &lcd_num[0], 18);
	}

	lcd_out_string(4,0,18);
}

void lcd_scr_1(unsigned char sel, unsigned int line_en)
{
	if (line_en == 0)
	{
		switch (sel)
		{
		case 1:
			str_cpy("Выходы неактивны  ", &lcd_num[0], 18);
		break;
		case 2:
			str_cpy("Аварий нет        ", &lcd_num[0], 18);
		break;
		}
		lcd_out_string(4,0,18);
		return;
	}

	lcd_j = scr_line_f;
	for (lcd_i = 0; lcd_i < 6; lcd_i++)
	{
		while ((line_en & (1<<lcd_j)) == 0)
		{
			lcd_j++;
			if (lcd_j > 31)
			{
				scr_line[lcd_i] = 255;
				break;
			}
		}
		if (sel != scr_sel)
			return;

		if (lcd_j > 31)
			break;

		scr_line[lcd_i] = lcd_j;
		lcd_j++;
	}

	for (lcd_i = lcd_i; lcd_i < 6; lcd_i++)
		scr_line[lcd_i] = 255;

	for (lcd_k = 0; lcd_k < 6; lcd_k++)
	{
		if (scr_line[lcd_k] == 0xff)
			str_cpy("                     ", &lcd_num[0], 21);
		else
		{
			switch (sel)
			{
			case 1:
				for (lcd_i = 0; lcd_i < 15; lcd_i++)
				{
					lcd_num[lcd_i] = pgm_read_byte(&(disp_out[lcd_i + (scr_line[lcd_k] * 15)]));
				}
				lcd_out_num(out[scr_line[lcd_k]], &lcd_num[15], 5, 0, 0);
				lcd_num[20] = '%';
			break;
			case 2:
				for (lcd_i = 0; lcd_i < 21; lcd_i++)
					lcd_num[lcd_i] = pgm_read_byte(&(disp_alarm[lcd_i + (scr_line[lcd_k] * 21)]));
			break;
			}
		}

		lcd_out_string_s(5-lcd_k,0,21);
	}
}

void lcd_menu(unsigned char sel)
{
	if (status & STAT_SYNC)
	{
		str_cpy("Идёт синхронизация", &lcd_num[0], 18);
		lcd_out_string(3,0,18);
		return;
	}

	//Вывод заголовков меню
	switch (sel)
	{
	case MENU_MAIN:
		str_cpy("  Главное меню    ", &lcd_num[0], 18);
	break;
	case MENU_PARAM:
		str_cpy("    Параметры     ", &lcd_num[0], 18);
	break;
	case MENU_SET:
		str_cpy("    Настройки     ", &lcd_num[0], 18);
	break;
	case MENU_CONF:
		str_cpy("   Конфигурация   ", &lcd_num[0], 18);
	break;
	case MENU_TIMER:
		str_cpy("      Таймер      ", &lcd_num[0], 18);
	break;
	case MENU_MON:
		str_cpy("   Понедельник    ", &lcd_num[0], 18);
	break;
	case MENU_TUE:
		str_cpy("     Вторник      ", &lcd_num[0], 18);
	break;
	case MENU_WED:
		str_cpy("      Среда       ", &lcd_num[0], 18);
	break;
	case MENU_THU:
		str_cpy("     Четверг      ", &lcd_num[0], 18);
	break;
	case MENU_FRI:
		str_cpy("     Пятница      ", &lcd_num[0], 18);
	break;
	case MENU_SAT:
		str_cpy("     Суббота      ", &lcd_num[0], 18);
	break;
	case MENU_SUN:
		str_cpy("   Воскресенье    ", &lcd_num[0], 18);
	break;
	case MENU_TIME:
		str_cpy("      Время       ", &lcd_num[0], 18);
	break;
	}
	lcd_out_string(6,0,18);

	switch (sel)
	{
	case MENU_MON:
	case MENU_TUE:
	case MENU_WED:
	case MENU_THU:
	case MENU_FRI:
	case MENU_SAT:
	case MENU_SUN:
		lcd_menu_timer();
		return;
	break;
	case MENU_TIME:
		lcd_menu_time();
		return;
	break;
	}


	lcd_k = 0;
	while (lcd_k != 2)
	{
		lcd_j = scr_line_f;
		for (lcd_i = 0; lcd_i < 6; lcd_i++)
		{
			while ((menu_line_en[sel] & (1<<lcd_j)) == 0)
			{
				if (sel != scr_menu)
					return;
				if (scr_line_sel == lcd_j)
					scr_line_sel = lcd_j + 1;
				lcd_j++;
				if (lcd_j > 15)
				{
					scr_line[lcd_i] = 255;
					break;
				}
			}
			if (sel != scr_menu)
				return;
			if (lcd_j > 15)
				break;
			scr_line[lcd_i] = lcd_j;
			if (lcd_j == scr_line_sel)
				lcd_k = 2;
			lcd_j++;
		}
		if (lcd_k == 0)
		{
			scr_line_f = 0;
			lcd_k = 1;
		}
		else if (lcd_k == 1)
		{
			scr_line_f++;
		}
	}
	for (lcd_i = lcd_i; lcd_i < 6; lcd_i++)
		scr_line[lcd_i] = 255;

	for (lcd_k = 0; lcd_k < 6; lcd_k++)
	{
		if (scr_line[lcd_k] == 0xff)
			str_cpy("                     ", &lcd_num[0], 21);
		else
		{
			for (lcd_i = 0; lcd_i < 15; lcd_i++)
			{
				switch (sel)
				{
				case MENU_MAIN:
					lcd_num[lcd_i] = pgm_read_byte(&(menu_0[lcd_i + (scr_line[lcd_k] * 15)]));
				break;
				case MENU_PARAM:
					lcd_num[lcd_i] = pgm_read_byte(&(menu_1[lcd_i + (scr_line[lcd_k] * 15)]));
				break;
				case MENU_SET:
					lcd_num[lcd_i] = pgm_read_byte(&(menu_2[lcd_i + (scr_line[lcd_k] * 15)]));
				break;
				case MENU_CONF:
					lcd_num[lcd_i] = pgm_read_byte(&(menu_3[lcd_i + (scr_line[lcd_k] * 15)]));
				break;
				case MENU_TIMER:
					lcd_num[lcd_i] = pgm_read_byte(&(menu_4[lcd_i + (scr_line[lcd_k] * 15)]));
				break;
				default:
					lcd_num[lcd_i] = ' ';
				}
			}

			switch (sel)
			{
			case MENU_PARAM:
			case MENU_SET:
				lcd_out_num(scr_set_tmp[scr_line[lcd_k]], &lcd_num[15], 5, 0, 0);
			break;
			case MENU_CONF:
				lcd_menu_config();
			break;
			default:
				str_cpy("     ", &lcd_num[15], 5);
			}

			if (scr_line_sel == scr_line[lcd_k])
			{
				if (lcd_stat & LCD_STAT_SEL)
				{
					lcd_num[20] = '•';
					lcd_out_num(scr_line_tmp, &lcd_num[15], 5, 0, 0);
				}
				else
					lcd_num[20] = '<';
			}
			else
				lcd_num[20] = ' ';
		}

		lcd_out_string_s(5-lcd_k,0,21);
	}
}

void lcd_menu_config()
{
	switch (scr_line[lcd_k])
	{
	case MEM_MB_BR:
		switch (mem_set_tmp[MEM_MB_BR])
		{
			case MB_BR_4800:
			str_cpy(" 4800", &lcd_num[15], 5);
			break;
			case MB_BR_9600:
			str_cpy(" 9600", &lcd_num[15], 5);
			break;
			case MB_BR_14400:
			str_cpy("14400", &lcd_num[15], 5);
			break;
			case MB_BR_19200:
			str_cpy("19200", &lcd_num[15], 5);
			break;
			case MB_BR_28800:
			str_cpy("28800", &lcd_num[15], 5);
			break;
			case MB_BR_38400:
			str_cpy("38400", &lcd_num[15], 5);
			break;
			default:
				mem_set_tmp[MEM_MB_BR] = 0;
		}
	break;
	case MEM_MB_PR:
		switch (mem_set_tmp[MEM_MB_PR])
		{
			case MB_PR_NONE:
			str_cpy(" none", &lcd_num[15], 5);
			break;
			case MB_PR_RES:
				mem_set_tmp[1] = MB_PR_EVEN;
			case MB_PR_EVEN:
			str_cpy(" even", &lcd_num[15], 5);
			break;
			case MB_PR_ODD:
			str_cpy("  odd", &lcd_num[15], 5);
			break;
			default:
				mem_set_tmp[MEM_MB_PR] = 0;
		}
	break;
	case MEM_MB_SB:
		switch (mem_set_tmp[MEM_MB_SB])
		{
			case MB_SB_1:
			str_cpy("    1", &lcd_num[15], 5);
			break;
			case MB_SB_2:
			str_cpy("    2", &lcd_num[15], 5);
			break;
			default:
				mem_set_tmp[MEM_MB_SB] = 0;
		}
	break;
	case MEM_MB_BIT:
		switch (mem_set_tmp[MEM_MB_BIT])
		{
			case MB_BIT_5:
			str_cpy("    5", &lcd_num[15], 5);
			break;
			case MB_BIT_6:
			str_cpy("    6", &lcd_num[15], 5);
			break;
			case MB_BIT_7:
			str_cpy("    7", &lcd_num[15], 5);
			break;
			case MB_BIT_8:
			str_cpy("    8", &lcd_num[15], 5);
			break;
			default:
				mem_set_tmp[MEM_MB_BIT] = 0;
		}
	break;
	case MEM_MB_ADR:
			if (mem_set_tmp[MEM_MB_ADR] < 1)
				mem_set_tmp[MEM_MB_ADR] = 1;
			else if (mem_set_tmp[MEM_MB_ADR] > 63)
				mem_set_tmp[MEM_MB_ADR] = 63;
			lcd_out_num(mem_set_tmp[MEM_MB_ADR], &lcd_num[15], 5, 0, 0);
	break;
	case MEM_LCD_C:
			if (mem_set_tmp[MEM_LCD_C] > 0x3f)
				mem_set_tmp[MEM_LCD_C] = 0x3f;
			else if (mem_set_tmp[MEM_LCD_C] < 0x07)
				mem_set_tmp[MEM_LCD_C] = 0x07;
			lcd_out_num(mem_set_tmp[MEM_LCD_C], &lcd_num[15], 5, 0, 0);
	break;
	}
}

void lcd_menu_timer()
{
	switch (scr_line_sel / 6)
	{
	case 0:
		str_cpy("Диапазон 1           ", &lcd_num[0], 21);
		lcd_m_tmpc = 0;
	break;
	case 1:
		str_cpy("Диапазон 2           ", &lcd_num[0], 21);
		lcd_m_tmpc = 6;
	break;
	}
	lcd_out_string_s(5,0,21);

	str_cpy("Время работы:        ", &lcd_num[0], 21);
	lcd_out_string_s(4,0,21);

	str_cpy(" 00 : 00  -  00 : 00 ", &lcd_num[0], 21);

	lcd_out_num(scr_set_tmp[lcd_m_tmpc + 0], &lcd_num[1], 2, 0, LCD_NUM_Z);
	lcd_out_num(scr_set_tmp[lcd_m_tmpc + 1], &lcd_num[6], 2, 0, LCD_NUM_Z);
	lcd_out_num(scr_set_tmp[lcd_m_tmpc + 2], &lcd_num[13], 2, 0, LCD_NUM_Z);
	lcd_out_num(scr_set_tmp[lcd_m_tmpc + 3], &lcd_num[18], 2, 0, LCD_NUM_Z);

	switch (scr_line_sel % 6)
	{
	case 0:
		lcd_tmpc = 3;
	break;
	case 1:
		lcd_tmpc = 8;
	break;
	case 2:
		lcd_tmpc = 15;
	break;
	case 3:
		lcd_tmpc = 20;
	break;
	}

	if (lcd_stat & LCD_STAT_SEL)
	{
		lcd_num[lcd_tmpc] = '•';
		lcd_out_num(scr_line_tmp, &lcd_num[lcd_tmpc-2], 2, 0, LCD_NUM_Z);
	}
	else
		lcd_num[lcd_tmpc] = '<';

	lcd_out_string_s(3,0,21);

	str_cpy("Уставка              ", &lcd_num[0], 21);
	lcd_out_num(scr_set_tmp[lcd_m_tmpc + 4], &lcd_num[15], 5, 1, 0);

	if ((scr_line_sel % 6) == 4)
	{
		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_num[20] = '•';
			lcd_out_num(scr_line_tmp, &lcd_num[15], 5, 1, 0);
		}
		else
			lcd_num[20] = '<';
	}

	lcd_out_string_s(2,0,21);

	str_cpy("Скорость             ", &lcd_num[0], 21);
	lcd_out_num(scr_set_tmp[lcd_m_tmpc + 5], &lcd_num[15], 3, 0, 0);

	if ((scr_line_sel % 6) == 5)
	{
		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_num[20] = '•';
			lcd_out_num(scr_line_tmp, &lcd_num[15], 3, 0, 0);
		}
		else
			lcd_num[20] = '<';
	}

	lcd_out_string_s(1,0,21);
}

void lcd_menu_time()
{
	str_cpy("Дата: 00 / 00 / 0000 ", &lcd_num[0], 21);

	lcd_out_num(scr_set_tmp[0], &lcd_num[6], 2, 0, LCD_NUM_Z);
	lcd_out_num(scr_set_tmp[1], &lcd_num[11], 2, 0, LCD_NUM_Z);
	lcd_out_num(scr_set_tmp[2], &lcd_num[16], 4, 0, LCD_NUM_Z);

	if (scr_line_sel <= 1)
	{
		switch (scr_line_sel)
		{
		case 0:
			lcd_tmpc = 8;
		break;
		case 1:
			lcd_tmpc = 13;
		break;
		}
		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_num[lcd_tmpc] = '•';
			lcd_out_num(scr_line_tmp, &lcd_num[lcd_tmpc-2], 2, 0, LCD_NUM_Z);
		}
		else
			lcd_num[lcd_tmpc] = '<';
	}

	if (scr_line_sel == 2)
	{
		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_num[20] = '•';
			lcd_out_num(scr_line_tmp, &lcd_num[16], 4, 0, LCD_NUM_Z);
		}
		else
			lcd_num[20] = '<';
	}

	lcd_out_string_s(4,0,21);

	str_cpy("День недели:         ", &lcd_num[0], 21);

	lcd_tmpuc = scr_set_tmp[3];

	if (scr_line_sel == 3)
	{
		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_num[20] = '•';
			if (scr_line_tmp < 0)
				scr_line_tmp = 0;
			else if (scr_line_tmp > 6)
				scr_line_tmp = 6;

			lcd_tmpuc = scr_line_tmp;
		}
		else
			lcd_num[20] = '<';
	}
	else
		lcd_tmpuc = scr_set_tmp[3];

	switch (lcd_tmpuc)
	{
	case 0:
		lcd_num[18] = 'П';
		lcd_num[19] = 'н';
	break;
	case 1:
		lcd_num[18] = 'В';
		lcd_num[19] = 'т';
	break;
	case 2:
		lcd_num[18] = 'С';
		lcd_num[19] = 'р';
	break;
	case 3:
		lcd_num[18] = 'Ч';
		lcd_num[19] = 'т';
	break;
	case 4:
		lcd_num[18] = 'П';
		lcd_num[19] = 'т';
	break;
	case 5:
		lcd_num[18] = 'С';
		lcd_num[19] = 'б';
	break;
	case 6:
		lcd_num[18] = 'В';
		lcd_num[19] = 'с';
	break;
	}

	lcd_out_string_s(3,0,21);

	str_cpy("Время:       00 : 00 ", &lcd_num[0], 21);
	lcd_out_num(scr_set_tmp[4], &lcd_num[13], 2, 0, LCD_NUM_Z);
	lcd_out_num(scr_set_tmp[5], &lcd_num[18], 2, 0, LCD_NUM_Z);
	if (scr_line_sel > 3)
	{
		switch (scr_line_sel)
		{
		case 4:
			lcd_tmpc = 15;
		break;
		case 5:
			lcd_tmpc = 20;
		break;
		}
		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_num[lcd_tmpc] = '•';
			lcd_out_num(scr_line_tmp, &lcd_num[lcd_tmpc-2], 2, 0, LCD_NUM_Z);
		}
		else
			lcd_num[lcd_tmpc] = '<';
	}
	lcd_out_string_s(2,0,21);
}
