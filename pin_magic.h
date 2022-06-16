#pragma once

#ifndef pin_magic
#define pin_magic
#include "util/delay.h"
#define __DELAY_BACKWARD_COMPATIBLE__

#define DELAY7                                                  \
asm volatile("rjmp .+0"                                         \
"\n\t"                                                          \
"rjmp .+0"                                                      \
"\n\t"                                                          \
"rjmp .+0"                                                      \
"\n\t"                                                          \
"nop"                                                           \
"\n" ::);
// 어샘블리어로 되어있는 부분은 _delay 함수로 변경하도록 하자.

#define RD_PORT PORTC /*pin G0 */ // C로 바꿈
#define WR_PORT PORTC /*pin G1 */
#define CD_PORT PORTC /*pin G2 */
#define CS_PORT PORTC /*pin G3 */
#define RD_MASK 0B10000000 // RD로 마스킹을 할때,
#define WR_MASK 0B00100000
#define CD_MASK 0B00001000
#define CS_MASK 0B00000010

#define RD_ACTIVE RD_PORT &= ~RD_MASK
#define RD_IDLE RD_PORT |= RD_MASK
#define WR_ACTIVE WR_PORT &= ~WR_MASK
#define WR_IDLE WR_PORT |= WR_MASK
#define CD_COMMAND CD_PORT &= ~CD_MASK
#define CD_DATA CD_PORT |= CD_MASK
#define CS_ACTIVE CS_PORT &= ~CS_MASK
#define CS_IDLE CS_PORT |= CS_MASK

#define write8 write8inline
#define read8 read8inline

#define WR_STROBE                                                              \
{                                                                              \
	WR_ACTIVE;                                                                 \
	WR_IDLE;                                                                   \
}

// 읽기 쓰기 처리 부분  - 해석에 따라 변경가능한 부분
 // 작품 회로 비트순서 반전
#define write8inline(d)										                   \
{                                                                              \
	PORTA = d;                                                                 \
	WR_STROBE;                                                                 \
}

#define read8inline(result)                                                    \
{                                                                              \
	RD_ACTIVE;                                                                 \
	_delay_ms(7);                                                              \
	result = PINA;                                                             \
	RD_IDLE;                                                                   \
}



// 8 bit 데이터를 어떻게 읽어올지 에 대한 함수를 설정해보자 그래서 어떻게 읽음?
// PINA 에서 읽어온다.
#define writeRegister8inline(a, d)                                             \
{                                                                              \
	CD_COMMAND;                                                                \
	write8(a);                                                                 \
	CD_DATA;                                                                   \
	write8(d);                                                                 \
}

#define writeRegister16inline(a, d)                                            \
{                                                                              \
	uint8_t hi, lo;                                                            \
	hi = (a) >> 8;                                                             \
	lo = (a);                                                                  \
	CD_COMMAND;                                                                \
	write8(hi);                                                                \
	write8(lo);                                                                \
	hi = (d) >> 8;                                                             \
	lo = (d);                                                                  \
	CD_DATA;                                                                   \
	write8(hi);                                                                \
	write8(lo);                                                                \
}
#define writeRegisterPairInline(aH, aL, d)                                     \
{                                                                              \
	uint8_t hi = (d) >> 8, lo = (d);                                           \
	CD_COMMAND;                                                                \
	write8(aH);                                                                \
	CD_DATA;                                                                   \
	write8(hi);                                                                \
	CD_COMMAND;                                                                \
	write8(aL);                                                                \
	CD_DATA;                                                                   \
	write8(lo);                                                                \
}



// 써야하는 PORTA 를 쓰기모드로 변경 즉 입력으로 만들기
#define setWriteDirInline() DDRA = 0x0;

// 써야하는 PORTA 를 읽기모드로 변경 즉 출력으로 만들기
#define setReadDirInline() DDRA  = 0xff;




#endif pin_magic