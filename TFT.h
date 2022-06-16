#include "spi.h"
#include "global.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "TFT_font.h"


#include <stdint.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#define TFTWIDTH   320
#define TFTHEIGHT  480
#define TFTLCD_DELAY 250
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

#include <avr/io.h>
#include "pin_magic.h"
#include "registers.h"
#include "Adafruit_TFTLCD.h"
#include "myGFX.h"



/////
#ifndef TFT.h
#define TFT.h

#define Red				0xf800				// 빨
#define Orange			0xfb00					// 주
#define Yellow			0x07ff					// 노
#define Green			0x07e0					// 초
#define Blue			0x001f			// 파
#define Dark_blue		0x000b					// 남
#define Violet			0x980e					// 보
#define White		 0xffff					// 흰
#define Black			  0x0000					// 검
#define	Cyan			  0xffe0 
#define	Magenta			0xf81f
#define khaki       0x7f13// 0xf731
#define olive       0x8400
#define pink        0xfb56
#define purple      0x8010
#define silver      0xc618

#define Transparent	0x0821

unsigned char ScreenMode = 'P';			// screen mode(P=portrait, L=landscape)
unsigned char LineLimit = 40;			// character length of line (30 or 40)
unsigned char KoreanBuffer[32] = {0};		// 32 byte Korean font buffer
unsigned char xcharacter, ycharacter;		// xcharacter(0-29), ycharacter(0-39) for portrait
						// xcharacter(0-39), ycharacter(0-29) for landscape
unsigned int  foreground, background;		// foreground and background color
unsigned char cursor_flag = 0;			// 0 = cursor off, 1 = cursor on
unsigned char xcursor=0, ycursor=0;			// cursor position
unsigned int  cursor=0;				// cursor color
unsigned char outline_flag = 0;			// 0 = outline off, 1 = outline on
unsigned int  outline=0;				// outline color

//#define tft_sel     PORTB4  ///tft_lcd칩 셀렉트
//#define tft_dc      PORTE5

typedef unsigned char uint8_t;
typedef unsigned int   uint16_t;
typedef unsigned char	U08;			/* data type definition */
typedef   signed char	S08;
typedef unsigned int	U16;
typedef   signed int	S16;
typedef unsigned long	U32;
typedef   signed long	S32;

static const uint8_t HX8357D_regValues[] = {
	SWRESET,
	0,
	SETC,
	3,
	0xFF,
	0x83,
	0x57,
	TFTLCD_DELAY,
	250,
	SETRGB,
	4,
	0x80,
	0x00,
	0x06,
	0x06,
	SETCOM,
	1,
	0x25, // -1.52V
	SETOSC,
	1,
	0x68, // Normal mode 70Hz, Idle mode 55 Hz
	SETPANEL,
	1,
	0x05, // BGR, Gate direction swapped
	SETPWR1,
	6,
	0x00,
	0x15,
	0x1C,
	0x1C,
	0x83,
	0xAA,
	SETSTBA,
	6,
	0x50,
	0x50,
	0x01,
	0x3C,
	0x1E,
	0x08,
	// MEME GAMMA HERE
	SETCYC,
	7,
	0x02,
	0x40,
	0x00,
	0x2A,
	0x2A,
	0x0D,
	0x78,
	COLMOD,
	1,
	0x55,
	MADCTL,
	1,
	0x4c,//0b11100000, // ���⸦ �ٲ�� ��  1110, 0011, xx1x�� �ٲٱ� 
	TEON,
	1,
	0x00,
	TEARLINE,
	2,
	0x00,
	0x02,
	SLPOUT,
	0,
	TFTLCD_DELAY,
	150,
	DISPON,
	0,
	TFTLCD_DELAY,
	50,
};
////////////////////////////////////////////
///////////////////////////////////////8bit

long size = (long)TFTWIDTH * (long)TFTHEIGHT;

void reset(void);
void begin(void);
void setAddrWindow (int x1, int y1, int x2, int y2);
void flood (uint16_t color, uint32_t len);
void drawPixel (int16_t x , int16_t y, uint16_t color);
void writeRegister32 (uint8_t r, uint32_t d);
void drawstring(int16_t x, int16_t y, char* str,
uint16_t text_color, uint16_t background_color, uint8_t size_x, uint8_t size_y);
void drawSLine(int16_t x, int16_t y,int16_t length,   uint16_t color);
	// x ��ǥ ,y ��ǥ , ����, ���� , ����
void drawLLine(int16_t x, int16_t y,int16_t length,  uint16_t color);

void drawChar(int16_t x, int16_t y, unsigned char c,
	uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y); 
void drawRect(int x1,int y1,int x2,int y2,uint16_t color);
void Drawtextbox(int x1,int y1,int x2,int y2,uint16_t color,uint16_t back_color,char* text,char width,char height);

void frame(void);

void reset(void) {

	CS_IDLE;
	//  CD_DATA;
	WR_IDLE;
	RD_IDLE;
	// Data transfer sync
	CS_ACTIVE;
	CD_COMMAND;
	write8(0x00);
	for (uint8_t i = 0; i < 3; i++)
		WR_STROBE; // Three extra 0x00s
	CS_IDLE;
}

