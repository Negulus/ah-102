#include "main.h"
#include "lcd.h"
#include "modbus.h"

//Disp/Mode
void but0call_s()
{

	if (PORTC & (1<<6))
		PORTC &= ~(1<<6);
	else
		PORTC |= (1<<6);

	if (!(status & STAT_MENU))
	{
		lcd_stat |= LCD_STAT_CLEAR;
		if (scr_sel > 1)
			scr_sel = 0;
		else
			scr_sel++;
		scr_line_f = 0;
	}
}

//Disp/Mode
void but0call_l()
{
	if (!(status & STAT_MENU))
	{
		mode <<= 1;
		while ((mode & mode_en) == 0)
		{
			mode <<= 1;
			if (mode == 0)
				mode = 1;
		}
	}
}

//Type/Esc
void but1call_s()
{
	if (status & STAT_MENU)
	{
		if (status & STAT_SYNC)
			return;

		if (lcd_stat & LCD_STAT_SEL)
		{
			lcd_stat &= ~LCD_STAT_SEL;

			if (scr_menu == MENU_CONF && scr_line_sel == MEM_LCD_C)
				lcd_contr = mem_set_tmp[MEM_LCD_C] | 0x80;
		}
		else
		{
			lcd_stat |= LCD_STAT_CLEAR;
			switch (scr_menu)
			{
			case MENU_MAIN:
				scr_line_sel = 0;
				status &= ~(STAT_MENU_READ|STAT_MENU_WRITE|STAT_MENU_READY|STAT_MENU);
			break;
			case MENU_PARAM:
				scr_menu = MENU_MAIN;
				scr_line_sel = 0;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_PARAM<<12);
			break;
			case MENU_SET:
				scr_menu = MENU_MAIN;
				scr_line_sel = 1;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_SET<<12);
			break;
			case MENU_CONF:
				scr_menu = MENU_MAIN;
				scr_line_sel = 2;
				
				tmpc = 0;
				if (eeprom_read_byte(&mem_set[MEM_MB_ADR]) != mem_set_tmp[MEM_MB_ADR])
				{
					eeprom_write_byte(&mem_set[MEM_MB_ADR], mem_set_tmp[MEM_MB_ADR]);
					tmpc = 1;
				}
				if (eeprom_read_byte(&mem_set[MEM_MB_BR]) != mem_set_tmp[MEM_MB_BR])
				{
					eeprom_write_byte(&mem_set[MEM_MB_BR], mem_set_tmp[MEM_MB_BR]);
					tmpc = 1;
				}
				if (eeprom_read_byte(&mem_set[MEM_MB_PR]) != mem_set_tmp[MEM_MB_PR])
				{
					eeprom_write_byte(&mem_set[MEM_MB_PR], mem_set_tmp[MEM_MB_PR]);
					tmpc = 1;
				}
				if (eeprom_read_byte(&mem_set[MEM_MB_SB]) != mem_set_tmp[MEM_MB_SB])
				{
					eeprom_write_byte(&mem_set[MEM_MB_SB], mem_set_tmp[MEM_MB_SB]);
					tmpc = 1;
				}
				if (eeprom_read_byte(&mem_set[MEM_MB_BIT]) != mem_set_tmp[MEM_MB_BIT])
				{
					eeprom_write_byte(&mem_set[MEM_MB_BIT], mem_set_tmp[MEM_MB_BIT]);
					tmpc = 1;
				}

				if (eeprom_read_byte(&mem_set[MEM_LCD_C]) != mem_set_tmp[MEM_LCD_C])
				{
					eeprom_write_byte(&mem_set[MEM_LCD_C], mem_set_tmp[MEM_LCD_C]);
				}

				if (tmpc == 1)
				{
					mb_init(mem_set_tmp[MEM_MB_ADR], mem_set_tmp[MEM_MB_BR], mem_set_tmp[MEM_MB_PR], mem_set_tmp[MEM_MB_SB], mem_set_tmp[MEM_MB_BIT]);
				}
			break;
			case MENU_TIMER:
				scr_menu = MENU_MAIN;
				scr_line_sel = 3;
			break;
			case MENU_MON:
				scr_menu = MENU_TIMER;
				scr_line_sel = 0;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_MON<<12);
			break;
			case MENU_TUE:
				scr_menu = MENU_TIMER;
				scr_line_sel = 1;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_TUE<<12);
			break;
			case MENU_WED:
				scr_menu = MENU_TIMER;
				scr_line_sel = 2;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_WED<<12);
			break;
			case MENU_THU:
				scr_menu = MENU_TIMER;
				scr_line_sel = 3;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_THU<<12);
			break;
			case MENU_FRI:
				scr_menu = MENU_TIMER;
				scr_line_sel = 4;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_FRI<<12);
			break;
			case MENU_SAT:
				scr_menu = MENU_TIMER;
				scr_line_sel = 5;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_SAT<<12);
			break;
			case MENU_SUN:
				scr_menu = MENU_TIMER;
				scr_line_sel = 6;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_SUN<<12);
			break;
			case MENU_TIME:
				scr_menu = MENU_MAIN;
				scr_line_sel = 4;
				status &= ~(STAT_MENU_READ|STAT_MENU_READY|(0xff<<12));
				status |= STAT_MENU_WRITE|STAT_SYNC|(MENU_TIME<<12);
			default:
				scr_menu = MENU_MAIN;
			}
		}
	}
	else
	{
		inout <<= 1;
		while (((inout&0x1FF) & (inout_en&0x1FF)) == 0)
		{
			inout <<= 1;
			if ((inout&0x1FF) == 0)
				inout = 1;
		}
	}
}

