#include <avr/eeprom.h>

//����������
#define	LED_R	(1<<2)
#define	LED_G	(1<<3)

//��������� ������
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

//������� �������
#define	STAT_RUN			(1<<0)	//������ �������
#define	STAT_LED0			(1<<1)	//��������� �������
#define	STAT_LED1			(1<<2)	//��������� ������
#define	STAT_BLOCK			(1<<3)	//���������� �������
#define	STAT_TIMER			(1<<4)	//������ �� �������
#define	STAT_TIMER_BLOCK	(1<<5)	//���������� ������ �� �������
#define	STAT_MENU			(1<<6)	//���� �������
#define	STAT_MENU_READ		(1<<7)	//���� ������
#define	STAT_MENU_WRITE		(1<<8)	//���� ������
#define	STAT_MENU_READY		STAT_MENU_READ|STAT_MENU_WRITE	//���� ������
#define	STAT_SYNC			(1<<9)	//�������������
#define	STAT_ALARM_R		(1<<10)	//����� ������

//������
#define	MODE_AUTO	(1<<0)
#define MODE_SUMMER	(1<<1)
#define MODE_WINTER	(1<<2)

//����� (�������)
#define SENS_OUT	(1<<0)
#define SENS_AIR	(1<<1)
#define SENS_WATER	(1<<2)
#define SENS_EX		(1<<3)
#define SENS_ROOM	(1<<4)
#define SENS_LOC	(1<<5)
#define SENS_OUT_H	(1<<6)
#define SENS_AIR_H	(1<<7)
#define SENS_ROOM_H	(1<<8)

//������
#define OUT_HEAT	(1<<9)
#define OUT_COOL	(1<<10)
#define OUT_VENT1	(1<<11)
#define OUT_VENT2	(1<<12)
#define OUT_REC		(1<<13)
#define OUT_HUM		(1<<14)
#define OUT_DHUM	(1<<15)

#define SPEED_AUTO	(1<<8)

//������
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

//���������� � EEPROM
#define	MEM_MB_BR		0
#define	MEM_MB_PR		1
#define	MEM_MB_SB		2
#define	MEM_MB_BIT		3
#define	MEM_MB_ADR		4
#define	MEM_LCD_C		5

//�������� �������� � ������
unsigned char mem_set_tmp[6];
extern unsigned char mem_set[6] EEMEM;

//��������� �������
unsigned int	t_ms;
unsigned int	t_s;
unsigned int	t_m;

//��������� ����������
int 			tmp;
char 			tmpc;
long			tmpl;
int				i;
int				j;

//���������� � ������� ��� ������
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

//���������� ��� ������
unsigned int 	menu_line_en[5];	//�������� ������ ����
unsigned int 	status;				//������� ���� �������
unsigned int 	mode_en;			//�������� ������
unsigned int 	mode;				//������� ������
unsigned int 	speed_en;			//�������� ��������
unsigned int 	speed;				//������� ��������
unsigned int 	in[9];				//�������
unsigned int 	time[6];			//����� (����, �����, ���, ������, ����, ������)
unsigned int 	alarm;				//������
unsigned int 	inout_en;			//�������� ���������
unsigned int 	inout;				//������� ���������
unsigned int 	out[7];				//������
int 			set_t;				//������� �����������
unsigned int 	set_h;				//������� ���������
unsigned int	scr_set_tmp[16];	//����� ��� ����

/*
//�������� modbus (������)
unsigned int	p_reg_10[29];
unsigned int	p_reg_20[7];
unsigned int	*p_reg_u;
unsigned int	*p_reg_c;
unsigned int	*p_reg_e;
*/

//�������� modbus
unsigned int	*reg_a[6];
unsigned int	*reg_b[14];
unsigned int	*reg_c[15];
unsigned int	*reg_d[16];

//������� �������������
void init_sys();
void init_reg();
void init_menu();
void init_mem();