void begin(void)
{
	
	uint8_t i = 0;



	_delay_us(200);  // delay ���ĵα�

	// HX8357D
	CS_ACTIVE;
	while (i < sizeof(HX8357D_regValues))
	{
		uint8_t r = pgm_read_byte(&HX8357D_regValues[i++]);
		uint8_t len = pgm_read_byte(&HX8357D_regValues[i++]);
		if (r == TFTLCD_DELAY)
		{
			_delay_us(len);
		}
		else
		{
		

			CS_ACTIVE;
			CD_COMMAND;
			write8(r);
			CD_DATA;
			for (uint8_t d = 0; d < len; d++)
			{
				uint8_t x = pgm_read_byte(&HX8357D_regValues[i++]);
				write8(x);
			}
			CS_IDLE;
		}
	}
	return;

	
}

void setAddrWindow(int x1, int y1, int x2, int y2) {
	CS_ACTIVE;
	uint32_t t;

	t = x1;
	t <<= 16;
	t |= x2;
	writeRegister32(CASET, t);
	t = y1;
	t <<= 16;
	t |= y2;
	writeRegister32(PASET, t);
	   CS_IDLE;
}
void drawstring(int16_t x, int16_t y, char* str,
uint16_t text_color, uint16_t background_color, uint8_t size_x, uint8_t size_y)
{
	char ch1;
	char width;
	if(size_x==1) width=5;
	else if(size_x==2) width=10;
	int16_t xpos=0,ypos=0,xchar=0,ychar=0;
	
	int count=0,cnty=0;
	xcharacter=x;
	ycharacter=y;
	while(*str)
	{
		
		ch1 = *str;
		str++;
		xpos=xcharacter+count*width;
		ypos=ycharacter+16*cnty;
		if(xpos>340){
			cnty++;
			xcharacter=0;
			count=0;
			//ychar=16;
		}
		drawChar(xpos,ypos,ch1,text_color,background_color,size_x,size_y);		// English ASCII character
		count++;
		
	}
}
void drawPixel(int16_t x, int16_t y, uint16_t color) {

	// Clip
	if ((x < 0) || (y < 0) || (x >= TFTWIDTH) || (y >= TFTHEIGHT))
	return;

	CS_ACTIVE;
	//setAddrWindow(x, y, TFTWIDTH - 1, TFTHEIGHT - 1);
	setAddrWindow(x, y, x+1, y+1);
	CS_ACTIVE;
	CD_COMMAND;
	write8(0x2C);
	CD_DATA;
	write8(color >> 8);
	write8(color);
	CS_IDLE;

}

void flood(uint16_t color, uint32_t len){
	uint16_t blocks;
	uint8_t i, hi = color >> 8, lo = color;
	

	CS_ACTIVE;
	CD_COMMAND;
	
	write8(RAMWR);
	
	// Write first pixel normally, decrement counter by 1
	CD_DATA;
	write8(hi);
	write8(lo);
	len--;
	
	blocks = (uint16_t)(len / 64); // 64 pixels/block
	if (hi == lo) {
		// High and low bytes are identical.  Leave prior data
		// on the port(s) and just toggle the write strobe.
		while (blocks--) {
			i = 16; // 64 pixels/block / 4 pixels/pass
			do {
				WR_STROBE;
				WR_STROBE;
				WR_STROBE;
				WR_STROBE; // 2 bytes/pixel
				WR_STROBE;
				WR_STROBE;
				WR_STROBE;
				WR_STROBE; // x 4 pixels
			} while (--i);
		}
		// Fill any remaining pixels (1 to 64)
		for (i = (uint8_t)len & 63; i--;) {
			WR_STROBE;
			WR_STROBE;
		}
		} else {
		while (blocks--) {
			i = 16; // 64 pixels/block / 4 pixels/pass
			do {
				write8(hi);
				write8(lo);
				write8(hi);
				write8(lo);
				write8(hi);
				write8(lo);
				write8(hi);
				write8(lo);
			} while (--i);
		}
		for (i = (uint8_t)len & 63; i--;) {
			write8(hi);
			write8(lo);
		}
	}
	CS_IDLE;
}

// ª�� �� �� �׸���
void drawSLine(int16_t x, int16_t y, int16_t length, uint16_t color)
{
	int i ;
	int x2 = x+ length;

	setAddrWindow(x,y,x2,y);
	CS_ACTIVE;
	CD_COMMAND;
	write8(RAMWR);
	CD_DATA;
	for (i=0; i<  (length-1) ; i++)
	{
		write8(color>>8);
		write8(color);
	}

	
}
// �� �� ���׸���
void drawLLine(int16_t x, int16_t y, int16_t length,  uint16_t color)
{
	int i ;
	int y2 = y+ length;
	setAddrWindow(x,y,x,y2);
	CS_ACTIVE;
	CD_COMMAND;
	write8(RAMWR);
	CD_DATA;
	for (i=0; i<  (length-1) ; i++)
	{
		write8(color>>8);
		write8(color);
	}
	
	
}
void drawRect(int x1,int y1,int x2,int y2,uint16_t color)
{
	setAddrWindow(x1,y1,x2,y2);
	CS_ACTIVE;
	CD_COMMAND;
	write8(RAMWR);
	CD_DATA;
	for (int i=0; i< x2-x1 ; i++)
	{
		for(int j=0;j<y2-y1;j++)
		{
			write8(color>>8);
			write8(color);
		}

	}
	
	
}

