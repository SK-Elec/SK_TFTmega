#ifndef S6D0129_H_
#define S6D0129_H_

/* LCD colour */
#define White          0xFFFF
#define Black          0x0000
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

#define COL_RED			0x105F
#define COL_GREEN		0x07E0
#define COL_BLUE		0xF9E7
#define COL_YELLOW		0x07FF
#define COL_PURPLE		0xF81F
#define COL_BLACK		0x0000
#define COL_WHITE		0xFFFF
#define COL_ORANGE		0x2BFF

#define  LCD_DATA		PORTA
#define  LCD_DATA_DIR	DDRA
#define  LCD_CTRL		PORTC
#define  LCD_CTRL_DIR	DDRC
#define  LCD_RS			PC6
#define  LCD_WR			PC5
#define  LCD_RD			PC4
#define  LCD_CS			PC7
#define  LCD_REST		PC1

void LCD_Write_COM(unsigned int DH);
void LCD_Write_DATA(unsigned int DH);
unsigned int LCD_Read_DATA(void);
void LCD_W_com_data(unsigned int com1, unsigned int dat1);
void LCD_SetPos(char x1,char x2,unsigned int y1,unsigned int y2);
void LCD_Init(void);
void LCD_Fill(unsigned int color);
void LCD_Rect(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, uint16_t color);
void LCDpix(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,const unsigned char *pic,unsigned short color);
void LCDCharDisp(unsigned char charactor,uint16_t sx,uint16_t sy, uint8_t font, uint16_t colour);
void LCDPrintStr(unsigned int x,unsigned int y,char *str,uint8_t font, uint16_t colour);
unsigned char Get_font(uint8_t font, uint16_t k);
void LCDpicture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,const unsigned char *pic);
void LCD_Digit(int32_t integer, uint16_t x_pos, uint16_t y_pos, uint8_t digits, uint8_t decimals, uint8_t font, uint16_t colour);

unsigned char NULL_PORT_DIR,NULL_PORT,DATA_DIR,DATA_PORT,DATA_PIN;

#define LCD_RST_DPRT	DDRC
#define LCD_RST_PR	PORTC
#define LCD_RST_PRTC	PORTC
#define LCD_RST_PIN		PC1

#define LCD_BL_DPRT		DDRC
#define LCD_BL_PR		PORTC
#define LCD_BL_PRTC		PORTC
#define LCD_BL_PIN		PC3

#define LCD_RS_DPRT		DDRC
#define LCD_RS_PR		PORTC
#define LCD_RS_PRTC		PORTC
#define LCD_RS_PIN		PC6

#define LCD_CS_DPRT		DDRC
#define LCD_CS_PR		PORTC
#define LCD_CS_PRTC		PORTC
#define LCD_CS_PIN		PC7

#define LCD_RD_DPRT		DDRC
#define LCD_RD_PR		PORTC
#define LCD_RD_PRTC		PORTC
#define LCD_RD_PIN		PC4

#define LCD_WR_DPRT		DDRC
#define LCD_WR_PR		PORTC
#define LCD_WR_PRTC		PORTC
#define LCD_WR_PIN		PC5

#define LCD_DB_DPRT		DDRA
#define LCD_DB_PORT		PORTA
#define LCD_DB_PINP		PINA

#define CS_DPRT			NULL_PORT_DIR
#define CS_PR			NULL_PORT
#define CS_PRTC			NULL_PORT
#define CS_PIN			1

#define PEN_DPRT		NULL_PORT_DIR
#define PEN_PR			NULL_PORT
#define PEN_PRTC		NULL_PORT
#define PEN_PIN			0


#endif
