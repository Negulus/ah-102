#include <avr/io.h>
#include <avr/interrupt.h>
#include "modbus.h"
#include "main.h"
#include "twi.h"

//unsigned char ctu;

//Инициализация
void mb_init(unsigned char adr, unsigned char br, unsigned char pr, unsigned char sb, unsigned char bit)
{
	mb.state = 0;
	mb.error = 0;
	mb.cnt = 0;
	mb.num = 0;
	mb.ctu = 0;
	mb.crc[0] = 0;
	mb.crc[1] = 0;
	mb.crc16 = 0;
	mb.adr = adr;

	UCSRA = 0;
	UCSRB = 0;

	switch (br)
	{
	case MB_BR_4800:
		tmp = 4800;
	break;
	case MB_BR_9600:
		tmp = 9600;
	break;
	case MB_BR_14400:
		tmp = 14400;
	break;
	case MB_BR_19200:
		tmp = 19200;
	break;
	case MB_BR_28800:
		tmp = 28800;
	break;
	case MB_BR_38400:
		tmp = 38400;
	break;
	}

	UBRRL = (((F_CPU / (tmp * 16UL))) - 1) ;
	UBRRH = ((((F_CPU / (tmp * 16UL))) - 1)  >> 8);
	UCSRC = (1<<URSEL);
	UCSRC |= (1<<URSEL)|(pr<<4)|(sb<<3)|(bit<<1);
	UCSRC &= ~(1<<URSEL);

	DDRC |= (1<<7);
	PORTC &= ~(1<<7);

	mb.t = 0;
	mb.t_set = 0;
	mb.t_next = 0;
	mb.t_next_set = 0;
	mb.t_check = 0;
	mb.t_stoptx = 0;
	mb.t_stoptx_set = 0;

	timer_mb_set(0);

	mb_start_rx();
}

//Начало приёма
void mb_start_rx()
{
	if ((mb.state & MB_TX) != 0)
	{
		UCSRB &= ~(1<<3);
		PORTC &= ~(1<<7);
		mb.state &= ~MB_TX;
	}

	PORTC &= ~(1<<7);

	mb.error = 0;
	mb.state |= MB_RX;
	mb.state &= ~MB_PROC;

	mb.ctu = 0;
	
	UCSRB |= (1<<4);
	UCSRB |= (1<<7);
}

//Приём байта
ISR (USART_RXC_vect)
{
	mb.tmp = UDR;
	if ((mb.state & MB_RX) == 0)
		return;

	timer_mb_set(MB_END_TIMEOUT);

	if (mb.error != 0)
		return;

	if (UCSRA & (1<<2))
		mb.error |= MB_ER_PR;

	mb.rx[mb.ctu] = mb.tmp;
	
	if (mb.ctu == 0)
	{
		if (mb.rx[0] != mb.adr)
		{
			mb.error |= MB_ER_ADR;
			return;
		}
	}

	mb.ctu++;

	if (mb.ctu >= 48)
		mb.error |= MB_ER_OF;

	return;
}

//Остановка приёма
void mb_stop_rx()
{
	if ((mb.state & MB_RX) == 0)
		return;

	UCSRB &= ~(1<<7);
	UCSRB &= ~(1<<4);
	
	if (mb.error & MB_ER_ADR)
	{
		mb_start_rx();
		return;
	}

	mb.state |= MB_PROC;
	mb.state &= ~MB_RX;
	mb_proc_start();
}

//Начало передачи
void mb_start_tx()
{
	if (mb.state & MB_RX)
	{
		UCSRB &= ~(1<<7);
		UCSRB &= ~(1<<4);
		mb.state &= ~MB_RX;
	}

	mb.state |= MB_TX;
	mb.state &= ~MB_PROC;
	mb.error = 0;

	if (mb.t_next < mb.t_next_set)
		return;

	mb.t_check = 0;

	UCSRB = (1<<3);

	crc_calc(&mb.tx[0]);
	mb.tx[mb.num] = mb.crc[1];
	mb.tx[mb.num+1] = mb.crc[0];
	mb.num += 2;
	mb.ctu = 0;

	PORTC |= (1<<7);
	UCSRB |= (1<<5);
}

