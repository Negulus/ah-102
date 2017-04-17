#include "main.h"
#include "modbus.h"
#include "lcd.h"

void init_reg()
{
	reg_a[0] = &menu_line_en[0];
	reg_a[1] = &menu_line_en[1];
	reg_a[2] = &menu_line_en[2];
	reg_a[3] = &mode_en;
	reg_a[4] = &speed_en;
	reg_a[5] = &inout_en;

	reg_b[0] = &status;
	reg_b[1] = &mode;
	reg_b[2] = &speed;
	reg_b[3] = &inout;
	reg_b[4] = &alarm;
	reg_b[5] = &time[0];
	reg_b[6] = &time[1];
	reg_b[7] = &time[2];
	reg_b[8] = &time[3];
	reg_b[9] = &time[4];
	reg_b[10] = &time[5];
	reg_b[11] = &set_t;
	reg_b[12] = &set_h;
	reg_b[13] = &in[5];

	reg_c[0] = &in[0];
	reg_c[1] = &in[1];
	reg_c[2] = &in[2];
	reg_c[3] = &in[3];
	reg_c[4] = &in[4];
	reg_c[5] = &in[6];
	reg_c[6] = &in[7];
	reg_c[7] = &in[8];
	reg_c[8] = &out[0];
	reg_c[9] = &out[1];
	reg_c[10] = &out[2];
	reg_c[11] = &out[3];
	reg_c[12] = &out[4];
	reg_c[13] = &out[5];
	reg_c[14] = &out[6];

	reg_d[0] = &scr_set_tmp[0];
	reg_d[1] = &scr_set_tmp[1];
	reg_d[2] = &scr_set_tmp[2];
	reg_d[3] = &scr_set_tmp[3];
	reg_d[4] = &scr_set_tmp[4];
	reg_d[5] = &scr_set_tmp[5];
	reg_d[6] = &scr_set_tmp[6];
	reg_d[7] = &scr_set_tmp[7];
	reg_d[8] = &scr_set_tmp[8];
	reg_d[9] = &scr_set_tmp[9];
	reg_d[10] = &scr_set_tmp[10];
	reg_d[11] = &scr_set_tmp[11];
	reg_d[12] = &scr_set_tmp[12];
	reg_d[13] = &scr_set_tmp[13];
	reg_d[14] = &scr_set_tmp[14];
	reg_d[15] = &scr_set_tmp[15];
}

void init_menu()
{
	menu_line_en[MENU_MAIN] = 0b00011100;
	menu_line_en[MENU_PARAM] = 0b11111101;
	menu_line_en[MENU_SET] = 0b11111111;
	menu_line_en[MENU_CONF] = 0b111111;
	menu_line_en[MENU_TIMER] = 0b1111111;

	for (i = 0; i < 16; i++)
		scr_set_tmp[i] = 0;
}

void init_sys()
{
	status = 0;
	mode_en = MODE_WINTER|MODE_SUMMER|MODE_AUTO;
	mode = 0b001;
	speed_en = SPEED_AUTO|4;
	speed = SPEED_AUTO;
	alarm = 0;
	inout_en = SENS_LOC|SENS_ROOM_H;
	inout = SENS_LOC;
	set_t = 20;
	set_h = 50;
	in[0] = 0;
	in[1] = 0;
	in[2] = 0;
	in[3] = 0;
	in[4] = 0;
	in[5] = 0;
	in[6] = 0;
	in[7] = 0;
	in[8] = 0;
	out[0] = 0;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;
	out[4] = 0;
	out[5] = 0;
	out[6] = 0;
	time[0] = 0;
	time[1] = 0;
	time[2] = 0;
	time[3] = 0;
	time[4] = 0;
	time[5] = 0;
}

void init_mem()
{
	mem_set_tmp[MEM_MB_BR] = eeprom_read_byte(&mem_set[MEM_MB_BR]);
	if ((mem_set_tmp[MEM_MB_BR] < MB_BR_4800) || mem_set_tmp[MEM_MB_BR] > MB_BR_38400)
	{
		mem_set_tmp[MEM_MB_BR] = MB_BR_4800;
		eeprom_write_byte(&mem_set[MEM_MB_BR], mem_set_tmp[MEM_MB_BR]);
	}

	mem_set_tmp[MEM_MB_PR] = eeprom_read_byte(&mem_set[MEM_MB_PR]);
	if ((mem_set_tmp[MEM_MB_PR] < MB_PR_NONE) || mem_set_tmp[MEM_MB_PR] > MB_PR_ODD)
	{
		mem_set_tmp[MEM_MB_PR] = MB_PR_NONE;
		eeprom_write_byte(&mem_set[MEM_MB_PR], mem_set_tmp[MEM_MB_PR]);
	}

	mem_set_tmp[MEM_MB_SB] = eeprom_read_byte(&mem_set[MEM_MB_SB]);
	if ((mem_set_tmp[MEM_MB_SB] < MB_SB_1) || mem_set_tmp[MEM_MB_SB] > MB_SB_2)
	{
		mem_set_tmp[MEM_MB_SB] = MB_SB_1;
		eeprom_write_byte(&mem_set[MEM_MB_SB], mem_set_tmp[MEM_MB_SB]);
	}

	mem_set_tmp[MEM_MB_BIT] = eeprom_read_byte(&mem_set[MEM_MB_BIT]);
	if ((mem_set_tmp[MEM_MB_BIT] < MB_BIT_5) || mem_set_tmp[MEM_MB_BIT] > MB_BIT_8)
	{
		mem_set_tmp[MEM_MB_BIT] = MB_BIT_5;
		eeprom_write_byte(&mem_set[MEM_MB_BIT], mem_set_tmp[MEM_MB_BIT]);
	}

	mem_set_tmp[MEM_MB_ADR] = eeprom_read_byte(&mem_set[MEM_MB_ADR]);
	if (mem_set_tmp[MEM_MB_ADR] < 1 || mem_set_tmp[MEM_MB_ADR] > 63)
	{
		mem_set_tmp[MEM_MB_ADR] = 1;
		eeprom_write_byte(&mem_set[MEM_MB_ADR], mem_set_tmp[MEM_MB_ADR]);
	}

	mem_set_tmp[MEM_LCD_C] = eeprom_read_byte(&mem_set[MEM_LCD_C]);
	if (mem_set_tmp[MEM_LCD_C] > 0x3f || mem_set_tmp[MEM_LCD_C] < 0x07)
	{
		mem_set_tmp[MEM_LCD_C] = 0x14;
		eeprom_write_byte(&mem_set[MEM_LCD_C], mem_set_tmp[MEM_LCD_C]);
	}
}
