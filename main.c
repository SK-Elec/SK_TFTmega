/*
 * MegaTFT-S6.c
 *
 * Created: 29/01/2016 9:28:59 PM
 * Author : SKFazer
 */


#include "app_config_s6.h"

#define NORM	1
#define REV		0
#define ARROW_XPOS	255
#define ARROW_YPOS	36
#define HL_X	180
#define HL_Y	31
#define AVG_HPOS 103
#define INS_HPOS (AVG_HPOS+33)
#define SMALL_FONT	Consolas_Bold_16x8

void USART_Init( unsigned int ubrr, unsigned char usart_no);
void USART_Transmit(uint8_t ByteToSend);
void Display_Initial(void);
void Hex2Dec(uint16_t value);
void main_init(void);
void startup_eeprom(void);
void rav_pin_functions(void);
void save_eeprom(void);

volatile unsigned char rpm=0,rpm_count=0,uart_buffer[200],tx_c=0,rx_c=0;				// setup rpm global variable
volatile uint8_t k=12, l=1, flag=0, hexdec[5], LCDflag = 0, fuel_array_inc;
volatile uint16_t speed_ins_div=0, fuel_array_avg = 0;
volatile uint64_t fuel_avg=1,speed_avg=1;
volatile uint32_t fuel_array_avg1=0, fuel_array_inc1=0, rpm_inc=1,fuel_inc=1, speed_inc=1, fuel_ins=0, speed_ins=0, speed_ins_temp=0, fuel_ins_count=0, speed_ins_count=0, fuel_array_sum = 0;
uint32_t EEMEM ee_fuel_h=1, EEMEM ee_fuel_l=1, EEMEM ee_fueli=1;
uint32_t EEMEM ee_speed_h=1, EEMEM ee_speed_l=1, EEMEM ee_speedi=1;
uint16_t EEMEM ee_speed_divisor=0;
uint8_t EEMEM ee_fuel_array[256], EEMEM ee_fuel_inc=0;
volatile uint16_t j=0, fuel_range=0, fuel_guage=0, fuel_guage_disp, fuel_disp=0, speed_divisor=44000, speed_actual, speed_actual_avg;   //fuel_avg_disp=0,