//Type/Esc
void but1call_l()
{
	if (status & STAT_MENU)
	{
		lcd_contr = mem_set_tmp[MEM_LCD_C] | 0x80;

		lcd_stat |= LCD_STAT_CLEAR;
		status &= ~(STAT_MENU_READ|STAT_MENU_WRITE|STAT_MENU_READY|STAT_MENU|STAT_SYNC);
		scr_menu = MENU_MAIN;
		scr_line_sel = 0;
	}
	scr_sel = 0;
}

//Speed/Ok
void but2call_s()
{
	if (status & STAT_MENU)
	{
		if (status & STAT_SYNC)
			return;

		if (lcd_stat & LCD_STAT_SEL)
		{
			switch (scr_menu)
			{
				case MENU_PARAM:
				case MENU_SET:
				case MENU_MON:
				case MENU_TUE:
				case MENU_WED:
				case MENU_THU:
				case MENU_FRI:
				case MENU_SAT:
				case MENU_SUN:
				case MENU_TIME:
					scr_set_tmp[scr_line_sel] = scr_line_tmp;
				break;
				case MENU_CONF:
					mem_set_tmp[scr_line_sel] = scr_line_tmp;
				break;
			}
			lcd_stat &= ~LCD_STAT_SEL;
		}
		else
		{
			switch (scr_menu)
			{
			case MENU_MAIN:
				switch (scr_line_sel)
				{
				case 0:
					scr_menu = MENU_PARAM;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_PARAM<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 1:
					scr_menu = MENU_SET;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_SET<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 2:
					scr_menu = MENU_CONF;
					lcd_stat |= LCD_STAT_CLEAR;
					mem_set_tmp[MEM_MB_BR] = eeprom_read_byte(&mem_set[MEM_MB_BR]);
					mem_set_tmp[MEM_MB_PR] = eeprom_read_byte(&mem_set[MEM_MB_PR]);
					mem_set_tmp[MEM_MB_SB] = eeprom_read_byte(&mem_set[MEM_MB_SB]);
					mem_set_tmp[MEM_MB_BIT] = eeprom_read_byte(&mem_set[MEM_MB_BIT]);
					mem_set_tmp[MEM_MB_ADR] = eeprom_read_byte(&mem_set[MEM_MB_ADR]);
				break;
				case 3:
					scr_menu = MENU_TIMER;
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 4:
					scr_menu = MENU_TIME;
//					scr_set_tmp[0] = time[0];
//					scr_set_tmp[1] = time[1];
//					scr_set_tmp[2] = time[2];
//					scr_set_tmp[3] = time[4];
//					scr_set_tmp[4] = time[5];
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_TIME<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				}
				scr_line_sel = 0;
				scr_line_f = 0;
			break;
			case MENU_PARAM:
			case MENU_SET:
			case MENU_MON:
			case MENU_TUE:
			case MENU_WED:
			case MENU_THU:
			case MENU_FRI:
			case MENU_SAT:
			case MENU_SUN:
			case MENU_TIME:
				scr_line_tmp = scr_set_tmp[scr_line_sel];
				lcd_stat |= LCD_STAT_SEL;
			break;
			case MENU_CONF:
				if (scr_line_sel == MEM_MB_ADR || scr_line_sel == MEM_LCD_C)
				{
					scr_line_tmp = mem_set_tmp[scr_line_sel];
					lcd_stat |= LCD_STAT_SEL;
				}
				else
				{
					if (mem_set_tmp[scr_line_sel] < 0)
						mem_set_tmp[scr_line_sel] = 0;
					mem_set_tmp[scr_line_sel]++;
					switch (scr_line_sel)
					{
					case MEM_MB_BR:
						tmpc = 5;
					break;
					case MEM_MB_PR:
						tmpc = 3;
					break;
					case MEM_MB_SB:
						tmpc = 1;
					break;
					case MEM_MB_BIT:
						tmpc = 3;
					break;
					}
					if (mem_set_tmp[scr_line_sel] > tmpc)
						mem_set_tmp[scr_line_sel] = 0;
				}
			break;
			case MENU_TIMER:
				switch (scr_line_sel)
				{
				case 0:
					scr_menu = MENU_MON;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_MON<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 1:
					scr_menu = MENU_TUE;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_TUE<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 2:
					scr_menu = MENU_WED;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_WED<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 3:
					scr_menu = MENU_THU;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_THU<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 4:
					scr_menu = MENU_FRI;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_FRI<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 5:
					scr_menu = MENU_SAT;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_SAT<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				case 6:
					scr_menu = MENU_SUN;
					status &= ~(STAT_MENU_WRITE|STAT_MENU_READY|(0xff<<12));
					status |= STAT_MENU_READ|STAT_SYNC|(MENU_SUN<<12);
					lcd_stat |= LCD_STAT_CLEAR;
				break;
				}
			break;
			}
		}
	}
	else
	{
		if (status & STAT_RUN)
		{
			if (speed & SPEED_AUTO)
				speed = 1;
			else if (speed < (speed_en&0x0f))
				speed++;
			else if (speed_en & SPEED_AUTO)
				speed = SPEED_AUTO;
			else
				speed = 1;
		}
	}
}

