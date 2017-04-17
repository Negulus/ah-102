#ifndef __MODBUS_H__
#define __MODBUS_H__ 1

#include <avr/eeprom.h>

#define USART_BAUDRATE 9600 
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) 

//Статусы modbus
#define MB_RX		(1<<0)	//Приём
#define MB_TX		(1<<1)	//Передача
#define MB_WAIT		(1<<2)	//Ожидание ответа
#define MB_REP		(1<<3)	//Повторная посылка команды
#define MB_PROC		(1<<4)	//Обработка ответа
#define MB_WRITE	(1<<5)	//Посылка ответа

//Чётность
#define MB_PR_NONE	0x00
#define MB_PR_RES	0x01
#define MB_PR_EVEN	0x02
#define MB_PR_ODD	0x03

//Стоп биты
#define MB_SB_1		0x00
#define MB_SB_2		0x01

//Длина байта
#define MB_BIT_5	0x00
#define MB_BIT_6	0x01
#define MB_BIT_7	0x02
#define MB_BIT_8	0x03

//Скорость modbus
#define MB_BR_4800	0x01
#define MB_BR_9600	0x02
#define MB_BR_14400	0x03
#define MB_BR_19200	0x04
#define MB_BR_28800	0x05
#define MB_BR_38400	0x06

//Ошибки приёма
#define MB_ER_ADR	(1<<0)	//Другой адрес
#define MB_ER_OF	(1<<1)	//Переполнение при приёме
#define MB_ER_PR	(1<<2)	//Ошибка паритета
#define MB_ER_TIME	(1<<3)	//Преевышено время ожидания

//Адрес устройства
//#define MB_ADR		0x01

//Ошибки modbus
#define ER_FUNC		0x01
#define ER_ADR		0x02
#define ER_DATA		0x03
#define	ER_SLAVE	0x04
#define	ER_BUSY		0x06

//Таймеры для modbus
#define	MB_TIMEOUT		500
#define	MB_END_TIMEOUT	5
#define	T_MB_NEXT	50
#define	T_MB_CHECK	5000

//Структура modbus
typedef struct MB_DATA
{
	unsigned char	state;		//Статус ModBus
	unsigned char	error;		//Ошибки ModBus
	unsigned int	reg_n;		//Начальный регистр
	unsigned int	reg_v[16];	//Буфер регистров
	unsigned char	cnt;		//Количество регистров в работе
	unsigned char	num;		//Количество байт в сообщении
	unsigned char	rx[48];		//Буфер приёма
	unsigned char	tx[48];		//Буфер передачи
	unsigned char	ctu;		//Счётчик байт
	unsigned char	crc[2];		//Байты для хранения контрольной суммы
	unsigned int	crc16;		//Байты для расчёта контрольной суммы
	unsigned char	tmp;
	unsigned int	*reg_tmp;
	unsigned char	adr;

	//Таймеры работы ModBus
	unsigned int  t;
	unsigned int  t_set;
	unsigned int  t_next;
	unsigned int  t_next_set;
	unsigned int  t_check;
	unsigned int  t_stoptx;
	unsigned int  t_stoptx_set;
} mb_data;
mb_data	mb;

//Функции modbus
void	mb_init(unsigned char adr, unsigned char br, unsigned char pr, unsigned char sb, unsigned char bit);
void	mb_start_tx();
void	mb_start_rx();
void	mb_stop_rx();
void	mb_stop_tx();
void	crc_calc(unsigned char *m_xx);

//Функции таймеров modbus
void	timer_mb();
void	timer_mb_set(unsigned int set);
void	timer_mb_stoptx_set(unsigned int set);
void	timer_mb_check();
void	timer_mb_next();
void	timer_mb_next_set(unsigned int set);

//Функции для обработки принятых данных
void	mb_proc_start();
void	ans_01(unsigned int **reg_tmp);
void	ans_03(unsigned int **reg_tmp);
void	ans_05(unsigned int **reg_tmp);
void	ans_10(unsigned int **reg_tmp);
void	ans_error(unsigned char er);
void	mb_ans();

#endif