int main(void)
{
    uint8_t i, k=0, m=20, n=0; //, fuel_guage_array[256];

	for (i=0;i<100;i++) _delay_ms(30);
	USART_Init(MYUBRR, 0);
    //USART_Init(MYUBRR, 1);
    main_init();
	LCD_Init();
    LCD_Fill(Black);

	startup_eeprom();

	for (i=0;i<255;i++) fuel_array_sum += eeprom_read_byte(&ee_fuel_array[i]);
	fuel_array_avg = fuel_array_sum >> 8;
	Display_Initial();

	sei();

    PORTF = 0x00;
    PORTD = 0x6B;
    PORTG = 0xFF;

    while(1)
    {
	    rav_pin_functions();
		save_eeprom();
	    j++;
	    if (j == 0x009F)
	    {
		    j = 0;
		    // Average Fuel Economy L/100km
		    if (m == 20) {
				speed_actual_avg = speed_avg / speed_inc;							// 100
				fuel_disp = ((fuel_avg/fuel_inc) << 3) / speed_actual_avg;			// 930*8 / 100 = 149
				LCD_Digit(fuel_disp,4*Consolas_32x16_width,AVG_HPOS,3,1,Consolas_32x16,White);
				LCD_Digit(fuel_array_sum >> 8,11*Consolas_32x16_width,AVG_HPOS,3,1,Consolas_32x16,White);

			    m=0;
				// Fuel Range (from L/100km & total calculated fuel)
				fuel_range = ((17820000 - fuel_avg) / 343) / (fuel_array_sum >> 8);
				Hex2Dec(fuel_range);
				for(i=0;i<3;i++) {
					LCDCharDisp(hexdec[i+2],(8+i)*Consolas_32x16_width,198,Consolas_32x16,COL_YELLOW);
				}
		    }
		    m++;

			if (PIND & (1<<PIND7))
			{
				if (n == 0x1F)
				{
					fuel_array_inc = eeprom_read_byte(&ee_fuel_inc) + 1;
					eeprom_update_byte(&ee_fuel_inc, fuel_array_inc);
					fuel_array_avg = (fuel_array_avg1 / fuel_array_inc1);
					if (fuel_array_avg > 0xFF)
						fuel_array_avg = 0xFF;
					fuel_array_sum = fuel_array_sum + fuel_array_avg - eeprom_read_byte(&ee_fuel_array[fuel_array_inc - 1]);
					eeprom_update_byte(&ee_fuel_array[fuel_array_inc],fuel_array_avg);
					fuel_array_avg1 = 0;
					fuel_array_inc1 = 0;
					eeprom_update_dword(&ee_fuel_h,fuel_avg >> 32);
					eeprom_update_dword(&ee_fuel_l,fuel_avg);
					eeprom_update_dword(&ee_fueli,fuel_inc);
					eeprom_update_dword(&ee_speed_h,speed_avg >> 32);
					eeprom_update_dword(&ee_speed_l,speed_avg);
					eeprom_update_dword(&ee_speedi,speed_inc);
				}
				n++;
			}
		    // Current Speed
		    if (speed_ins_count < 0xFFFFFFFF) {
				speed_ins = speed_ins_count / speed_ins_div;
				speed_ins_count = 0;
				speed_ins_div = 0;
		    } else {
				speed_ins = 0xFFFF;
			}
			if (speed_ins < 20000) {
			    speed_ins = (speed_ins + speed_ins_temp) / 2;
			    speed_actual = speed_divisor / speed_ins;
			} else {
				speed_actual = 0;
			}
		    speed_ins_temp = speed_ins;
		    Hex2Dec(speed_actual);
		    for(i=0;i<3;i++) {
			    LCDCharDisp(hexdec[i+2],70+(i*Consolas_Numbers_width),ARROW_YPOS+14,Consolas_Numbers,COL_WHITE);
		    }
			if (PIND & (1<<PIND7)) {
				speed_avg += speed_actual;
				speed_inc++;
			}

			// Instantaneous Fuel Economy L/hr or L/100km if over 12km/h
		    //fuel_disp_sum = fuel_ins*speed_ins;
		    //fuel_disp = fuel_disp_sum >> 12;
		    if (speed_ins > 20000) {
			    if (!(LCDflag & (1<<4))) {
				    LCDPrintStr((9*Consolas_32x16_width)+5,INS_HPOS+12,"L/hr   ",SMALL_FONT,COL_WHITE);
				    Setb(LCDflag,4);
			    }
			    fuel_disp = fuel_ins / 12;
			} else if (LCDflag & (1<<4)) {
			    LCDPrintStr((9*Consolas_32x16_width)+5,INS_HPOS+12,"L/100km",SMALL_FONT,COL_WHITE);
			    Clrb(LCDflag,4);
		    } else {
				fuel_disp = (fuel_ins << 3) / speed_actual;
				fuel_array_avg1 += fuel_disp;
				fuel_array_inc1++;
		    }
			Hex2Dec(fuel_disp);
		    for(i=0;i<2;i++) {
			    LCDCharDisp(hexdec[i+2],(5+i)*Consolas_32x16_width,INS_HPOS,Consolas_32x16,COL_WHITE);
		    }
		    LCDPrintStr(7*Consolas_32x16_width,INS_HPOS,".",Consolas_32x16,COL_WHITE);
		    LCDCharDisp(hexdec[4],8*Consolas_32x16_width,INS_HPOS,Consolas_32x16,COL_WHITE);


		    // Fuel Gauge
		    if (PIND & (1<<PIND7)) {
			    if (k == 20) {
				    fuel_guage_disp = 0xFFFF / fuel_guage;
				    Hex2Dec(fuel_guage_disp);
				    for(i=0;i<3;i++) {
					    LCDCharDisp(hexdec[i+2],(2+i)*Consolas_32x16_width,198,Consolas_32x16,COL_YELLOW);
				    }
				    fuel_guage = 0;
				    k=0;
					// Fuel Range (from fuel guage & L/100km)
/*					fuel_range = (fuel_guage_disp * 512) / fuel_avg_disp;
					Hex2Dec(fuel_range);
					for(i=0;i<3;i++) {
						LCDCharDisp(hexdec[i+2],(7+i)*Consolas_32x16_width,198,Consolas_32x16,COL_YELLOW);
					}
*/			    }
				fuel_guage += ADCH;
				Setb(ADCSRA,ADSC);
			    k++;
		    }
			//_delay_ms(20);
			/*
			if (PIND & (1<<PIND4)) LCDPrintStr(0,165,"D4",SMALL_FONT,COL_YELLOW);
			if (PIND & (1<<PIND7)) LCDPrintStr(3*8,165,"D7",SMALL_FONT,COL_YELLOW);
			if (PINE & (1<<PINE7)) LCDPrintStr(6*8,165,"E7",SMALL_FONT,COL_YELLOW);
			if (PINE & (1<<PINE6)) LCDPrintStr(9*8,165,"E6",SMALL_FONT,COL_YELLOW);
			if (PINF & (1<<PINF1)) LCDPrintStr(12*8,165,"F1",SMALL_FONT,COL_YELLOW);
			if (PINF & (1<<PINF3)) LCDPrintStr(15*8,165,"F3",SMALL_FONT,COL_YELLOW);
			if (PINF & (1<<PINF5)) LCDPrintStr(18*8,165,"F5",SMALL_FONT,COL_YELLOW);
			if (PINF & (1<<PINF7)) LCDPrintStr(21*8,165,"F7",SMALL_FONT,COL_YELLOW);
			if (PIND & (1<<PIND0)) LCDPrintStr(24*8,165,"D0",SMALL_FONT,COL_YELLOW);
			if (PIND & (1<<PIND1)) LCDPrintStr(27*8,165,"D1",SMALL_FONT,COL_YELLOW);
			if (PIND & (1<<PIND2)) LCDPrintStr(30*8,165,"D2",SMALL_FONT,COL_YELLOW);
			if (PIND & (1<<PIND3)) LCDPrintStr(33*8,165,"D3",SMALL_FONT,COL_YELLOW);
			*/
	    }
    }
}

