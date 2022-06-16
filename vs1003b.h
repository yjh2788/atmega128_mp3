/* VS1003B - Using the VS1003B MP3/WMA/WAV/MID decoder
 *
 * Copyright (c) 2013, Sven Hesse <drmccoy@drmccoy.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VS1003B_H_
#define VS1003B_H_

#include "C:\Users\spc\Desktop\microprocessor\types.h"
#include "C:\Users\spc\Desktop\microprocessor\util.h"
#include "global.h"
#include <avr/io.h>

// VS1003 SCI Write Command byte is 0x02
#define VS_WRITE_COMMAND 0x02

// VS1003 SCI Read COmmand byte is 0x03
#define VS_READ_COMMAND  0x03

// SCI Registers



// SCI_MODE bits



// Commands
#define COMMAND_MODE       0x00
#define COMMAND_STATUS     0x01
#define COMMAND_BASS       0x02
#define COMMAND_CLOCKF     0x03
#define COMMAND_DECODETIME 0x04
#define COMMAND_AUDATA     0x05
#define COMMAND_WRAM	   0X06
#define COMMAND_WRAMADDR   0X07
#define COMMAND_HEADER0    0x08
#define COMMAND_HEADER1    0x09
#define COMMAND_AIADDR	   0X0A //어플리케이션 시작 주소
#define COMMAND_VOLUME     0x0B

// Remember the current volume and bass/treble setting
static uint16_t vs1003b_current_volume = 0xFEFE;
static uint16_t vs1003b_current_bass = 0x0000;

uint8_t volume=0;
uint8_t bass=0;
uint8_t treble=0;

#define SPIF 7
// -- Low level bit fiddling --
#define XCS 5// PB5
#define DREQ 6//PB6
#define XRESET 3//PE3
#define XDCS 0//PF0
//#define SCK PB1
//#define MOSI PB2
//#define MISO PB3

typedef enum {
	kVS1003BFormatNone = 0,
	kVS1003BFormatWAV,
	kVS1003BFormatMP3,
	kVS1003BFormatWMA,
	kVS1003BFormatMID,
	kVS1003BFormatUnknown
} vs1003b_format_t;

/** Initialize the VS1003B MP3/WMA/WAV/MID decoder. */
bool vs1003b_init(void);

/** Reset the VS1003B. */
void vs1003b_reset(void);

/** Is the VS1003B ready to receive more data? */
bool vs1003b_ready(void);

/** Feed the VS1003B data.
 *
 *  @param data  The data to write.
 *  @param count The number of bytes to write.
 *
 *  @return The number of bytes successfully written to the VS1003B.
 */
uint16_t vs1003b_feed_data(uint8_t *data, uint16_t count);

/** Stop playing/decoding. */
void vs1003b_stop(void);

/** Set the volume for the left and right output channel. */
void vs1003b_set_volume(uint8_t left, uint8_t right);

/** Return the current volume. */
void vs1003b_get_volume(uint8_t *left, uint8_t *right);

/** Return the number of seconds the current data has been decoded. */
uint16_t vs1003b_get_decode_time(void);

/** Return the currently playing format. */
vs1003b_format_t vs1003b_get_format(void);

/** Control the bass and treble control.
 *
 *  @param bass_amplitude   Bass enhancement in dB (0-15). 0 = off
 *  @param bass_freqlimit   Lower limit frequency of the bass enhancement in Hz (20-150). Granularity is 10Hz.
 *  @param treble_amplitude Treble control in 1.5dB steps (-8-7). 0 = off.
 *  @param treble_freqlimit Lower limit frequency of the treble control in kHz (0-15).
 *
 *  @return TRUE if the parameters were within valid range and the control was applied.
 */
bool vs1003b_set_bass_treble(uint8_t bass_amplitude, uint8_t bass_freqlimit, int8_t treble_amplitude, uint8_t treble_freqlimit);

void vs1003_write_reg(uint8_t reg, uint16_t data);


//*******************************************************************************************************************************************
//
//************************함수정의
//
//**************************************************************************************

static void vs1003b_set_cs(void) {
	//PORTB &= ~(1 << XCS);
	sbi(PORTB,XCS);
}

static void vs1003b_set_dcs(void) {
	//PORTE &= ~(1 << XDCS);
	sbi(PORTF,XDCS);
}

static void vs1003b_clear_cs(void) {
	//PORTB |= 1 << XCS;
	cbi(PORTB,XCS);
}

static void vs1003b_clear_dcs(void) {
	//PORTE |= 1 <<XDCS;
	cbi(PORTF,XDCS);
}

