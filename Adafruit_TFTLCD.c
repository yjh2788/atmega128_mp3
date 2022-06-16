
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
#include <util/delay.h>

// LCD ��ŸƮ ���ɾ� ����
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
	0x00,
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
	0x8c,//0b11100000, // ���⸦ �ٲ�� ��  1110, 0011, xx1x�� �ٲٱ� 
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

	//reset();

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
			// Serial.print("Register $"); Serial.print(r, HEX);
			// Serial.print(" datalen "); Serial.println(len);

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
	//   CS_IDLE;
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {

	// Clip
	if ((x < 0) || (y < 0) || (x >= TFTWIDTH) || (y >= TFTHEIGHT))
	return;

	CS_ACTIVE;
	setAddrWindow(x, y, TFTWIDTH - 1, TFTHEIGHT - 1);
	//setAddrWindow(x, y, x+2, y+2);
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