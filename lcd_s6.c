
#include "app_config_s6.h"

unsigned char font_width,font_height;

void LCD_Write_COM(unsigned int DH)
{
    Clrb(LCD_CTRL,LCD_RS);
	Clrb(LCD_CTRL,LCD_CS);
	LCD_DATA=DH>>8;
	Clrb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_WR);
	LCD_DATA=DH;
	Clrb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_CS);
	Setb(LCD_CTRL,LCD_RS);
}
void LCD_Write_DATA(unsigned int DH)
{
	Clrb(LCD_CTRL,LCD_CS);
	LCD_DATA=DH>>8;
	Clrb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_WR);
	LCD_DATA=DH;
	Clrb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_CS);
}
unsigned int LCD_Read_DATA(void)
{
	uint16_t data;
	LCD_DATA_DIR = 0x00;
	Clrb(LCD_CTRL,LCD_CS);
	Clrb(LCD_CTRL,LCD_RD);
	Clrb(LCD_CTRL,LCD_RD);
	Setb(LCD_CTRL,LCD_RD);
	data = PINA << 8;
	Clrb(LCD_CTRL,LCD_RD);
	Clrb(LCD_CTRL,LCD_RD);
	Setb(LCD_CTRL,LCD_RD);
	data |= PINA;
	Setb(LCD_CTRL,LCD_CS);
	LCD_DATA_DIR = 0xFF;
	return data;
}

void LCD_W_com_data(unsigned int com1, unsigned int dat1)
{
   LCD_Write_COM(com1);
   LCD_Write_DATA(dat1);
}
void LCD_SetPos(char x1,char x2,unsigned int y1,unsigned int y2)
{
    uint16_t x_pos;
	x_pos = (x2<<8) | x1;
	LCD_Write_COM(0x0046);LCD_Write_DATA(x_pos);
	LCD_Write_COM(0x0047);LCD_Write_DATA(y2);
    LCD_Write_COM(0x0048);LCD_Write_DATA(y1);
	LCD_Write_COM(0x0020);LCD_Write_DATA(x1);
    LCD_Write_COM(0x0021);LCD_Write_DATA(y1);

    LCD_Write_COM(0x0022);
}

void LCD_Init(void)
{
	LCD_CTRL_DIR = 0xF2;
	LCD_DATA_DIR = 0xFF;

    Setb(LCD_CTRL,LCD_REST);
    _delay_ms(5);
	Clrb(LCD_CTRL,LCD_REST);
	_delay_ms(5);
	Setb(LCD_CTRL,LCD_REST);
	Setb(LCD_CTRL,LCD_CS);
	Setb(LCD_CTRL,LCD_RD);
	Setb(LCD_CTRL,LCD_WR);
	Setb(LCD_CTRL,LCD_RS);
	_delay_ms(5);

	Clrb(LCD_CTRL,LCD_CS);
	//************* Start Initial Sequence **********//

	LCD_W_com_data(0x0000,0x0001); _delay_ms(100);

	LCD_W_com_data(0x0011,0x2E00);
    LCD_W_com_data(0x0014,0x040B);
    LCD_W_com_data(0x0010,0x1040);	_delay_ms(10);
    LCD_W_com_data(0x0013,0x0040);	_delay_ms(10);
	LCD_W_com_data(0x0013,0x0060);	_delay_ms(10);
	LCD_W_com_data(0x0013,0x0070);	_delay_ms(6);
	LCD_W_com_data(0x0011,0x3704);
	LCD_W_com_data(0x0010,0x1600);	_delay_ms(2);
	LCD_W_com_data(0x0001,0x0B27);		// 0x0927
    LCD_W_com_data(0x0002,0x0700);
	LCD_W_com_data(0x0003,0x1038);		//(0x1028,0x1030,0x1010) Screen Orientation
    LCD_W_com_data(0x0007,0x0004);		// 0x0004
    LCD_W_com_data(0x0008,0x0505);
    LCD_W_com_data(0x0009,0x0000);
    LCD_W_com_data(0x000B,0x0000);
    LCD_W_com_data(0x000C,0x0000);

    LCD_W_com_data(0x0040,0x0000);
	#if defined LCD_REVERSE
		LCD_W_com_data(0x0042,0x013F);
		LCD_W_com_data(0x0043,0x0000);
		LCD_W_com_data(0x0044,0x013F);
		LCD_W_com_data(0x0045,0x0000);
	#endif
    LCD_W_com_data(0x0046,0xEF00);
    LCD_W_com_data(0x0047,0x013F);
    LCD_W_com_data(0x0048,0x0000);	_delay_ms(5);

    LCD_W_com_data(0x0030,0x0000);
    LCD_W_com_data(0x0031,0x0006);
    LCD_W_com_data(0x0032,0x0000);
    LCD_W_com_data(0x0033,0x0000);
    LCD_W_com_data(0x0034,0x0707);
    LCD_W_com_data(0x0035,0x0700);
    LCD_W_com_data(0x0036,0x0303);
    LCD_W_com_data(0x0037,0x0007);
    LCD_W_com_data(0x0038,0x1100);
    LCD_W_com_data(0x0039,0x1100);

    LCD_W_com_data(0x0007,0x0015);	_delay_ms(4);		// 0x0015
    LCD_W_com_data(0x0007,0x0017);	_delay_ms(4);		// 0x0017

    LCD_W_com_data(0x0020,0x0000);	_delay_ms(10);		//Set Address... LSB 0x10 (76560)
    LCD_W_com_data(0x0021,0x0000);	_delay_ms(10);      //Set Address... HSB 0x12B
	LCD_Write_COM(0x0022);

	Setb(LCD_CTRL,LCD_CS);
}

