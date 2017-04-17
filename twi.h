#define SCL_CLOCK		400000L
#define TWI_TIME		20
#define SENS_ADR		0b10010110
#define TIME_ADR		0xA2

#define TWI_RD	1
#define TWI_WR	0

#define TWI_DEV_TIME	1
#define TWI_DEV_SENS	2

typedef struct TWI_DATA
{
	unsigned char	state;
	unsigned char	accept;
	unsigned char	func;
	unsigned char	cnt;
	unsigned char	ctu;
	unsigned char	adr;
	unsigned char	reg;
	unsigned char	data[8];
	unsigned char	line;
	unsigned char	device;
} twi_data;
twi_data	twi;

unsigned int t_twi;
unsigned int t_twi_set;

void twi_init();
void twi_read(unsigned char dev, unsigned char adr, unsigned char reg, unsigned char cnt);
void twi_write(unsigned char dev, unsigned char adr, unsigned char reg, unsigned char cnt);
void twi_receive();
void twi_queue();

typedef struct SENS_DATA
{
	signed int		sens;	//Temperature Register
	unsigned char	state;	//Configuration Register
	signed int		sens_l;	//TLOW Register
	signed int		sens_h;	//THIGH Register
} sens_data;

//Данные датчика температуры
sens_data	sens_room;
void		sens_proc();
