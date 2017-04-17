#ifndef __MODBUS_H__
#define __MODBUS_H__ 1

#include <avr/eeprom.h>

#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) 

//������� modbus
#define MB_RX		(1<<0)	//����
#define MB_TX		(1<<1)	//��������
#define MB_WAIT		(1<<2)	//�������� ������
#define MB_REP		(1<<3)	//��������� ������� �������
#define MB_PROC		(1<<4)	//��������� ������
#define MB_WRITE	(1<<5)	//������� ������

//׸������
#define MB_PR_NONE	0x00
#define MB_PR_RES	0x01
#define MB_PR_EVEN	0x02
#define MB_PR_ODD	0x03

//���� ����
#define MB_SB_1		0x00
#define MB_SB_2		0x01

//����� �����
#define MB_BIT_5	0x00
#define MB_BIT_6	0x01
#define MB_BIT_7	0x02
#define MB_BIT_8	0x03

//�������� modbus
#define MB_BR_4800	0x01
#define MB_BR_9600	0x02
#define MB_BR_14400	0x03
#define MB_BR_19200	0x04
#define MB_BR_28800	0x05
#define MB_BR_38400	0x06

//������ �����
#define MB_ER_ADR	(1<<0)	//������ �����
#define MB_ER_OF	(1<<1)	//������������ ��� �����
#define MB_ER_PR	(1<<2)	//������ ��������
#define MB_ER_TIME	(1<<3)	//���������� ����� ��������

//����� ����������
//#define MB_ADR		0x01

//������ modbus
#define ER_FUNC		0x01
#define ER_ADR		0x02
#define ER_DATA		0x03
#define	ER_SLAVE	0x04
#define	ER_BUSY		0x06

//������� ��� modbus
#define	MB_TIMEOUT		500
#define	MB_END_TIMEOUT	5
#define	T_MB_NEXT	50
#define	T_MB_CHECK	5000

//��������� modbus
typedef struct MB_DATA
{
	unsigned char	state;		//������ ModBus
	unsigned char	error;		//������ ModBus
	unsigned int	reg_n;		//��������� �������
	unsigned int	reg_v[16];	//����� ���������
	unsigned char	cnt;		//���������� ��������� � ������
	unsigned char	num;		//���������� ���� � ���������
	unsigned char	rx[48];		//����� �����
	unsigned char	tx[48];		//����� ��������
	unsigned char	ctu;		//������� ����
	unsigned char	crc[2];		//����� ��� �������� ����������� �����
	unsigned int	crc16;		//����� ��� ������� ����������� �����
	unsigned char	tmp;
	unsigned int	*reg_tmp;
	unsigned char	adr;

	//������� ������ ModBus
	unsigned int  t;
	unsigned int  t_set;
	unsigned int  t_next;
	unsigned int  t_next_set;
	unsigned int  t_check;
	unsigned int  t_stoptx;
	unsigned int  t_stoptx_set;
} mb_data;
mb_data	mb;

//������� modbus
void	mb_init(unsigned char adr, unsigned char br, unsigned char pr, unsigned char sb, unsigned char bit);
void	mb_start_tx();
void	mb_start_rx();
void	mb_stop_rx();
void	mb_stop_tx();
void	crc_calc(unsigned char *m_xx);

//������� �������� modbus
void	timer_mb();
void	timer_mb_set(unsigned int set);
void	timer_mb_stoptx_set(unsigned int set);
void	timer_mb_check();
void	timer_mb_next();
void	timer_mb_next_set(unsigned int set);

//������� ��� ��������� �������� ������
void	mb_proc_start();
void	ans_01(unsigned int **reg_tmp);
void	ans_03(unsigned int **reg_tmp);
void	ans_05(unsigned int **reg_tmp);
void	ans_10(unsigned int **reg_tmp);
void	ans_error(unsigned char er);
void	mb_ans();

#endif