void frame(void)
{
	drawSLine(0, 0,				TFTWIDTH-1,	 0xFFFF);
	drawSLine(0, TFTHEIGHT-1,	TFTWIDTH-1,  0xFFFF);
	drawLLine(0, 0,				TFTHEIGHT-1, 0xFFFF);
	drawLLine(TFTWIDTH-1, 0,	TFTHEIGHT-1, 0xFFFF);
}

// write () ����  �������� ���� ��� ��Ƶα�    (�̺κ��� ���� ):: )) wirte
#ifndef setWriteDir
void setWriteDir(void) { setWriteDirInline(); }
#endif

#ifndef setReadDir
void setReadDir(void) { setReadDirInline(); }
#endif

#ifndef writeRegister8
void writeRegister8(uint8_t a, uint8_t d) {
	writeRegister8inline(a, d);
}
#endif

#ifndef writeRegister16
void writeRegister16(uint16_t a, uint16_t d) {
	writeRegister16inline(a, d);
}
#endif

#ifndef writeRegisterPair
void writeRegisterPair(uint8_t aH, uint8_t aL, uint16_t d) {
	writeRegisterPairInline(aH, aL, d);
}
#endif

void writeRegister24(uint8_t r, uint32_t d) {
	CS_ACTIVE;
	CD_COMMAND;
	write8(r);
	CD_DATA;
	_delay_us(10);
	write8(d >> 16);
	_delay_us(10);
	write8(d >> 8);
	_delay_us(10);
	write8(d);
	CS_IDLE;
}
#ifndef writeRegisterPair
void writeRegister32 (uint8_t r, uint32_t d) {
	CS_ACTIVE;
	CD_COMMAND;
	write8(r);
	CD_DATA;
	_delay_us(10);
	write8(d >> 24);
	_delay_us(10);
	write8(d >> 16);
	_delay_us(10);
	write8(d >> 8);
	_delay_us(10);
	write8(d);
	CS_IDLE;
}
#endif

void drawChar(int16_t x, int16_t y, unsigned char c,
	uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y)
{	
	for (int8_t i = 0; i < 5; i++)  // Char bitmap = 5 columns
	{
		uint8_t line = pgm_read_byte(&font[c * 5 + i]);
		for (int8_t j = 0; j < 8; j++, line >>= 1) 
		{
			if (line & 1) 
			{
				if (size_x == 1 && size_y == 1)
					drawPixel(x + i, y + j, color);
				else
					writeFillRect(x + i * size_x, y + j * size_y, size_x, size_y, color);
			} 
			else if (bg != color)
			{
				if (size_x == 1 && size_y == 1)
					drawPixel(x + i, y + j, bg);
				else
					writeFillRect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
			}
		}
	}
	if (bg != color)  // If opaque, draw vertical line for last column
	{
		if (size_x == 1 && size_y == 1)
			writeFastVLine(x + 5, y, 8, bg);
		else
			writeFillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
	}
}  



///////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
///////////////////////////////////////

void TFT_init();
void TFT_send16(uint8_t IR,uint16_t data);
void TFT_send8(uint8_t IR,uint8_t data);
void TFT_command(uint8_t IR);
void TFT_data8(uint8_t data);
void TFT_data(uint16_t data);
//void TFT_clear();
void TFT_clear_screen(void);			// TFT-LCD clear screen with black color
//void TFT_color_screen(U16 color);		// TFT-LCD full screen color
//void TFT_GRAM_address(U16 xPos, U16 yPos);	// set GRAM address of TFT-LCD

//void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w,uint16_t h);
void TFT_color_screen(U16 color);
void TFT_screen_mode(U08 mode);			// set screen direction mode



void TFT_xy(U08 xChar, U08 yChar);		// set character position
void TFT_color(U16 colorfore, U16 colorback);	// set foreground and background color
void TFT_string_size(U08 xChar, U08 yChar, U16 colorfore, U16 colorback, char *str,U08 width, U08 height );
void TFT_string(U08 xChar, U08 yChar, U16 colorfore, U16 colorback, char *str);	// write TFT-LCD string
void TFT_English(U08 code);			// write a English ASCII character
unsigned int KS_code_conversion(U16 KSSM);	// convert KSSM(�ϼ���) to KS(������)
void TFT_Korean(U16 code);			// write a Korean character
void TFT_cursor(U16 cursor_color);		// set cursor and color
void TFT_outline(U16 outline_color);		// set outline and color
unsigned int Unicode_to_KS(U16 unicode);
void TFT_unsigned_decimal(U32 number, U08 zerofill, U08 digit); // display unsigned decimal number
void TFT_signed_decimal(S32 number, U08 zerofill, U08 digit);	// display signed decimal number
void TFT_hexadecimal(U32 number, U08 digit);	// display hexadecimal number
void TFT_0x_hexadecimal(U32 number, U08 digit);	// display hexadecimal number with 0x
void TFT_unsigned_float(float number, U08 integral, U08 fractional); // display unsigned floating-point number
void TFT_signed_float(float number, U08 integral, U08 fractional);   // display signed floating-point number
//
void Line(S16 x1,S16 y1, S16 x2,S16 y2, U16 color);	// draw a straight line
void Rectangle(S16 x1,S16 y1, S16 x2,S16 y2, U16 color);// draw a rectangle
void Block(S16 x1,S16 y1, S16 x2,S16 y2, U16 color, U16 fill); // draw a rectangle with filled color
void Circle(S16 x1,S16 y1, S16 r, U16 color);	// draw a circle
void Sine(S16 peak, U08 mode, U16 color);	// draw a sine curve

