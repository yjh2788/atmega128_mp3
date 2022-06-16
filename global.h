
#ifndef global.h
#define global.h

typedef unsigned char uint8_t;
typedef unsigned int   uint16_t;
typedef unsigned char	U08;			/* data type definition */
typedef   signed char	S08;
typedef unsigned int	U16;
typedef   signed int	S16;
typedef unsigned long	U32;
typedef   signed long	S32;

#define BV(bit)		(1<<(bit))		/* bit processing */
#define cbi(reg,bit)	reg &= ~(BV(bit))//비트 클리어
#define sbi(reg,bit)	reg |= (BV(bit))//비트 셋


#define DISOFF          0x28

#define spd1 0
#define spd2 1
/*
#define PB0 0X00
#define PB1 0X01
#define PB2 0X02
#define PB3 0X04
#define PB4 0X08
#define PB5 0X10
#define PB6 0X20
#define PB7 0X40
#define PB8 0X80

#define PE0 0X00
#define PE1 0X01
#define PE2 0X02
#define PE3 0X04
#define PE4 0X08
#define PE5 0X10
#define PE6 0X20
#define PE7 0X40
#define PE8 0X80

#define PD0 0X00
#define PD1 0X01
#define PD2 0X02
#define PD3 0X04
#define PD4 0X08
#define PD5 0X10
#define PD6 0X20
#define PD7 0X40
#define PD8 0X80
*/
#define HX8357_NOP 0x00
#define HX8357_SWRESET 0x01
#define HX8357_RDDID 0x04
#define HX8357_RDDST 0x09
#define HX8357_SLPIN 0x10
#define HX8357_SLPOUT 0x11
#define HX8357_INVOFF 0x20
#define HX8357_INVON 0x21
#define HX8357_DISPOFF 0x28
#define HX8357_DISPON 0x29
#define HX8357_CASET 0x2A
#define HX8357_PASET 0x2B
#define HX8357_RAMWR 0x2C
#define HX8357_RAMRD 0x2E
#define HX8357_TEON 0x35
#define HX8357_TEARLINE 0x44
#define HX8357_MADCTL 0x36
#define HX8357_COLMOD 0x3A
#define HX8357_SETOSC 0xB0
#define HX8357_SETPWR1 0xB1
#define HX8357_SETRGB 0xB3
#define HX8357D_SETCOM 0xB6
#define HX8357D_SETCYC 0xB4
#define HX8357D_SETC 0xB9
#define HX8357D_SETSTBA 0xC0
#define HX8357_SETPANEL 0xCC
#define HX8357D_SETGAMMA 0xE0
/*
void digitalWrite(char pin, char level);
void pinmode(char pin, char IO);
int analogeRead(char pin);
void digitalWrite(char pin, char level)
{
    PIND|=level? (1<<pin):(0<<pin);
}
void pinmode(char pin, char IO)
{

}
int analogeRead(char pin)
{

}*/


#endif