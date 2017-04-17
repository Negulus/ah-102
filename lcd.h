#define LCD_SC	(1<<0)
#define LCD_RES	(1<<1)
#define LCD_A0	(1<<2)
#define LCD_RW	(1<<3)
#define LCD_E	(1<<4)
#define LCD_0	LCD_RES

#define LCD_NUM_Z	(1<<0)
#define LCD_NUM_P	(1<<1)

#define	MENU_MAIN	0
#define	MENU_PARAM	1
#define	MENU_SET	2
#define	MENU_CONF	3
#define	MENU_TIMER	4
#define	MENU_TIME	5
#define	MENU_MON	6
#define	MENU_TUE	7
#define	MENU_WED	8
#define	MENU_THU	9
#define	MENU_FRI	10
#define	MENU_SAT	11
#define	MENU_SUN	12

#define	LCD_STAT_SEL	(1<<0)		//выделение строки
#define	LCD_STAT_CLEAR	(1<<1)		//очистка экрана

//Переменные для дисплея

unsigned char 	lcd_stat;			//Статусы дисплея
unsigned char	lcd_contr;

unsigned int	lcd_i;
unsigned int	lcd_j;
unsigned int	lcd_k;

unsigned char	lcd_n_i;
unsigned char	lcd_n_j;
unsigned char	lcd_n_k;
unsigned char	lcd_n_z;
unsigned char	lcd_n_tmpc;
unsigned int	lcd_n_tmp;

unsigned char	lcd_m_tmpc;

unsigned char	lcd_tmpuc;
unsigned char	lcd_tmpc;
unsigned int	lcd_tmp;
unsigned char	lcd_start;
char			lcd_num[21];

//Системные функции
void lcd_init();
void lcd_wait();
void lcd_out_i(unsigned char data);
void lcd_out_d(unsigned char data);
void lcd_setpage(unsigned char page);
void lcd_setadr(unsigned char adr);
void lcd_clear();
void lcd_contrast();

//Пользовательские функции
void lcd_out_string(unsigned char page, unsigned char adr, unsigned char lcd_len);
void lcd_out_string_s(unsigned char page, unsigned char adr, unsigned char lcd_len);
void lcd_out_num(signed int lcd_in, char *lcd_out, unsigned char lcd_len, unsigned char lcd_float, unsigned char lcd_type);
void str_cpy(char lcd_in[21], char *lcd_out, unsigned char lcd_len);

void lcd_out_logo();

void lcd_out_line_u();
void lcd_out_line_d();
void lcd_out_time();
void lcd_out_speed();
void lcd_out_mode_1();
void lcd_out_mode_2();
void lcd_out_mode_3();
void lcd_out_mode_4();

//Функции отрисовки
void lcd_draw();
void lcd_scr_0();
void lcd_scr_1(unsigned char sel, unsigned int line_en);
void lcd_menu_config();
void lcd_menu_timer();
void lcd_menu_time();
void lcd_menu(unsigned char sel);

//Переменные для экранов
unsigned char	scr_sel;		//Текущий экран
unsigned char	scr_menu;		//Текущее меню
unsigned char	scr_line[6];	//Строки меню на экране
signed int		scr_line_tmp;	//Буфер параметра
unsigned char	scr_line_f;		//Первая строка на экране
unsigned char	scr_line_sel;	//Выделенная строка

//unsigned int	scr_line_en[5];	//Активные строки меню
