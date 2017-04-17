#include <avr/interrupt.h>
#include "twi.h"
#include "main.h"

void twi_init()
{
	twi.state = 0;
	twi.accept = 0;
	twi.func = 0;
	twi.cnt = 0;
	twi.ctu = 0;
	twi.adr = 0;
	twi.reg = 0;
	twi.line = 0;
	twi.device = 0;

	t_twi = 0;
	t_twi_set = 2000;

	for (int i = 0; i < 8; i++)
		twi.data[i] = 0;

	TWSR = 0;
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;

	sens_room.sens = 0;
	sens_room.state = 0;
	sens_room.sens_l = 0;
	sens_room.sens_h = 0;
}

void twi_read(unsigned char dev, unsigned char adr, unsigned char reg, unsigned char cnt)
{
	twi.state = 1;
	twi.adr = adr;
	twi.reg = reg;
	twi.ctu = 0;
	twi.cnt = cnt;
	twi.func = 1;
	twi.device = dev;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
	twi.accept = 0;
	TWCR |= (1<<TWSTA);
}

void twi_write(unsigned char dev, unsigned char adr, unsigned char reg, unsigned char cnt)
{
	twi.state = 1;
	twi.adr = adr;
	twi.reg = reg;
	twi.ctu = 0;
	twi.cnt = cnt;
	twi.func = 0;
	twi.device = dev;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
	twi.accept = 0;
	TWCR |= (1<<TWSTA);
}

ISR (TWI_vect)
{
	switch ((TWSR&0b11111000))
	{
	//START condition has been transmitted
	case 0x08:
		TWDR = twi.adr;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//repeated START condition has been transmitted
	case 0x10:
		TWDR = twi.adr + twi.func;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//SLA+W has been transmitted; ACK has been received
	case 0x18:
		TWDR = twi.reg;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//Data byte has been transmitted; ACK has been received
	case 0x28:
		if (twi.func == TWI_RD)
		{
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTA);
			return;
		}
		else
		{
			if (twi.ctu < twi.cnt)
			{
				TWDR = twi.data[twi.ctu];
				twi.ctu++;
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
				return;
			}
			else
			{
				t_twi_set = TWI_TIME;
				twi.line++;
				twi.state = 0;
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
				return;
			}
		}
	break;

	//SLA+R has been transmitted; ACK has been received
	case 0x40:
		if (twi.cnt > 1)
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		else
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
		return;
	break;

	//Data byte has been received; ACK has been returned
	case 0x50:
		twi.data[twi.ctu] = TWDR;
		twi.ctu++;
		if ((twi.ctu >= twi.cnt) || (twi.ctu > 7))
		{
			twi_receive();
			t_twi_set = TWI_TIME;
			twi.line++;
			twi.state = 0;
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
			return;
		}
		else
		{
			if (twi.cnt > 1)
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
			else
				TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
			return;
		}
	break;

	//Data byte has been received; NOT ACK has been returned
	case 0x58:
		twi.data[twi.ctu] = TWDR;
		twi_receive();
		t_twi_set = TWI_TIME;
		twi.line++;
		twi.state = 0;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
		return;
	break;

	//Data byte has been transmitted; NOT ACK has been received
	case 0x30:

	//SLA+W has been transmitted; NOT ACK has been received
	case 0x20:

	//Arbitration lost in SLA+R or NOT ACK bit
	case 0x38:

	//SLA+R has been transmitted; NOT ACK has been received
	case 0x48:

	default:
		t_twi_set = TWI_TIME;
		twi.state = 0;
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWSTO);
		return;
	break;
	}

	return;
}

void twi_receive()
{
	switch (twi.device)
	{
		case TWI_DEV_SENS:
			sens_proc();
		break;
	}
}

void sens_proc()
{
	switch (twi.line)
	{
	case 0:
		sens_room.state = twi.data[0];
	break;
	case 1:		
		sens_room.sens = twi.data[0]<<8;
		sens_room.sens |= twi.data[1];
		sens_room.sens >>= 4;
		if (sens_room.sens & (1<<11))
			sens_room.sens |= 0xF000;
		sens_room.sens *= 10;
		sens_room.sens /= 16;
		in[5] = sens_room.sens;
	break;
	case 2:		
		sens_room.sens_l = twi.data[0]<<8;
		sens_room.sens_l |= twi.data[1];
	break;
	case 3:		
		sens_room.sens_h = twi.data[0]<<8;
		sens_room.sens_h |= twi.data[1];
	break;
	}
}

void twi_queue()
{
	if (twi.accept == 1)
	{
		if (twi.line > 2)
			twi.line = 1;

		switch (twi.line)
		{
		//Настройка датчика температуры
		case 0:
			twi.data[0] = 0b01100000;
			twi_write(TWI_DEV_SENS, SENS_ADR, 0x01, 1);
		break;

		//Чтение данныx датчика температуры
		case 1:
			twi_read(TWI_DEV_SENS, SENS_ADR, 0x00, 2);
		break;

		default:
			twi.line = 1;
		}
	}
}