void LCDCharDisp(unsigned char charactor,uint16_t sx,uint16_t sy, uint8_t font,uint16_t colour) //low level function to print a character on LCD
{
	unsigned int i,j,x,y,ex,ey,k=0;
	unsigned char h=0;
	unsigned int pic_byte; //,bg_colour;

	pic_byte = Get_font(font, k);

	ex = sx + font_width - 1;
	ey = sy + font_height - 1;

	//buf_store(charactor - 0x20);
	k = ((charactor - 0x20) * font_width * (font_height/8));

	LCD_SetPos(sy,ey,sx,ex);
	x = ex - sx + 1;
	y = ey - sy + 1;

	for (j=0; j<y; j++)
	for (i=0; i<x; i++)
	{
		if (h == 0) {
			pic_byte = Get_font(font, k);
			k++;
		}
		if (pic_byte & (1<<h))
		{
			LCD_Write_DATA(colour);
		} else {
			//bg_colour = LCD_Read_DATA();
			LCD_Write_DATA(Black);
		}
		h++;
		if (h == 8) { h = 0; }
	}
}

void LCDPrintStr(unsigned int x,unsigned int y,char *str,uint8_t font, uint16_t colour) //print string on LCD
{
	unsigned int i=0;

	while(str[i])
	{
		LCDCharDisp(str[i],x,y,font,colour);
		x += font_width;
		i++;
	}
}

void LCDpix(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,const unsigned char *pic,unsigned short color) //display a pixmap
{
	unsigned int i,j;
	unsigned long k=0;
	unsigned char pic_byte,h=0;

	LCD_SetPos(sy,sy+ey-1,sx,sx+ex-1);

	for (j=0; j<ey; j++)
	for (i=0; i<ex; i++)
	{
		if (h == 0) {
			pic_byte = pgm_read_byte(&pic[k]);
			k++;
		}
		if (pic_byte & (1<<h))
		{
			LCD_Write_DATA(color);
			} else {
			LCD_Write_DATA(Black);
		}
		h++;
		if (h == 8) { h = 0; }
	}
}

void LCD_Rect(uint16_t x_pos, uint16_t y_pos, uint16_t x_size, uint16_t y_size, uint16_t color)
{
	uint16_t i,j;
	LCD_SetPos(y_pos,y_size+y_pos,x_pos,x_size+x_pos);
	for (i=0; i<=x_size;i++) {
		for (j=0;j<=y_size;j++) {
			LCD_Write_DATA(color);
		}
	}
}