//Speed/Ok
void but2call_l()
{

if (!(status & STAT_MENU))
{
	switch (scr_sel)
	{
	case 0:
		if (status & STAT_TIMER)
			status &= ~STAT_TIMER;
		else
			status |= STAT_TIMER;
	break;
	case 2:
			status |= STAT_ALARM_R;
	break;
	}
}

}

//On/Setup
void but3call_s()
{
	if (!(status & STAT_MENU))
	{
		if (!(status & STAT_BLOCK))
		{
			if (status & STAT_TIMER)
			{
				if (status & STAT_TIMER_BLOCK)
					status &= ~STAT_TIMER_BLOCK;
				else
					status |= STAT_TIMER_BLOCK;
			}
			else
			{
				if (status & STAT_RUN)
					status &= ~STAT_RUN;
				else
					status |= STAT_RUN;
			}
		}
	}
}

//On/Setup
void but3call_l()
{
	if (!(status & STAT_MENU))
	{
		lcd_stat |= LCD_STAT_CLEAR;
		status |= STAT_MENU;
		scr_menu = MENU_MAIN;
	}
}

//Up
void but4call_s()
{
	if (status & STAT_MENU)
	{
		if (status & STAT_SYNC)
			return;

		if (lcd_stat & LCD_STAT_SEL)
		{
			if (scr_menu == MENU_CONF && scr_line_sel == MEM_LCD_C)
			{
				scr_line_tmp++;
				if (scr_line_tmp > 0x3f)
					scr_line_tmp = 0x3f;
				lcd_contr = scr_line_tmp | 0x80;
			}
			else if (scr_menu == MENU_TIME)
			{
				scr_line_tmp++;
				if (scr_line_tmp > 2099)
					scr_line_tmp = 2099;
			}
			else
			{
				scr_line_tmp++;
				if (scr_line_tmp > 500)
					scr_line_tmp = 500;
			}
		}
		else
		{

			switch (scr_menu)
			{
			case MENU_MON:
			case MENU_TUE:
			case MENU_WED:
			case MENU_THU:
			case MENU_FRI:
			case MENU_SAT:
			case MENU_SUN:
			case MENU_TIME:
				if (scr_line_sel > 0)
					scr_line_sel--;
			break;
			default:
				if (scr_line_sel > 0)
				{
					tmpc = scr_line_sel;
					tmpc--;
					while ((menu_line_en[scr_menu] & (1<<tmpc)) == 0)
					{
						tmpc--;
						if (tmpc < 1)
							break;
					}
					scr_line_sel = tmpc;
				}
			}
		}
	}
	else
	{
		switch (scr_sel)
		{
		case 0:
			switch (inout)
			{
			case SENS_OUT:
			case SENS_AIR:
			case SENS_WATER:
			case SENS_EX:
			case SENS_ROOM:
			case SENS_LOC:
			default:
				if (set_t < 80)
					set_t++;
			break;
			case SENS_OUT_H:
			case SENS_AIR_H:
			case SENS_ROOM_H:
				if (set_h < 80)
					set_h++;
			break;
			}
		break;
		case 1:
			j = 0;
			for (i = scr_line_f; i < 7; i++)
			{
				if (((inout_en>>6) & (1<<i)) != 0)
					j++;
			}
			if ((j - scr_line_f) > 6)
				scr_line_f++;
		break;
		case 2:
			j = 0;
			for (i = scr_line_f; i < 16; i++)
			{
				if ((alarm & (1<<i)) != 0)
					j++;
			}
			if ((j - scr_line_f) > 6)
				scr_line_f++;
		break;
		}
	}
}