void Display_Initial(void)
{
	//LCD_Rect(0,319,0,239,COL_BLACK);

	LCDPrintStr(0,AVG_HPOS,"Avg:",Consolas_32x16,COL_WHITE);
	LCDPrintStr(0,INS_HPOS,"Ins:",Consolas_32x16,COL_WHITE);
	LCDPrintStr(10*Consolas_32x16_width,AVG_HPOS,"/",Consolas_32x16,COL_WHITE);
	LCDPrintStr((16*Consolas_32x16_width)+5,AVG_HPOS+12,"L/100km",SMALL_FONT,COL_WHITE);
	//	LCDPrintStr((11*Consolas_32x16_width)+2,INS_HPOS-31,"L/hr   ",SMALL_FONT,COL_WHITE);
	LCDPrintStr(144,ARROW_YPOS+35,"km/h",SMALL_FONT,COL_WHITE);

	LCDPrintStr(0,198,"#",Consolas_Numbers,COL_YELLOW);								// fuel guage symbol
	LCDPrintStr((6*Consolas_32x16_width),198,"!",Consolas_Numbers,COL_YELLOW);		// road/range symbol
	//LCDPrintStr((11*Consolas_32x16_width),198,"(",Consolas_32x16,COL_YELLOW);
	//LCDPrintStr((17*Consolas_32x16_width),198,")",Consolas_32x16,COL_YELLOW);
	//LCDPrintStr(0,,"L/hr   ",SMALL_FONT,COL_WHITE);

	LCDpix(ARROW_XPOS,ARROW_YPOS,Arrow_Hollow_Right_width,Arrow_Hollow_Right_height,Arrow_Hollow_Right_bits,COL_GREEN);
	LCDpix(0,ARROW_YPOS,Arrow_Hollow_Left_width,Arrow_Hollow_Left_height,Arrow_Hollow_Left_bits,COL_GREEN);

	//LCDpicture(0,0,SHIVAAN_WIDTH, SHIVAAN_HEIGHT,SHIVAAN_pixel_data);

	//LCDPrintStr(10,160,"Testing", Consolas_32x16,Green);
	//LCDPrintStr(10,120,"The", Consolas_32x16,Green);
	//LCDPrintStr(10,80,"display", Consolas_32x16,Green);
	//LCDPrintStr(10,40,"to see", Consolas_32x16,Green);
	//LCDPrintStr(10,0,"if it is working", Consolas_32x16,Green);

	//LCDPrintStr(0,200,"1234567890", Consolas_Numbers,Yellow);

	//LCDpix(0,100,Arrow_Solid_Left_width,Arrow_Solid_Left_height,Arrow_Solid_Left_bits,Green);
	//LCDpix(100,0,LowBeam_width,LowBeam_height,LowBeam_bits,Yellow);
}