//********************************************************************************
// 함수 선언
//***********************************************************************************
//TFT_SPI부분
void TFT_send16(uint8_t IR,uint16_t data)
{
    cbi(PORTE,5);
    cbi(PORTB,4);
    spi_transfer_byte(IR);
    sbi(PORTE,5);
    spi_transfer_word(data);
    sbi(PORTB,4);
}
void TFT_send8(uint8_t IR,uint8_t data)
{
    cbi(PORTE,5);
    cbi(PORTB,4);
    spi_transfer_byte(IR);
    sbi(PORTE,5);
    spi_transfer_byte(data);
    sbi(PORTB,4);
}

void TFT_command(uint8_t IR)
{ 
    cbi(PORTE,5);  
    cbi(PORTB,4);
    spi_transfer_byte(IR);
    sbi(PORTB,4);
    
}
void TFT_data8(uint8_t data)
{
    sbi(PORTE,5);
    cbi(PORTB,4);
    spi_transfer_byte(data); 
    sbi(PORTB,4);
}
void TFT_data(uint16_t data)
{
    sbi(PORTE,5);
    cbi(PORTB,4);
    
    spi_transfer_word(data);
    sbi(PORTB,4);    
}


//*****************------TFT initial-------------------------------
void TFT_init(void)			/* initialize TFT-LCD with HX8347 */
{
  reset();
  begin();
 /*
  TFT_send8(HX8357_SWRESET,0x01);		// window setting
   _delay_ms(10);
  TFT_send16(HX8357D_SETC,0xFF83);
  TFT_data8(0x57);TFT_data8(0xFF);
  
  _delay_ms(300);

  TFT_send16(HX8357_SETRGB,0x8000);
  TFT_data(0x0606);

  TFT_send8(HX8357D_SETCOM,0x25);
  TFT_send8(HX8357_SETOSC,0x68);
  TFT_send8( HX8357_SETPANEL,0x05);
  TFT_send16(HX8357_SETPWR1,0x0015);
  TFT_data(0x1C1C);TFT_data(0x83AA);
  TFT_send16(HX8357D_SETSTBA,0x5050);
  TFT_data(0x013C);TFT_data(0x1E08);
  TFT_send16(HX8357D_SETCYC,0x0240);
  TFT_data(0x002A);TFT_data(0x2A0D);TFT_data(0x78);
  TFT_command(HX8357D_SETGAMMA);      //setting gamma
  TFT_data(0x020A);TFT_data(0x111d);
  TFT_data(0x2335);TFT_data(0x414b);
  TFT_data(0x4b42);TFT_data(0x3A27);
  TFT_data(0x1b08);TFT_data(0x0903);
  TFT_data(0x020A);TFT_data(0x111d);
  TFT_data(0x2335);TFT_data(0x414b);
  TFT_data(0x4b42);TFT_data(0x3A27);
  TFT_data(0x1b08);TFT_data(0x0903);
  TFT_data(0x0001);  

  TFT_send8(HX8357_COLMOD,0x55);
  TFT_send8(HX8357_MADCTL,0x8c);
  TFT_send8(HX8357_TEON,0x00);
  TFT_send16(HX8357_TEARLINE,0x0002);
  TFT_send8(HX8357_SLPOUT,0x11);
  TFT_send8(HX8357_DISPON,0x29);
  */
}
//*****************************************************************************
/* ---------------------------------------------------------------------------- */
/*	 출력제어 함수					*/
/* ---------------------------------------------------------------------------- */
/*
void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w,uint16_t h) 
{
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
  TFT_command(HX8357_CASET); // Column address set
  TFT_data(x1);
  TFT_data(x2);
  TFT_command(HX8357_PASET); // Row address set
  TFT_data(y1);
  TFT_data(y2);
  TFT_command(HX8357_RAMWR); // Write to RAM

}*/
void TFT_GRAM_address(U16 xPos, U16 yPos)	/* set GRAM address of TFT-LCD */
{
  //if((xPos > 239) || (yPos > 319)) return;

  TFT_send16(HX8357_CASET,0x0000);		// xPos = 0~239
  TFT_data(xPos);
  TFT_send16(HX8357_PASET,0x0000);	// yPos = 0~319
  TFT_data(yPos);
  TFT_command(HX8357_RAMWR);
}

