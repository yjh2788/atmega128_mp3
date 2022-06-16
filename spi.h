#include "global.h"

#ifndef spi.h
#define spi.h
/////bit_order
#define msb_fst 0B00000000
#define lsb_fst 0B00100000


//--------------clk모드------------------------------------------//
#define SPI_CLK_DIV2    0x04
#define SPI_CLK_DIV4    0x00
#define SPI_CLK_DIV8    0x05
#define SPI_CLK_DIV16   0x01
#define SPI_CLK_DIV32   0x06
#define SPI_CLK_DIV64   0x02
#define SPI_CLK_DIV128  0x03
//--------------------------------------------------------------//
//-------------spi모드0~3---------------------------------------//
#define SPI_MODE0       0x00    //CPOL=0,CPHA=0
#define SPI_MODE1       0x01    //CPOL=0,CPHA=1
#define SPI_MODE2       0x02    //CPOL=1,CPHA=0
#define SPI_MODE3       0x03    //CPOL=1,CPHA=1
//--------------------------------------------------------------//
//-------------함수---------------------------------------------//
void spi_init(uint8_t clkDiv ,uint8_t bit_order, uint8_t mode);
uint8_t spi_transfer_byte(uint8_t val);
uint16_t spi_transfer_word(uint16_t val);
//void spi_transfer_block(uint8_t *buffer,uint8_t n);
//void spi_slave_init(uint16_t bit_order, uint16_t mode);
//uint8_t spi_slave_read_byte(uint8_t val);
//uint16_t spi_slave_read_word(uint16_t val);


//#define DC 0X016//PE5_출력 TFT LCD 명령/데이터선
//#define CS PB4//0X04//PB4_출력
#define MISO 0x04//0x03//PB3_입력
#define MOSI 0x02//0x02//PB2_출력
#define SCK  0x01//0x01//PB1_출력

#define SPE_  0x40//spi 인에이블
#define MSTR_ 0x10//마스터모드 1, 슬레이브 모드 0


void spi_init(uint8_t clkDiv ,uint8_t bit_order, uint8_t mode)
{
    uint8_t clk_div;
    clk_div=clkDiv&0b00000011;
    PORTB |= (MISO|MOSI|SCK);//
    //PORTE |= DC;

    SPCR |= (SPE_|bit_order|MSTR_|(mode<<2)|(clk_div&0b00000011));

    if((clk_div&0b00000100)==0b00000100)
    {
        SPSR|=0b00000001;
    }
    
}
uint8_t spi_transfer_byte(uint8_t val)
{

    SPDR=val;
    while((SPSR&0b10000000)==0b00000000);///SPIF=1이 되어 전송이 완료될때까지 대기
    return SPDR;

}
uint16_t spi_transfer_word(uint16_t val)
{   
    uint16_t word;
    SPDR=(uint8_t)(val>>8);//상위 8비트 전송
    while((SPSR&0b10000000)==0b00000000);///SPIF=1이 되어 전송이 완료될때까지 대기
    word|=((uint16_t)SPDR)<<8;

    SPDR=(uint8_t)(val&0x00ff);//하위 8비트 전송
    while((SPSR&0b10000000)==0b00000000);///SPIF=1이 되어 전송이 완료될때까지 대기
    word|=SPDR;
    return word;
}
/*
void spi_slave_init(uint16_t bit_order, uint16_t mode)
{

}
uint8_t spi_slave_read_byte(uint8_t val)
{

}
uint16_t spi_slave_read_word(uint16_t val)
{

}

void spi_transfer_block(uint8_t *buffer,uint8_t n)
{

}*/
#endif