ISR(TIMER0_OVF_vect)								// Timer 0 interrupt @ 32.7ms
{
	rpm_count++;
	if (rpm_count == 39)							// 42 cycles x 32.7ms = 1373ms (32.7ms = 256 timer1 cycles)
	{
		rpm_count = 0;
		if (PIND & (1<<PIND7)) {
			if (TCCR1B == 0x00)
			{
				fuel_avg += TCNT1;
				fuel_ins = TCNT1;
				TCNT1 = 0;
				} else {
				TCCR1B = 0x00;
				fuel_avg += TCNT1;
				fuel_ins = TCNT1;
				TCNT1 = 0;
				TCCR1B = ((1<<CS10)|(1<<CS12));
			}
			if (speed_ins < 20000) {
				fuel_inc++;
			}
		}
	}
	// 	if (PINF & (1<<PINF3)) {
	// 		LCD_BL_PR ^= LCD_BL_PIN;
	// 	}
}

ISR(TIMER1_OVF_vect) {
	TCCR1B = 0;
	TCNT1 = 0;
	fuel_ins = 0;
}

ISR(TIMER3_OVF_vect) {
	TCCR3B = 0;
	TCNT3 = 0;
	speed_ins_count = 0xFFFFFFFF;
}

ISR(INT6_vect)
{
	if (PINE & (1<<6))
	{
		TCCR1B = 0x00;							// stop timer 1 on rising edge
		} else {
		TCCR1B = ((1<<CS10)|(1<<CS12));			// start timer 1 on falling edge
	}
}

ISR(INT2_vect)
{
	uint16_t timer_3;
	if (TCNT3 == 0) {
		TCCR3B = (1<<CS32); //((1<<CS31)|(1<<CS30));			// Start Timer 3
		speed_ins_count = 0;
		speed_ins_div = 0;
	} else {
		TCCR3B = 0;												// Stop Timer 3
		speed_ins_div++;
		timer_3 = TCNT3;
		speed_ins_count += timer_3;
		TCNT3 = 0;
		TCCR3B = (1<<CS32); //((1<<CS31)|(1<<CS30));
	}
}

ISR(USART0_RX_vect)
{
	char ReceiveByte,i=0;
	uint16_t value = 0;
	ReceiveByte = UDR0;
	USART_Transmit(ReceiveByte);
	if (ReceiveByte < 0x39)	{
		if (i == 0) { value += ((ReceiveByte-0x30) * 10000); }
		if (i == 1) { value += ((ReceiveByte-0x30) * 1000); }
		if (i == 2) { value += ((ReceiveByte-0x30) * 100); }
		if (i == 3) { value += ((ReceiveByte-0x30) * 10); }
		if (i == 4) { value += (ReceiveByte-0x30); }
		i++;
		} else {
		switch (ReceiveByte) {
			case '~' :
			Clrb(LCD_RST_PRTC,LCD_RST_PIN);
			wdt_enable(WDTO_15MS);
			while(1==1) { ; }
			break;
			case 'r' :
			eeprom_update_dword(&ee_fuel_h,0x0000);							// Clear EEPROM
			eeprom_update_dword(&ee_fuel_l,0x0000);
			eeprom_update_dword(&ee_fueli,0x0000);
			eeprom_update_dword(&ee_speed_h,0x0000);
			eeprom_update_dword(&ee_speed_l,0x0000);
			eeprom_update_dword(&ee_speedi,0x0000);
			LCDPrintStr(0,45,"EEPROM Erased",SMALL_FONT,COL_YELLOW);
			fuel_avg = 0;
			fuel_inc = 0;
			speed_avg = 0;
			speed_inc = 0;
			break;
			case 's' :
			Hex2Dec(value);
			USART_Transmit(0x0D);
			USART_Transmit(0x73);
			USART_Transmit(0x3A);
			USART_Transmit(hexdec[0]);
			USART_Transmit(hexdec[1]);
			USART_Transmit(hexdec[2]);
			USART_Transmit(hexdec[3]);
			USART_Transmit(hexdec[4]);
			eeprom_update_word(&ee_speed_divisor,value);
			//speed_divisor = value;
			value = 0;
			i = 0;
			break;
		}
	}

}