void LCD_Fill(unsigned int color)
{
	unsigned int i,j;
	LCD_SetPos(0,239,0,319);
	for(i=0;i<320;i++) {
		for (j=0;j<240;j++)	{
			LCD_Write_DATA(color);
		}
	}
}

unsigned char Get_font(uint8_t font, uint16_t k)
{
	uint8_t char_byte = 0;
	switch (font)
	{
		#if defined Consolas_Numbers
		case Consolas_Numbers :
			font_width = Consolas_Numbers_width; font_height = Consolas_Numbers_height;
			char_byte = pgm_read_byte(&Consolas_Numbers_bits[k]);
			break;
		#endif
		#if defined Comic_32x16
		case Comic_32x16 :
			font_width = Comic_32x16_width; font_height = Comic_32x16_height;
			char_byte = pgm_read_byte(&Comic_32x16_bits[k]);
			break;
		#endif
		#if defined Consolas_32x16
		case Consolas_32x16 :
			font_width = Consolas_32x16_width; font_height = Consolas_32x16_height;
			char_byte = pgm_read_byte(&Consolas_32x16_bits[k]);
			break;
		#endif
		#if defined Comic_24x16
		case Comic_24x16 :
			font_width = Comic_24x16_width; font_height = Comic_24x16_height;
			char_byte = pgm_read_byte(&Comic_24x16_bits[k]);
			break;
		#endif
		#if defined Consolas_Bold_16x8
		case Consolas_Bold_16x8 :
			font_width = Consolas_Bold_16x8_width; font_height = Consolas_Bold_16x8_height;
			char_byte = pgm_read_byte(&Consolas_Bold_16x8_bits[k]);
			break;
		#endif
		#if defined Courier_New_16x8
		case Courier_New_16x8 :
			font_width = Courier_New_16x8_width; font_height = Courier_New_16x8_height;
			char_byte = pgm_read_byte(&Courier_New_16x8_bits[k]);
			break;
		#endif
		#if defined Impact_16x8
		case Impact_16x8 :
			font_width = Impact_16x8_width; font_height = Impact_16x8_height;
			char_byte = pgm_read_byte(&Impact_16x8_bits[k]);
			break;
		#endif
	}
	return char_byte;
}

void LCDpicture(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,const unsigned char *pic)	// display full colour picture
{
	unsigned int pic_byte,i,j;
	unsigned long long k=1;
	//unsigned char h=0;

	LCD_SetPos(sy,sy+ey-1,sx,sx+ex-1);

	for (j=0; j<ey; j++)
	for (i=0; i<ex; i++)
	{
		//pic_byte = pgm_read_word(&pic[k]);
		pic_byte = pgm_read_byte(&pic[k]);
		k++;
		pic_byte |= (pgm_read_byte(&pic[k]) << 8);
		k++;
		LCD_Write_DATA(pic_byte);
	}
}

void LCD_Digit(int32_t integer, uint16_t x_pos, uint16_t y_pos, uint8_t digits, uint8_t decimals, uint8_t font, uint16_t colour)
{
	char array[10] = {0,0,0,0,0,0,0,0,0,0}, out_array[10] =  {0,0,0,0,0,0,0,0,0,0};
	unsigned char i=digits,j=0, k=0;
	ltoa(integer,array,10);

	Get_font(font,0);

	for (j = 0; j <= digits; j++) {
		if ((i == decimals - 1) && (decimals > 0)) {
			out_array[j] = 0x2E;		// "."
			j++;
			digits++;
		}
		if (array[i] == 0) {
			if (i < decimals+1) {
				out_array[j] = 0x30;		// "0"
			} else {
				out_array[j] = 0x20;		// "space"
			}
		} else {
			out_array[j] = array[k];
			k++;
		}
		i--;
	}

	LCDPrintStr(x_pos, y_pos, out_array, Consolas_32x16, White);
}