void TFT_clear_screen(void)			/* TFT-LCD clear screen with black color */
{
  TFT_color_screen(Black);
}

void TFT_color_screen(U16 color)		/* TFT-LCD full screen color */
{
	CS_ACTIVE;
	setAddrWindow(0,0, TFTWIDTH - 1, TFTHEIGHT - 1);
	//setAddrWindow(x, y, x+2, y+2);
	CS_ACTIVE;
	CD_COMMAND;
	write8(0x2C);
	CD_DATA;
	for(int j=0;j<480;j++)
	{
		for(int i=0;i<320;i++)
		{
			write8(color >> 8);
			write8(color);
		}
	}
	
	
	
	CS_IDLE;
  
}

void TFT_screen_mode(uint8_t mode)
{

}

void TFT_pixel(U16 xPos, U16 yPos, U16 color)	/* write a pixel */
{
  drawPixel (xPos , yPos, color);
/*
  TFT_command(HX8357_CASET); // Column address set
    TFT_data(xPos);
    TFT_data(xPos+1);
    TFT_command(HX8357_PASET); // Row address set
    TFT_data(yPos);
    TFT_data(yPos+1);
    TFT_command(HX8357_RAMWR); // Write to RAM
    for(int i=0;i<=4;i++)
        TFT_data(color);
*/


}
void TFT_xy(U08 xChar, U08 yChar)		/* set character position (x,y) */
{
  xcharacter = xChar;
  ycharacter = yChar;
}

void TFT_color(U16 colorfore, U16 colorback)	/* set foreground and background color */
{
  foreground = colorfore;
  background = colorback;
}


void TFT_string_size(U08 xChar, U08 yChar, U16 colorfore, U16 colorback, char  *str,U08 width, U08 height )	/* write TFT-LCD string */
{
 drawstring(xChar,yChar,str,colorfore,colorback,width,height);
}
void TFT_string(U08 xChar, U08 yChar, U16 colorfore, U16 colorback, char *str)	/* write TFT-LCD string */
{
	drawstring(xChar,yChar,str,colorfore,colorback,1,1);
}

void TFT_English(U08 code)			/* write a English ASCII character */
{
  unsigned char data, x, y;
  unsigned int pixel[8][16];
  unsigned int dot0, dot1, dot2, dot3, dot4;

  for(x = 0; x < 8; x++)			// read English ASCII font
    { data = pgm_read_byte(&E_font[code][x]);
      for(y = 0; y < 8; y++)
        { if(data & 0x01) pixel[x][y] = foreground;
	  else            pixel[x][y] = background;
	  data = data >> 1;
	}
    }

  for(x = 0; x < 8; x++)
    { data = pgm_read_byte(&E_font[code][x+8]);
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y+8] = foreground;
	  else            pixel[x][y+8] = background;
	  data = data >> 1;
	}
    }

  if(outline_flag == 1)				// display outline
    for(x = 0; x < 8; x++)
      { dot0 = pgm_read_byte(&E_font[code][x]) + pgm_read_byte(&E_font[code][x+8])*256;
        dot1 = dot0 >> 1;								 // up side
        dot2 = dot0;									 // down side
        dot3 = pgm_read_byte(&E_font[code][x+1]) + pgm_read_byte(&E_font[code][x+9])*256;// left side
        dot4 = pgm_read_byte(&E_font[code][x-1]) + pgm_read_byte(&E_font[code][x+7])*256;// right side

        for(y = 0; y < 15; y++)
	  { if(!(dot0 & 0x0001))
	      { if(dot1 & 0x0001) pixel[x][y] = outline;
	        if(dot2 & 0x0001) pixel[x][y] = outline;
	        if((dot3 & 0x0001) && (x < 7 )) pixel[x][y] = outline;
	        if((dot4 & 0x0001) && (x > 0 )) pixel[x][y] = outline;
	      }

            dot1 >>= 1;
            dot2 = dot0;
	    dot0 >>= 1;
	    dot3 >>= 1;
	    dot4 >>= 1;
          }
      }

  if((cursor_flag == 1) && (xcharacter == xcursor) &&(ycharacter == ycursor))
    { for(x = 0; x < 8; x++)			// display cursor
        { pixel[x][14] = cursor;
          pixel[x][15] = cursor;
        }
    }

  if(ScreenMode == 'P')
    { for(y = 0; y < 16; y++)			// write in portrait mode
        for(x = 0; x < 8; x++)
          TFT_pixel(xcharacter*8 + x, ycharacter*8 + y, pixel[x][y]);
    }
  else
    { for(y = 0; y < 16; y++)			// write in landscape mode
        for(x = 0; x < 8; x++)
          TFT_pixel((29-ycharacter)*8 + 7 - y, xcharacter*8 + x, pixel[x][y]);
    }

  xcharacter += 1;
  if(xcharacter >= LineLimit)			// end of line ?
    { xcharacter = 0;
      ycharacter += 2;
    }
}

