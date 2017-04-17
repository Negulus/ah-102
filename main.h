#include <avr/eeprom.h>

//Светодиоды
#define	LED_R	(1<<2)
#define	LED_G	(1<<3)

//Константы кнопок
#define	BUT0_S	0
#define	BUT0_L	1
#define	BUT1_S	2
#define	BUT1_L	3
#define	BUT2_S	4
#define	BUT2_L	5
#define	BUT3_S	6
#define	BUT3_L	7
#define	BUT4_S	8
#define	BUT4_L	9
#define	BUT5_S	10
#define	BUT5_L	11

//Статусы системы
#define	STAT_RUN			(1<<0)	//работа системы
#define	STAT_LED0			(1<<1)	//индикатор красный
#define	STAT_LED1			(1<<2)	//индикатор зелёный
#define	STAT_BLOCK			(1<<3)	//блокировка системы
#define	STAT_TIMER			(1<<4)	//работа по таймеру
#define	STAT_TIMER_BLOCK	(1<<5)	//блокировка работы по таймеру
#define	STAT_MENU			(1<<6)	//меню открыто
#define	STAT_MENU_READ		(1<<7)	//меню чтение
#define	STAT_MENU_WRITE		(1<<8)	//меню запись
#define	STAT_MENU_READY		STAT_MENU_READ|STAT_MENU_WRITE	//меню готово
#define	STAT_SYNC			(1<<9)	//синхронизация
#define	STAT_ALARM_R		(1<<10)	//сброс аварий

//Режимы
#define	MODE_AUTO	(1<<0)
#define MODE_SUMMER	(1<<1)
#define MODE_WINTER	(1<<2)

//Входы (датчики)
#define SENS_OUT	(1<<0)
#define SENS_AIR	(1<<1)
#define SENS_WATER	(1<<2)
#define SENS_EX		(1<<3)
#define SENS_ROOM	(1<<4)
#define SENS_LOC	(1<<5)
#define SENS_OUT_H	(1<<6)
#define SENS_AIR_H	(1<<7)
#define SENS_ROOM_H	(1<<8)

//Выходы
#define OUT_HEAT	(1<<9)
#define OUT_COOL	(1<<10)
#define OUT_VENT1	(1<<11)
#define OUT_VENT2	(1<<12)
#define OUT_REC		(1<<13)
#define OUT_HUM		(1<<14)
#define OUT_DHUM	(1<<15)

#define SPEED_AUTO	(1<<8)

//Аварии
#define SYS_ER_FIRE		(1<<0)
#define SYS_ER_VENT1	(1<<1)
#define SYS_ER_VENT2	(1<<2)
#define SYS_ER_DPS1		(1<<3)
#define SYS_ER_DPS2		(1<<4)
#define SYS_ER_FILTER	(1<<5)
#define SYS_ER_OVERHEAT	(1<<6)
#define SYS_ER_FROST	(1<<7)
#define SYS_ER_AIN0		(1<<8)
#define SYS_ER_AIN1		(1<<9)
#define SYS_ER_AIN2		(1<<10)
#define SYS_ER_AIN3		(1<<11)
#define SYS_ER_AIN4		(1<<12)

//Переменные в EEPROM
#define	MEM_MB_BR		0
#define	MEM_MB_PR		1
#define	MEM_MB_SB		2
#define	MEM_MB_BIT		3
#define	MEM_MB_ADR		4
#define	MEM_LCD_C		5

//Хранение настроек в памяти
unsigned char mem_set_tmp[6];
extern unsigned char mem_set[6] EEMEM;

//Системные таймеры
unsigned int	t_ms;
unsigned int	t_s;
unsigned int	t_m;

//Системные переменные
int 			tmp;
char 			tmpc;
long			tmpl;
int				i;
int				j;

//Переменные и функции для кнопок
unsigned char	but_tmp;
unsigned int	but_t[6];
unsigned char	but_timer;
unsigned int	t_button;
unsigned int 	but;
void			but_upd(unsigned char bit, unsigned char ch);
void			but0call_s();
void			but0call_l();
void			but1call_s();
void			but1call_l();
void			but2call_s();
void			but2call_l();
void			but3call_s();
void			but3call_l();
void			but4call_s();
void			but4call_l();
void			but5call_s();
void			but5call_l();

int lcd_ac;

//Переменные для работы
unsigned int 	menu_line_en[5];	//Активные строки меню
unsigned int 	status;				//Статусы всей системы
unsigned int 	mode_en;			//Активные режимы
unsigned int 	mode;				//Текущие режимы
unsigned int 	speed_en;			//Активный скорости
unsigned int 	speed;				//Текущая скорость
unsigned int 	in[9];				//Датчики
unsigned int 	time[6];			//Время (день, месяц, год, неделя, часы, минуты)
unsigned int 	alarm;				//Аварии
unsigned int 	inout_en;			//Активные индикации
unsigned int 	inout;				//Текущие индикации
unsigned int 	out[7];				//Выходы
int 			set_t;				//Уставка температуры
unsigned int 	set_h;				//Уставка влажности
unsigned int	scr_set_tmp[16];	//Буфер для меню

/*
//Регистры modbus (старое)
unsigned int	p_reg_10[29];
unsigned int	p_reg_20[7];
unsigned int	*p_reg_u;
unsigned int	*p_reg_c;
unsigned int	*p_reg_e;
*/

//Регистры modbus
unsigned int	*reg_a[6];
unsigned int	*reg_b[14];
unsigned int	*reg_c[15];
unsigned int	*reg_d[16];

//Функции инициализации
void init_sys();
void init_reg();
void init_menu();
void init_mem();