static void vs1003b_spi_send(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
}

static uint8_t vs1003b_spi_receive(void) {
	SPDR = 0x00;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

static bool vs1003b_is_busy(void) {
	return !(PINB & (1 << DREQ));
}

static void vs1003b_hardware_reset(void) {
	//PORTE &= ~(1 << XRESET);
	cbi(PORTE,XRESET);
	_delay_us(1);
	//PORTE |=   1 << XRESET;
	sbi(PORTE,XRESET);
}


// -- Internal command and data wrangling --
void vs1003_write_reg(uint8_t reg, uint16_t data)
{
	while((vs1003b_is_busy()));

	vs1003b_clear_cs();

	SPI_write(COMMAND_WRAM) ;
	SPI_write(reg) ;
	SPI_write((data >> 8) & 0xFF);
	SPI_write(data & 0xFF) ;

	vs1003b_set_cs();
}

static void vs1003b_wait_while_busy(void) {
	while (vs1003b_is_busy());
}

static void vs1003b_command_write(uint8_t reg, uint16_t data) {
	
	vs1003b_clear_cs();
	vs1003b_spi_send(0x02);

	vs1003b_spi_send(reg);

	vs1003b_spi_send((data & 0xFF00) >> 8);
	vs1003b_spi_send( data & 0x00FF);
	vs1003b_set_cs();
	
}

static uint16_t vs1003b_command_read(uint8_t reg) {
	
	vs1003b_clear_cs();
	vs1003b_spi_send(0x03);

	vs1003b_spi_send(reg);

	uint8_t high = vs1003b_spi_receive();
	uint8_t low  = vs1003b_spi_receive();

	
	vs1003b_set_cs();
	return (((uint16_t) high) << 8) | low;
}

static void vs1003b_command_bass(uint16_t control) {
	vs1003b_current_bass = control;

	vs1003b_command_write(COMMAND_BASS, control);
	vs1003b_wait_while_busy();
}

static void vs1003b_command_volume(uint16_t volume) {
	vs1003b_current_volume = volume;

	vs1003b_command_write(COMMAND_VOLUME, volume);
	vs1003b_wait_while_busy();
}

static void vs1003b_data_write(uint8_t *data, uint8_t n) {
	
	vs1003b_clear_dcs();
	while (n-- > 0)
		vs1003b_spi_send(*data++);

	vs1003b_set_dcs();
}

static void vs1003b_data_empty() {
	if (vs1003b_is_busy())
		return;
	vs1003b_clear_dcs();
	

	for (int i = 0; i < 32; i++)
		vs1003b_spi_send(0x00);
	vs1003b_set_dcs();
	
}

static void vs1003b_init_registers(void) {
	vs1003b_wait_while_busy();

	/* Set a clock multiplier of 4.5x.
	   This is the highest supported clock and will allow the most simultaneous features.
	   It will also draw the most power.
	 */
	vs1003b_command_write(COMMAND_CLOCKF, 0xE000);
	vs1003b_wait_while_busy();

	vs1003b_command_volume(vs1003b_current_volume);
	vs1003b_command_bass(vs1003b_current_bass);
}


// -- Public VS1003B functions --

bool vs1003b_init(void) {
	
	
	volume =200;					// initial volume = 200
	bass = 0;					// initial bass and treble = 0
	treble = 0;
	SPI_mode0_slow();
	vs1003b_reset();
	_delay_ms(2);
	
	vs1003b_set_volume(volume,volume);
	while(vs1003b_is_busy());
	vs1003b_set_bass_treble(bass,100,treble,5);
	while(vs1003b_is_busy());
	vs1003b_command_write(COMMAND_CLOCKF,0xe000); // Experimenting with higher clock settings
	while(vs1003b_is_busy());
	vs1003b_command_write(COMMAND_AUDATA,44101); // 44.1kHz stereo
	//while(vs1003b_is_busy());


	// vs1003b_command_write(COMMAND_VOLUME,0x2020); // VOL	
	// 
	 // soft reset
	// vs1003b_command_write(COMMAND_MODE, _BV(SM_SDINEW) | _BV(SM_RESET));

	// await_data_request();
	 
	//SPI_mode0_4MHz();
	
	
	//	VS1003_RegWrite(VS1003_CLOCKF_REG, 0x9800);
	//	VS1003_RegWrite(VS1003_INT_FCTLH_REG, 0x8008);
	//b8
	//vs1003b_command_write(COMMAND_CLOCKF,0xe000); 
	// Make sure we're really using a VS1003B
	uint16_t mode   = vs1003b_command_read(COMMAND_MODE);
	uint16_t status = vs1003b_command_read(COMMAND_STATUS);
	uint16_t clk=vs1003b_command_read(COMMAND_CLOCKF);
	uint16_t bassre = vs1003b_command_read(COMMAND_BASS);
	uint16_t vol=vs1003b_command_read(COMMAND_VOLUME);

	
	
	//TFT_hexadecimal(mode, 4);
	//TFT_hexadecimal(status,4);
	//TFT_hexadecimal(clk,4);
	//TFT_hexadecimal(bassre,4);
	//TFT_hexadecimal(vol,4);
	//while(1);
	_delay_ms(1000);
	
	
	if (((status & 0x0030) != 0x0030) || !(mode & 0x0800))
	{
		TFT_string(0,240,Cyan,Black," VS1003b initialize failed");
		while(1);
		return FALSE;
	}
	if((status==0xff)||(mode==0xff)) 
	{
		TFT_string(0,240,Cyan,Black," VS1003b initialize failed");
		while(1);
		return FALSE;
		
	}
	TFT_string(0,240,Cyan,Black," VS1003b initialized");
	return TRUE;
}

void vs1003b_reset(void) {
	vs1003b_hardware_reset();
	vs1003b_init_registers();
}

bool vs1003b_ready(void) {
	return !vs1003b_is_busy();
}

uint16_t vs1003b_feed_data(uint8_t *data, uint16_t count) {
	uint16_t processed = 0;
	while (count > 0) {
		if (!vs1003b_ready())
			break;

		uint8_t n = MIN(count, 32);

		vs1003b_data_write(data, n);

		processed += n;
		count     -= n;
		data      += n;
	}

	return processed;
}

void vs1003b_stop(void) {
	// Mute
	vs1003b_command_write(COMMAND_VOLUME, 0xFEFE);

	// Send stop command and write 0x00 until the format field has been cleared (= no current sound). MP3 can be stopped immediately.
	vs1003b_command_write(COMMAND_MODE, 0x8008);
	vs1003b_format_t format = vs1003b_get_format();
	while ((format != kVS1003BFormatNone) && (format != kVS1003BFormatMP3)) {
		vs1003b_data_empty();
		format = vs1003b_get_format();
	}

	// Reset the VS1003B, just to be sure a broken format didn't mess anything up
	vs1003b_reset();
}

void vs1003b_set_volume(uint8_t left, uint8_t right) {
	// 0 is full volume, 254 is silence
	left  = 254 - MIN(left , 254);
	right = 254 - MIN(right, 254);

	vs1003b_command_volume(((uint16_t)left << 8) | right);
}

void vs1003b_get_volume(uint8_t *left, uint8_t *right) {
	*left  = 254 - ((vs1003b_current_volume & 0xFF00) >> 8);
	*right = 254 -  (vs1003b_current_volume & 0x00FF);
}

uint16_t vs1003b_get_decode_time(void) {
	return vs1003b_command_read(COMMAND_DECODETIME);
}

vs1003b_format_t vs1003b_get_format(void) {
	uint16_t header1 = vs1003b_command_read(COMMAND_HEADER1);

	if (header1 == 0x0000)
		return kVS1003BFormatNone;
	if (header1 == 0x7665)
		return kVS1003BFormatWAV;
	if (header1 == 0x574D)
		return kVS1003BFormatWMA;
	if (header1 == 0x4D54)
		return kVS1003BFormatMID;

	if ((header1 >> 5) == 0x7FF)
		return kVS1003BFormatMP3;

	return kVS1003BFormatUnknown;
}

bool vs1003b_set_bass_treble(uint8_t bass_amplitude, uint8_t bass_freqlimit, int8_t treble_amplitude, uint8_t treble_freqlimit) {
	if (bass_amplitude > 15)
		return FALSE;
	if ((bass_freqlimit < 20) || (bass_freqlimit > 150))
		return FALSE;
	if ((treble_amplitude < -8) || (treble_amplitude > 7))
		return FALSE;
	if (treble_freqlimit > 15)
		return FALSE;

	bass_freqlimit /= 10;

	uint16_t control = (((uint16_t)((uint8_t)treble_amplitude)) << 12) | (((uint16_t)treble_freqlimit) << 8) | (bass_amplitude << 4) | (bass_freqlimit);

	vs1003b_command_bass(control);
	return TRUE;
}






#endif /* VS1003B_H_ */