unsigned int KS_code_conversion(U16 KSSM)	/* convert KSSM(�ϼ���) to KS(������) */
{
  unsigned char HB, LB;
  unsigned int index, KS;

  HB = KSSM >> 8;
  LB = KSSM & 0x00FF;

  if(KSSM >= 0xB0A1 && KSSM <= 0xC8FE)
    { index = (HB - 0xB0)*94 + LB - 0xA1;
      KS  =  pgm_read_byte(&KS_Table[index][0]) * 256;
      KS |=  pgm_read_byte(&KS_Table[index][1]);

      return KS;
    }
  else
    return -1;
}

void TFT_Korean(U16 code)			/* write a Korean character */
{
  unsigned char cho_5bit, joong_5bit, jong_5bit;
  unsigned char cho_bul, joong_bul, jong_bul = 0, i, jong_flag;
  unsigned int ch;

  cho_5bit   = pgm_read_byte(&table_cho[(code >> 10) & 0x001F]);  // get 5bit(14-10) of chosung
  joong_5bit = pgm_read_byte(&table_joong[(code >> 5) & 0x001F]); // get 5bit(09-05) of joongsung
  jong_5bit  = pgm_read_byte(&table_jong[code & 0x001F]);	  // get 5bit(04-00) of jongsung

  if(jong_5bit == 0)				// if jongsung not exist
    { jong_flag = 0;
      cho_bul = pgm_read_byte(&bul_cho1[joong_5bit]);
      if((cho_5bit == 1) || (cho_5bit == 16))
   	joong_bul = 0;
      else
	joong_bul = 1;
    }
  else						// if jongsung exist
    { jong_flag = 1;
      cho_bul = pgm_read_byte(&bul_cho2[joong_5bit]);
      if((cho_5bit == 1) || (cho_5bit == 16))
	joong_bul = 2;
      else
	joong_bul = 3;
      jong_bul = pgm_read_byte(&bul_jong[joong_5bit]);
    }

  ch = cho_bul*20 + cho_5bit;			// get chosung font 
  for(i = 0; i < 32; i++)
    KoreanBuffer[i] = pgm_read_byte(&K_font[ch][i]);
  ch = 8*20 + joong_bul*22 + joong_5bit;	// OR joongsung font
  for(i = 0; i < 32; i++)
    KoreanBuffer[i] |= pgm_read_byte(&K_font[ch][i]);
  if(jong_flag)					// OR jongsung font
    { ch = 8*20 + 4*22 + jong_bul*28 + jong_5bit;
      for(i = 0; i < 32; i++)
	KoreanBuffer[i] |= pgm_read_byte(&K_font[ch][i]);
    }

  unsigned char data, x, y;
  unsigned int pixel[16][16];
  unsigned int dot0, dot1, dot2, dot3, dot4;

  for(x = 0; x < 16; x++)			// read Korean font
    { data = KoreanBuffer[x];
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y] = foreground;
	  else            pixel[x][y] = background;
	  data = data >> 1;
	}
    }

  for(x = 0; x < 16; x++)
    { data = KoreanBuffer[x+16];
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y+8] = foreground;
	  else            pixel[x][y+8] = background;
	  data = data >> 1;
	}
    }

  if(outline_flag == 1)				// display outline
    for(x = 0; x < 16; x++)
      { dot0 = KoreanBuffer[x] + KoreanBuffer[x+16]*256;
        dot1 = dot0 >> 1;				   // up side
        dot2 = dot0;					   // down side
        dot3 = KoreanBuffer[x+1] + KoreanBuffer[x+17]*256; // left side
        dot4 = KoreanBuffer[x-1] + KoreanBuffer[x+15]*256; // right side

        for(y = 0; y < 16; y++)
	  { if(!(dot0 & 0x0001))
	      { if(dot1 & 0x0001) pixel[x][y] = outline;
	        if(dot2 & 0x0001) pixel[x][y] = outline;
 	        if((dot3 & 0x0001) && (x < 15 )) pixel[x][y] = outline;
	        if((dot4 & 0x0001) && (x > 0 ))  pixel[x][y] = outline;
	      }

	    dot1 >>= 1;
	    dot2 = dot0;
	    dot0 >>= 1;
	    dot3 >>= 1;
	    dot4 >>= 1;
          }
      }

  if((cursor_flag == 1) && (xcharacter == xcursor) &&(ycharacter == ycursor))
    { for(x = 0; x < 16; x++)			// display cursor
        { pixel[x][14] = cursor;
          pixel[x][15] = cursor;
        }
    }

  if(xcharacter >= (LineLimit-1))		// end of line ?
    { xcharacter = 0;
      ycharacter += 2;
    }

  if(ScreenMode == 'P')				// write in portrait mode
    { for(y = 0; y < 16; y++)
        for(x = 0; x < 16; x++)
          TFT_pixel(xcharacter*8 + x, ycharacter*8 + y, pixel[x][y]);
    }
  else						// write in landscape mode
    { for(y = 0; y < 16; y++)
        for(x = 0; x < 16; x++)
          TFT_pixel((29-ycharacter)*8 + 7 - y, xcharacter*8 + x, pixel[x][y]);
    }

  xcharacter += 2;
  if(xcharacter >= LineLimit)			// end of line ?
    { xcharacter = 0;
      ycharacter += 2;
    }
}