void Hex2Dec(uint16_t value)
{
	uint16_t a,b,c,d;

	if (value > 9999) {
		hexdec[0] = value/10000;
		a = hexdec[0]*10000;
		hexdec[0] += 0x30;
		hexdec[1] = (value - a)/1000;
		b = hexdec[1]*1000;
		hexdec[1] += 0x30;
		hexdec[2] = (value - a - b)/100;
		c = hexdec[2]*100;
		hexdec[2] += 0x30;
		hexdec[3] = (value - a - b - c)/10;
		d = hexdec[3]*10;
		hexdec[3] += 0x30;
		} else if (value > 999) {
		hexdec[0] = 0x20;
		a = 0;
		hexdec[1] = value/1000;
		b = hexdec[1]*1000;
		hexdec[1] += 0x30;
		hexdec[2] = (value - b)/100;
		c = hexdec[2]*100;
		hexdec[2] += 0x30;
		hexdec[3] = (value - b - c)/10;
		d = hexdec[3]*10;
		hexdec[3] += 0x30;
		} else if (value > 99) {
		hexdec[0] = 0x20;
		a = 0;
		hexdec[1] = 0x20;
		b = 0;
		hexdec[2] = value/100;
		c = hexdec[2]*100;
		hexdec[2] += 0x30;
		hexdec[3] = (value - c)/10;
		d = hexdec[3]*10;
		hexdec[3] += 0x30;
		} else {
		hexdec[0] = 0x20;
		a=0;
		hexdec[1] = 0x20;
		b=0;
		hexdec[2] = 0x20;
		c=0;
		hexdec[3] = value/10;
		d = hexdec[3]*10;
		hexdec[3] += 0x30;
	}
	hexdec[4] = value - a - b - c - d;
	hexdec[4] += 0x30;

}

void startup_eeprom(void)
{
	if (PINF & (1<<PINF3))							// When booting in accessories position
	{
		eeprom_update_dword(&ee_fuel_h,0x0000);							// Clear EEPROM
		eeprom_update_dword(&ee_fuel_l,0x0000);
		eeprom_update_dword(&ee_fueli,0x0000);
		eeprom_update_dword(&ee_speed_h,0x0000);
		eeprom_update_dword(&ee_speed_l,0x0000);
		eeprom_update_dword(&ee_speedi,0x0000);
		eeprom_update_byte(&ee_fuel_inc,0x0000);
//		for (i=0;i<255;i++)
//			eeprom_update_byte(&ee_fuel_array[i],100);
//		LCDPrintStr(0,179,"EEPROM Erased",SMALL_FONT,COL_YELLOW);
	}

	fuel_avg = eeprom_read_dword(&ee_fuel_h);
	fuel_avg <<= 32;
	fuel_avg |= eeprom_read_dword(&ee_fuel_l);
	fuel_inc = eeprom_read_dword(&ee_fueli);
	fuel_array_inc = eeprom_read_byte(&ee_fuel_inc);
	speed_avg = eeprom_read_dword(&ee_speed_h);
	speed_avg <<= 32;
	speed_avg |= eeprom_read_dword(&ee_speed_l);
	speed_inc = eeprom_read_dword(&ee_speedi);
	//speed_divisor = eeprom_read_word(&ee_speed_divisor);
}