//Отправка следующего байта
ISR (USART_UDRE_vect)
{
	if (!(mb.state & MB_TX))
		return;

	if (mb.ctu >= mb.num)
	{
		timer_mb_stoptx_set(5);
		UCSRB &= ~(1<<5);
	}
	else
	{
		UDR = mb.tx[mb.ctu];
		mb.ctu++;
	}
}

//Окончание передачи (после отправки последнего байта)
void mb_stop_tx()
{
	if (!(mb.state & MB_TX))
		return;

	UCSRB &= ~(1<<3);
	PORTC &= ~(1<<7);

	mb.state &= ~MB_TX;

	mb_start_rx();
	return;
}

//Подсчёт контрольной суммы
void crc_calc(unsigned char *mb_xx)
{
	mb.crc16=0xFFFF;
	for (i = 0; i < mb.num; i++)
	{
		mb.crc16 ^= *mb_xx++;
		for (j=0; j<8; j++)
		{
			if (mb.crc16 & 0x0001) 
			{
				mb.crc16 >>= 1;
				mb.crc16 ^= 0xA001;
			}
			else
				mb.crc16 >>= 1;
		}
	}
	mb.crc[1] = mb.crc16&0xff;
	mb.crc[0] = (mb.crc16 >> 8);
	return;
}

void ans_01(unsigned int **reg_tmp)
{
	mb.tx[0] = mb.adr;
	mb.tx[1] = 0x01;

	tmp = *(*reg_tmp);
	tmp <<= (8 - (mb.rx[3] + mb.rx[5]));
	tmp >>= (8 - mb.rx[5]);
	mb.tx[3] = tmp&0xff;
	mb.num = 4;

	if (mb.rx[5] <= 8)
		mb.tx[2] = 1;
	else
	{
		mb.num++;
		mb.tx[2] = 2;
		mb.tx[4] = mb.tx[3];
		mb.tx[3] = (tmp >> 8);
	}
}

void ans_03(unsigned int **reg_tmp)
{
	mb.tx[0] = mb.adr;
	mb.tx[1] = 0x03;
	mb.tx[2] = mb.rx[5] * 2;
	mb.num = 3 + mb.tx[2];

	for (i = 0; i < mb.rx[5]; i++)
	{
		if (mb.rx[2] == 0x0B && (i + mb.rx[3]) == 0 && (status & STAT_MENU_READ) && (status & STAT_MENU_WRITE))
		{
			tmp = status & ~(STAT_MENU_READ|STAT_MENU_WRITE);
			mb.tx[(i*2)+3] = tmp>>8;
			mb.tx[(i*2)+4] = tmp&0xff;
		}
		else
		{
			//Понятия не имею, как эти и подобные строчки работают
			mb.tx[(i*2)+3] = *(*(reg_tmp + (i + mb.rx[3])))>>8;
			mb.tx[(i*2)+4] = *(*(reg_tmp + (i + mb.rx[3])))&0xff;
		}
	}
}

//Write single coil
void ans_05(unsigned int **reg_tmp)
{
	if (mb.rx[3] == 6)
		status |= STAT_MENU_READY;
	else
	{
		if (mb.rx[4] == 0)
			*(*reg_tmp) &= ~(1<<mb.rx[3]);
		else
			*(*reg_tmp) |= (1<<mb.rx[3]);
	}

	mb.tx[0] = mb.adr;
	mb.tx[1] = 0x05;
	mb.tx[2] = mb.rx[2];
	mb.tx[3] = mb.rx[3];
	mb.tx[4] = mb.rx[4];
	mb.tx[5] = mb.rx[5];
	mb.num = 6;
}