void but4call_l()
{
	but4call_s();
}

//Down
void but5call_s()
{
	if (status & STAT_MENU)
	{
		if (status & STAT_SYNC)
			return;

		if (lcd_stat & LCD_STAT_SEL)
		{
			if (scr_menu == MENU_CONF && scr_line_sel == MEM_LCD_C)
			{
				scr_line_tmp--;
				if (scr_line_tmp < 0x07)
					scr_line_tmp = 0x07;
				lcd_contr = scr_line_tmp | 0x80;
			}
			else
			{
				scr_line_tmp--;
				if (scr_line_tmp < -500)
					scr_line_tmp = -500;
			}
		}
		else
		{
			switch (scr_menu)
			{
			case MENU_MON:
			case MENU_TUE:
			case MENU_WED:
			case MENU_THU:
			case MENU_FRI:
			case MENU_SAT:
			case MENU_SUN:
				if (scr_line_sel < 11)
					scr_line_sel++;
			break;
			case MENU_TIME:
				if (scr_line_sel < 5)
					scr_line_sel++;
			break;
			default:
				if (scr_line_sel < 31)
				{
					tmpc = scr_line_sel;
					tmpc++;
					while ((menu_line_en[scr_menu] & (1<<tmpc)) == 0)
					{
						tmpc++;
						if (tmpc > 31)
							break;
					}
					if (tmpc < 32)
						scr_line_sel = tmpc;
				}
			}
		}
	}
	else
	{
		switch (scr_sel)
		{
		case 0:
			switch (inout)
			{
			case SENS_OUT:
			case SENS_AIR:
			case SENS_WATER:
			case SENS_EX:
			case SENS_ROOM:
			case SENS_LOC:
			default:
				if (set_t > -10)
					set_t--;
			break;
			case SENS_OUT_H:
			case SENS_AIR_H:
			case SENS_ROOM_H:
				if (set_h > 0)
					set_h--;
			break;
			}
		break;
		case 1:
		case 2:
			if (scr_line_f > 0)
				scr_line_f--;
		break;
		}
	}
}

void but5call_l()
{
	but5call_s();
}