void rav_pin_functions(void)
{
	if (PINF & (1<<PINF5))	{			// Left Blinker On
		if (!(LCDflag & (1<<0))) {
			LCDpix(0,ARROW_YPOS,Arrow_Solid_Left_width,Arrow_Solid_Left_height,Arrow_Solid_Left_bits,COL_GREEN);
		}
		Setb(LCDflag,0);
		} else {						// Left Blinker Off
		if (LCDflag & (1<<0)) {
			LCDpix(0,ARROW_YPOS,Arrow_Hollow_Left_width,Arrow_Hollow_Left_height,Arrow_Hollow_Left_bits,COL_GREEN);
		}
		Clrb(LCDflag,0);
	}
	if (PINF & (1<<PINF7))	{			// Right Blinker On
		if (!(LCDflag & (1<<1))) {
			LCDpix(ARROW_XPOS,ARROW_YPOS,Arrow_Solid_Right_width,Arrow_Solid_Right_height,Arrow_Solid_Right_bits,COL_GREEN);
		}
		Setb(LCDflag,1);
		} else {						// Right Blinker Off
		if (LCDflag & (1<<1)) {
			LCDpix(ARROW_XPOS,ARROW_YPOS,Arrow_Hollow_Right_width,Arrow_Hollow_Right_height,Arrow_Hollow_Right_bits,COL_GREEN);
		}
		Clrb(LCDflag,1);
	}

	if (!(PIND & (1<<PIND4)) && (PINF & (1<<PINF3)))
	{									// Headlights High Beam
		if (!(LCDflag & (1<<2))) {
			LCD_Rect(HL_X,HL_Y,LowBeam_width,LowBeam_height,COL_BLACK);
			LCDpix(HL_X,HL_Y+12,HighBeam_width,HighBeam_height,HighBeam_bits,COL_WHITE);
			Setb(LCDflag,2);
			Clrb(LCDflag,3);
		}
	} else if ((PIND & (1<<PIND4)) && (PINF & (1<<PINF3))) {		// Headlights Low Beam
		if (!(LCDflag & (1<<3))) {
			LCDpix(HL_X,HL_Y,LowBeam_width,LowBeam_height,LowBeam_bits,COL_BLUE);
			Setb(LCDflag,3);
			Clrb(LCDflag,2);
		}
	} else {							// Headlights Off
		if (LCDflag & (1<<3)) {
			LCD_Rect(HL_X,HL_Y,LowBeam_width,LowBeam_height,COL_BLACK);
			Clrb(LCDflag,3);
			} else if (LCDflag & (1<<2)) {
			LCD_Rect(HL_X,HL_Y,LowBeam_width,LowBeam_height,COL_BLACK);
			Clrb(LCDflag,2);
		}
	}
}

void save_eeprom(void)
{
	uint8_t i;
	if (!(PIND & (1<<PIND7))) {
		if (LCDflag & (1<<7)) {
			cli();
			for (i=0;i<50;i++) _delay_ms(10);
			eeprom_update_dword(&ee_fuel_h,fuel_avg >> 32);
			eeprom_update_dword(&ee_fuel_l,fuel_avg);
			eeprom_update_dword(&ee_fueli,fuel_inc);
			eeprom_update_dword(&ee_speed_h,speed_avg >> 32);
			eeprom_update_dword(&ee_speed_l,speed_avg);
			eeprom_update_dword(&ee_speedi,speed_inc);
			LCDPrintStr(0,179,"Saved EEPROM",SMALL_FONT,COL_YELLOW);
			sei();
		}
		Clrb(LCDflag,7);
		} else {
		Setb(LCDflag,7);
		LCD_Rect(0,179,96,16,COL_BLACK);
	}
}

void main_init(void)
{
	// Setup Timer 0 as 32.768ms Timer
	TCCR0 = ((1<<CS00)|(1<<CS01)|(1<<CS02));		// clk/1024 (32ms)
	Setb(TIMSK,TOIE0);								// Enable T0 Overflow Interrupt
	Setb(TIMSK,TOIE1);								// Enable T1 Overflow Interrupt
	Setb(ETIMSK,TOIE3);								// Enable T3 Overflow Interrupt
	Setb(EICRA,ISC21);								// set INT2 Interrupt on Falling Edge
	Setb(EIMSK,INT2);								// Enable INT2 Pin
	Setb(EICRB,ISC60);								// set INT6 Interrupt on change
	Setb(EIMSK,INT6);								// Enable INT6 Pin
	// Setup AD
	ADCSRA |= ((1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2)|(1<<ADEN));
	Setb(ADCSRA,ADSC);
	Setb(ADMUX,MUX0);
	Setb(ADMUX,ADLAR);
}
