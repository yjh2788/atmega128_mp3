// Adafruit_TFTLCD.h

#pragma once

#include <stdint.h>
#include <avr/io.h>

void reset(void);
void begin(void);
void setAddrWindow (int x1, int y1, int x2, int y2);
void flood (uint16_t color, uint32_t len);
void drawPixel (int16_t x , int16_t y, uint16_t color);
void writeRegister32 (uint8_t r, uint32_t d);

void drawSLine(int16_t x, int16_t y,int16_t length,   uint16_t color);
	// x ��ǥ ,y ��ǥ , ����, ���� , ����
void drawLLine(int16_t x, int16_t y,int16_t length,  uint16_t color);

void drawChar(int16_t x, int16_t y, unsigned char c,
	uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);