void TFT_cursor(U16 cursor_color)		/* set cursor and color */
{
  if(cursor_color == Transparent)		// disable cursor
    cursor_flag = 0;
  else						// enable cursor
    { cursor_flag = 1;
      cursor = cursor_color;
    }
}
void TFT_outline(U16 outline_color)		/* set outline and color */
{
  if(outline_color == Transparent)		// disable outline
    outline_flag = 0;
  else						// enable outline
    { outline_flag = 1;
      outline = outline_color;
    }
}

unsigned int Unicode_to_KS(U16 unicode)		// convert Unicode(�����ڵ�) to KS(������) 
{
  unsigned char cho = 0, joong = 0, jong = 0;
  unsigned int value;

  value = unicode - 0xAC00;                	// �����ڵ忡�� '��'�� �ش��ϴ� ���� ����.

  jong  = value % 28;				// �����ڵ带 �ʼ�, �߼�, �������� �и�
  joong = ((value - jong) / 28 ) % 21;
  cho   = ((value - jong) / 28 ) / 21;

  cho += 2;                           	 	// �ʼ� + ������

  if(joong < 5)       joong += 3;		// �߼� + ������
  else if(joong < 11) joong += 5;
  else if(joong < 17) joong += 7;
  else                joong += 9;

  if(jong < 17) jong++;				// ���� + ������
  else          jong += 2;

  return 0x8000 | (cho << 10) | ( joong<<5) | jong; // ������ �ڵ�
}

void TFT_unsigned_decimal(U32 number, U08 zerofill, U08 digit) /* display unsigned decimal number */
{
  unsigned char zero_flag, character,count=0;
  unsigned long div;

  if((digit == 0) || (digit > 9)) return;

  div = 1;
  while(--digit) div *= 10;

  zero_flag = zerofill;
  while(div > 0)				// display number
    { 
		count++;
		xcharacter+=5*count;
		character = number / div;
		if((character == 0) && (zero_flag == 0) && (div != 1))
		drawChar(xcharacter,ycharacter+16,character+' ',Cyan,Black,1,1); // TFT_English(character + ' ');
		else
		{ zero_flag = 1;
			drawChar(xcharacter,ycharacter+16,character+'0',Cyan,Black,1,1);//TFT_English(character + '0');
		}
		number %= div;
		div /= 10;
    }
}

void TFT_signed_decimal(S32 number, U08 zerofill, U08 digit)	/* display signed decimal number */
{
  unsigned char zero_flag, character;
  unsigned long div;

  if((digit == 0) || (digit > 9)) return;

  if(number >= 0)				// display sign
    TFT_English('+');
  else
    { TFT_English('-');
      number = -number;
    }

  div = 1;
  while(--digit) div *= 10;

  zero_flag = zerofill;
  while(div > 0)				// display number
    { character = number / div;
      if((character == 0) && (zero_flag == 0) && (div != 1))
		drawChar(xcharacter,ycharacter+16,character+' ',Cyan,Black,1,1); // TFT_English(character + ' ');
      else
        { zero_flag = 1;
          drawChar(xcharacter,ycharacter+16,character+'0',Cyan,Black,1,1);//TFT_English(character + '0');
        }
      number %= div;
      div /= 10;
    }
}

void TFT_hexadecimal(U32 number, U08 digit)	/* display hexadecimal number */
{
  unsigned char i, character,count=0;

  if((digit == 0) || (digit > 8)) return;

  for(i = digit; i > 0; i--)
    { 
		count++;
		character = (number >> 4*(i-1)) & 0x0F;
		xcharacter+=5*count;
		if(xcharacter>230) { xcharacter=0;ycharacter+=16;}
		if(character < 10) drawChar(xcharacter,ycharacter+16,character+'0',Cyan,Black,1,1);//TFT_English(character + '0');//
		else  drawChar(xcharacter,ycharacter+16,character - 10 + 'A',Cyan,Black,1,1) ;  //TFT_English(character - 10 + 'A');// 
		//xcharacter+=8;
		
		
    }
	xcharacter+=8;
}

void TFT_0x_hexadecimal(U32 number, U08 digit)	/* display hexadecimal number with 0x */
{
  unsigned char i, character;

  if((digit == 0) || (digit > 8)) return;

  TFT_English('0');
  TFT_English('x');

  for(i = digit; i > 0; i--)
    { character = (number >> 4*(i-1)) & 0x0F;
      if(character < 10) TFT_English(character + '0');
      else               TFT_English(character - 10 + 'A');
    }
}

void TFT_unsigned_float(float number, U08 integral, U08 fractional) /* display unsigned floating-point number */
{
  unsigned char zero_flag, digit, character;	// integral = digits of integral part
  unsigned long div, integer;			// fractional = digits of fractional part

  digit = integral + fractional;
  if((integral == 0) || (fractional == 0) || (digit > 9)) return;

  div = 1;
  while(--digit) div *= 10;

  while(fractional--) number *= 10.;
  integer = (U32)(number + 0.5);

  zero_flag = 0;
  digit = 1;
  while(div > 0)				// display number
    { character = integer / div;
      if((character == 0) && (zero_flag == 0) && (digit != integral))
        TFT_English(character + ' ');
      else
        { zero_flag = 1;
          TFT_English(character + '0');
        }
      integer %= div;
      div /= 10;

      if(digit == integral)
        TFT_English('.');
      digit++;
    }
}