void ans_10(unsigned int **reg_tmp)
{
	for (i = 0; i < mb.rx[5]; i++)
	{
		tmp = mb.rx[7+(i*2)];
		tmp <<= 8;
		tmp |= mb.rx[8+(i*2)];

		if (mb.rx[2] == 0x0B)
		{
			if ((i + mb.rx[3]) < 4)
			{
				if (tmp == 0)
					continue;
			}
		}

		*(*(reg_tmp + (i + mb.rx[3]))) = tmp;
	}

	mb.tx[0] = mb.adr;
	mb.tx[1] = 0x10;
	mb.tx[2] = mb.rx[2];
	mb.tx[3] = mb.rx[3];
	mb.tx[4] = mb.rx[4];
	mb.tx[5] = mb.rx[5];
	mb.num = 6;
}

void ans_error(unsigned char er)
{
	mb.tx[0] = mb.adr;
	mb.tx[1] = mb.rx[1] + 0x80;
	mb.tx[2] = er;
	mb.num = 3;
}

void mb_proc_start()
{
	if (mb.error == 0)
	{
		if (mb.rx[1] < 7)
			mb.num = 6;
		else if (mb.rx[1] == 0x0F)
			mb.num = mb.rx[6] * 2 + mb.rx[7];
		else if (mb.rx[1] == 0x10)
			mb.num = mb.rx[6] + mb.rx[7];
		else
			mb.num = 3;

		mb_ans();
		timer_mb_next_set(T_MB_NEXT);

	}
	else
	{
		if ((mb.error & MB_ER_OF) > 0 || (mb.error & MB_ER_PR) > 0)
		{
			ans_error(ER_FUNC);
			timer_mb_next_set(T_MB_NEXT);
		}
		else
		{
			mb_start_rx();
		}
	}
}

void mb_ans()
{
	switch (mb.rx[2])
	{
		case 0x0A:
			tmp = 6;
		break;
		case 0x0B:
			tmp = 14;
		break;
		case 0x0C:
			tmp = 15;
		break;
		case 0x0D:
			tmp = 16;
		break;
		case 0x0E:
			tmp = 6;
		break;
		default:
			ans_error(ER_ADR);
			return;
	}

	switch (mb.rx[1])
	{
	//Read holding registers
	case 0x03:
		if (mb.rx[5] == 0 && mb.rx[4] == 0)
			ans_error(ER_DATA);
		else if ((mb.rx[4] > 0) || ((mb.rx[3] + mb.rx[5]) > tmp))
			ans_error(ER_ADR);
		else
		{
			switch (mb.rx[2])
			{
				case 0x0A:
					ans_03(&reg_a[0]);
				break;
				case 0x0B:
					ans_03(&reg_b[0]);
				break;
				case 0x0C:
					ans_03(&reg_c[0]);
				break;
				case 0x0D:
					ans_03(&reg_d[0]);
				break;
				default:
					ans_error(ER_ADR);
			}
		}
	break;

	//Write single coil
	case 0x05:
		if ((mb.rx[5] > 0) || (mb.rx[4] != 0 && mb.rx[4] != 0xFF))
			ans_error(ER_DATA);
		else if (mb.rx[3] > tmp)
			ans_error(ER_ADR);
		else
		{
			switch (mb.rx[2])
			{
				case 0x0E:
					ans_05(&reg_b[0]);
				break;
				default:
					ans_error(ER_ADR);
			}
		}
	break;

	//Write multiple registers
	case 0x10:
		if (mb.rx[5] == 0 && mb.rx[4] == 0)
			ans_error(ER_DATA);
		else if ((mb.rx[4] > 0) || ((mb.rx[3] + mb.rx[5]) > tmp))
			ans_error(ER_ADR);
		else
		{
			switch (mb.rx[2])
			{
				case 0x0A:
					ans_10(&reg_a[0]);
				break;
				case 0x0B:
					if (mb.rx[3] == 0)
						ans_error(ER_ADR);
					else
						ans_10(&reg_b[0]);
				break;
				case 0x0C:
					ans_10(&reg_c[0]);
				break;
				case 0x0D:
					ans_10(&reg_d[0]);
				break;
				default:
					ans_error(ER_ADR);
			}
		}
	break;

	//Function error
	default:
		ans_error(ER_FUNC);
	break;
	}
}
