/*
 * myGFX.c
 *
 * Created: 2022-05-19 오전 10:06:46
 *  Author: Wooseok
 */ 

#include "myGFX.h"
#include <avr/pgmspace.h>

void writeFillRect(int16_t x, int16_t y, int16_t size_x, int16_t size_y, uint16_t color)
{
	for (short i = 0; i < size_x; i++)
		drawSLine(x+1, y + i, size_y, color);	
}


void writeFastVLine(int16_t x, int16_t y, int16_t i, uint16_t bg)
{
	// Overwrite in subclasses if startWrite is defined!
	// Can be just 
	writeLine(x, y, x, y+i-1, bg);
	// or writeFillRect(x, y, 1, h, bg);
};

void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
		} 
	else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++)
	{
		drawPixel(y0, x0, color);
		err -= dy;
		
		if (err < 0) 
		{
			y0 += ystep;
			err += dx;
		}
	}
}