void TFT_signed_float(float number, U08 integral, U08 fractional) /* display signed floating-point number */
{
  unsigned char zero_flag, digit, character;
  unsigned long div, integer;

  digit = integral + fractional;
  if((integral == 0) || (fractional == 0) || (digit > 9)) return;

  if(number >= 0)				// display sign
    TFT_English('+');
  else
    { TFT_English('-');
      number = -number;
    }

  div = 1;
  while(--digit) div *= 10;

  while(fractional--) number *= 10.;
  integer = (U32)(number + 0.5);

  zero_flag = 0;
  digit = 1;
  while(div > 0)				// display number
    { character = integer / div;
      if((character == 0) && (zero_flag == 0) && (digit != integral))
        TFT_English(character + ' ');
      else
        { zero_flag = 1;
          TFT_English(character + '0');
        }
      integer %= div;
      div /= 10;

      if(digit == integral)
        TFT_English('.');
      digit++;
    }
}

/* ---------------------------------------------------------------------------- */
/*		�׷��� �Լ�							*/
/* ---------------------------------------------------------------------------- */

void Line(S16 x1,S16 y1, S16 x2,S16 y2, U16 color)	/* draw a straight line */
{
  int x, y;

  if(y1 != y2)					// if y1 != y2, y is variable
    { if(y1 < y2)				//              x is function
        for(y = y1; y <= y2; y++)
          { x = x1 + (long)(y - y1)*(long)(x2 - x1)/(y2 - y1);
            TFT_pixel(x, y, color);
          }
      else
        for(y = y1; y >= y2; y--)
          { x = x1 + (long)(y - y1)*(long)(x2 - x1)/(y2 - y1);
            TFT_pixel(x, y, color);
          }
    }
  else if(x1 != x2)				// if x1 != x2, x is variable
    { if(x1 < x2)				//              y is function
        for(x = x1; x <= x2; x++)
          { y = y1 + (long)(x - x1)*(long)(y2 - y1)/(x2 - x1);
            TFT_pixel(x, y, color);
          }
      else
        for(x = x1; x >= x2; x--)
          { y = y1 + (long)(x - x1)*(long)(y2 - y1)/(x2 - x1);
            TFT_pixel(x, y, color);
          }
    }
  else						// if x1 == x2 and y1 == y2, it is a dot
    TFT_pixel(x1, y1, color);
}

void Rectangle(S16 x1,S16 y1, S16 x2,S16 y2, U16 color) /* draw a rectangle */
{
  Line(x1,y1, x1,y2, color);			// horizontal line
  Line(x2,y1, x2,y2, color);
  Line(x1,y1, x2,y1, color);			// vertical line
  Line(x1,y2, x2,y2, color);
}

void Block(S16 x1,S16 y1, S16 x2,S16 y2, U16 color, U16 fill) /* draw a rectangle with filled color */
{
  int i;

  Line(x1,y1, x1,y2, color);			// horizontal line
  Line(x2,y1, x2,y2, color);
  Line(x1,y1, x2,y1, color);			// vertical line
  Line(x1,y2, x2,y2, color);

  if((y1 < y2) && (x1 != x2))			// fill block
    { for(i = y1+1; i <= y2-1; i++)
         Line(x1+1,i, x2-1,i, fill);
    }
  else if((y1 > y2) && (x1 != x2))
    { for(i = y2+1; i <= y1-1; i++)
         Line(x1+1,i, x2-1,i, fill);
    }
}

void Circle(S16 x1,S16 y1, S16 r, U16 color)	/* draw a circle */
{
  int x, y;
  float s;

  for(y = y1 - r*3/4; y <= y1 + r*3/4; y++)	// draw with y variable
    { s = sqrt((long)r*(long)r - (long)(y-y1)*(long)(y-y1)) + 0.5;
      x = x1 + (int)s;
      TFT_pixel(x, y, color);
      x = x1 - (int)s;
      TFT_pixel(x, y, color);
    }

  for(x = x1 - r*3/4; x <= x1 + r*3/4; x++)	// draw with x variable
    { s = sqrt((long)r*(long)r - (long)(x-x1)*(long)(x-x1)) + 0.5;
      y = y1 + (int)s;
      TFT_pixel(x, y, color);
      y = y1 - (int)s;
      TFT_pixel(x, y, color);
    }
}

void Sine(S16 peak, U08 mode, U16 color)	/* draw a sine curve */
{
  int x, y;

  if(mode == 0)
    for(y = 0; y <= 319; y++)
      { x = 120 - (int)(sin((float)y * 1.6875 * M_PI / 180.) * peak + 0.5);
        TFT_pixel(x, y, color);
      }
  else
    for(y = 0; y <= 319; y++)
      { x = 120 + (int)(sin((float)y * 1.6875 * M_PI / 180.) * peak + 0.5);
        TFT_pixel(x, y, color);
      }
}


#endif