/*
* SD_.c
*
* Created: 2022-05-05 오전 2:49:29
*  Author: Zephyrus
*/
//#include <stdio.h>
//#ifndef __SDCARD_H__
//#define __SDCARD_H__

#include <util/delay.h>
#include "SDcard.h"
#include <avr/io.h>
#include "TFTLCD_base.h"

#define _BV(n) (1 << n)
// 비트 클리어
#define cbi(reg, bit) reg &= ~(_BV(bit))
// 비트 셋
#define sbi(reg, bit) reg |= (_BV(bit))

	
#define check(t) {			\
	PORTD = 0x0;		\
	_delay_ms((t));	\
	PORTD = 0xFF;	\
	_delay_ms((t));		\
}					\	
	
/************************************************************************
************************************************************************/


// 파일 시작 cluster
uint32_t	fileStartClust[MAX_NUM_FILE];
uint32_t	file_len[MAX_NUM_FILE];
uint32_t	RootDirSector;
uint16_t	numOfFile;			// 파일명
uint32_t	First_FAT_Sector;
u_char		SecPerClus;
u_char		sd_type;
u_char		FileName[MAX_NUM_FILE][9];
u_char		buffer[BytePerSec];

u_char		stop_play = 0;
u_char		repeat = 0;
uint16_t	sel_pg = 1;
uint16_t	sel_no = 0;
uint16_t	sel_tno;
uint16_t	tot_pg;
uint16_t	tot_no;
uint16_t	play_no = 0;
u_char		sbuf[20];

void SD_Init(void)
{
	u_char i, j, status = 1, res1, cmd_flag = 0;
	u_char r1_response;
	u_char r7_response[4];
	u_char r3_response[7];
	
	
	// SD카드 체크 /////////////////////////////////////////////////////
	
	u_char tmphex = 0;
	
	 sbi(DDRB, CS_SD);				// -CS_SD is output
	 TFT_color_screen(Black);
	 TFT_string(0,0,Green,  Black,"******************************");
	 TFT_string(0,2,Magenta,Black,"     SD/SDHC initialize...     ");
	 TFT_string(0,4,Green,  Black,"******************************");
//////////////////////////////////////////////////////////////////
	
	_delay_ms(100);
	SPI_mode0_slow(); // SPI 250kHz (100 ~ 400 kHz)
	
	// SD_CS = High로 놓고 Dummy Data 10번 전송
	// 74 클럭 이상 인가해서 SPI 모드 진입
	sbi(PORTB, SD_CS);
	for(i=0; i<10; i++) SPI_write(0xFF);
	
	//	SD_CS = 0;
	cbi(PORTB, SD_CS);
	///////////////////////////////////////////////////////
	//printString(tftlcd_xpos, tftlcd_ypos, "CMD 0 : ", 0xFFFF, 0x0, 1);
	///////////////////////////////////////////////////////
	_delay_ms(1);
	
	SD_Command(CMD0, 0);
	_delay_ms(10);
	
	for(i = 0; i<10 ;i++){
		tmphex = SPI_write(0xFF);
		///////////////////////////////////////////////////////
		//printHex(tftlcd_xpos, tftlcd_ypos, tmphex, 1);
		//printString(tftlcd_xpos, tftlcd_ypos, " ", 0xFFFF, 0x0, 1);
		///////////////////////////////////////////////////////
		_delay_ms(3);
	}
	
	SD_Command(CMD8, 0x1AA);
	///////////////////////////////////////////////////////
	//printString(tftlcd_xpos, tftlcd_ypos, "\nCMD8 : ", 0xFFFF, 0x0, 1);
	///////////////////////////////////////////////////////
	_delay_ms(10);
	
	do {
		r1_response = SPI_write(0xFF);

		if(r1_response == 0x05){
			sd_type = SD_VER1;
			///////////////////////////////////////////////////////
			//printString(tftlcd_xpos, tftlcd_ypos, "V1.x\n", 0xFFFF, 0xF800, 1);
			///////////////////////////////////////////////////////
			break;
		}
		else if(r1_response == 0x01){
			sd_type = SD_VER2;
			///////////////////////////////////////////////////////
			//printString(tftlcd_xpos, tftlcd_ypos, "V2.x\n", 0xFFFF, 0xF800, 1);
			///////////////////////////////////////////////////////
			break;
		}
	} while(1);
	
	
	switch(sd_type){
		case SD_VER2:
		// R7 Response 수신 4 u_chars
		// (R1 Response 는 do while 에서 수신)
		for(i = 0; i<4; i++) r7_response[i] = SPI_write(0xFF);
		
		if(r7_response[2] == 0x01 && r7_response[3] == 0xAA){
			// check(500);
			/*
			while(SPI_write(0xFF) != 0){
				SD_Command(CMD55, 0);
				SD_Command(CMD41, 0X40000000);
			}
			*/
			//check(500);
			
			// 확인
			//check(500);
			
			SPI_write(0xFF);
			for(j=0; j<10; j++){
				// 5번 이상 반복해야 R1 Response = 0x0 수신 가능
				
				SD_Command(CMD55, 0);
				///////////////////////////////////////////////////////
				///printString(tftlcd_xpos, tftlcd_ypos, "CMD55 : ", 0xFFFF, 0x0, 1);
				///////////////////////////////////////////////////////
				// R1 Response
				for(i=0;i<3;i++){
					tmphex = SPI_write(0xFF);
					///////////////////////////////////////////////////////
					//printHex(tftlcd_xpos, tftlcd_ypos, tmphex, 1);
					//printString(tftlcd_xpos, tftlcd_ypos, " ", 0xFFFF, 0x0, 1);
					///////////////////////////////////////////////////////
					
				}
				//printString(tftlcd_xpos, tftlcd_ypos, "\n", 0xFFFF, 0x0, 1);
				
				SD_Command(CMD41, 0x40000000); // HCS bit = 1
				for(i =0 ; i< 10; i++){
					res1 = SPI_write(0xFF);		// R1 Response 
					if(res1 == 0x0) cmd_flag = 1;
					//printHex(tftlcd_xpos, tftlcd_ypos, res1, 1);
					//printString(tftlcd_xpos, tftlcd_ypos, " ", 0xFFFF, 0x0, 1);
				}
				//printString(tftlcd_xpos, tftlcd_ypos, "\n", 0xFFFF, 0x0, 1);
				_delay_ms(3);
			}	
		} //end of if
		if(cmd_flag == 1) //printString(tftlcd_xpos, tftlcd_ypos, "SD ready!\n", 0xFFFF, 0xF800, 2);
		
		SD_Command(CMD58, 0);			// check ccs bit
		_delay_ms(10);
		
		for(i = 0;i <5;i++){
			r3_response[0] = SPI_write(0xff);
			_delay_ms(3);
			if(r3_response[0] == 0x0){
				for(j = 1; j<5; j++){
					r3_response[j] = SPI_write(0xff);
					_delay_ms(3);
				}
				break;
			}
		}
		if((r3_response[1] & 0x40) != 0){ 
			sd_type = SD_VER2_HC;
			//printString(tftlcd_xpos, tftlcd_ypos, "SDHC card\n", 0xFFFF, 0xF800, 1);
		}
		else {sd_type = SD_VER2_SD;
			//printString(tftlcd_xpos, tftlcd_ypos, "SDSD card\n", 0xFFFF, 0xF800, 1);
		}
		
		for(i = 0; i<10; i++){
			SPI_write(0xFF);
			_delay_ms(1);
		}
		break;
		
		
		// case ver1
		case SD_VER1:
		for(j = 0; j<5; j ++){
			SD_Command(CMD55, 0);
			_delay_ms(10);
			for(i = 0; i < 10; i++){
				SPI_write(0xFF);
				_delay_ms(3);
			}
			
			SD_Command(CMD41, 0x00000000);
			_delay_ms(10);
			for(i=0;i<10;i++){
				SPI_write(0xFF);
				_delay_ms(3);
			}
		}
		break;
	}
	//	SD_CS = 1;
	sbi(PORTB, SD_CS);
}

void SD_Command(u_char cmd, uint32_t arg)
{
	u_char crc;
	
	SPI_write(cmd | 0x40);		// transition bit 1(6비트째) | cmd
	SPI_write(arg >> 24);		// 4byte 인수 전송
	SPI_write(arg >> 16);
	SPI_write(arg >> 8);
	SPI_write(arg);
	
	crc = 0x01;
	if(cmd == CMD0) crc = 0x95;		// CRC for CMD0
	if(cmd == CMD8) crc = 0x87;		// CRC for CMD8(0x000001AA)
	// else crc = 0xFF;				// CRC for other CMD
	SPI_write(crc);
}

void SD_Read(uint32_t sector)
{
	uint16_t i;
	SPI_mode0_4MHz();
	
	// High Capacity 카드가 아니면 섹터번호에 *512
	if(sd_type == SD_VER1 || sd_type == SD_VER2_SD) sector = sector << 9;
	
	//	SD_CS = 0;			// SD_CS = Low (SD 카드 활성화)
	cbi(PORTB, SD_CS);
	//_delay_ms(100);
	
	SD_Command(CMD17, sector);

	while(SPI_write(0xFF) != 0x00);		// wait for R1 = 0x00
	while(SPI_write(0xFF) != 0xFE);		// wait for Start Block Token = 0xFE
	//check(500);
	for(i=0; i<512; i++) buffer[i] = SPI_write(0xFF);
	
	// CRC 수신 (2 byte)
	SPI_write(0xFF);
	SPI_write(0xFF);
	SPI_write(0xFF);
	
	//	SD_CS = 1;				// SD 카드 비활성화
	sbi(PORTB, SD_CS);
}


void SPI_mode0_4MHz(void)
{
	SPCR = 0x50;
	SPSR = 0x00;
	//0x01;
}

void SPI_mode0_slow(void)
{
	SPCR = 0x53;			// 16MHz/64 = 250kHz
	SPSR = 0x01;			// Double SPI Speed
}

void SPI_Mode0_1MHz(void)
{
	SPCR = 0x51;			// 16MHz/16 = 1MHz
	SPSR = 0x0;
}

u_char SPI_write(u_char data)
{
	SPDR = data;
	while(!(SPSR & 0x80));	// SPIF = 1 때까지(종료) 대기
	
	return SPDR;
}
/************************************************************************
MBR과 PBR sector를 차례대로 읽어 loot 디렉터리 섹터 찾기
출력:
First_FAT_Sector : FAT 시작 섹터
RootDirSector : 루트 디렉터리 섹터
SecPerClus : 클러스터당 섹터 수
************************************************************************/
void FAT_Init(void)
{
	uint32_t StartLBA;		// PBR 섹터 주소
	struct PartTable *PartTable_ptr;		// MBR sector partition table 구조체
	struct BootSector *BootSector_ptr;	// PBR sector 구조체
	
	SD_Read(MBR);						// MBR 섹터(0번섹터)를 읽어 버퍼에 저장
	
	// MBR 섹터의 첫번째 파티션 테이블을 저장
	// 446바이트는 Boot code이고, 446번부터 16바이트씩 파티션#1, #2, #3...
	PartTable_ptr = (struct PartTable*)(buffer + 446);
	
	StartLBA = PartTable_ptr->LBA_Begin;
	
	SD_Read(StartLBA);
	BootSector_ptr = (struct BootSector *)buffer;
	
	// root dir sector 계산
	First_FAT_Sector = StartLBA + BootSector_ptr->BPB_RsvdSecCnt;
	RootDirSector = First_FAT_Sector
	 + BootSector_ptr->BPB_FATSz32
	 * BootSector_ptr->BPB_NumFATs;
	 
	 // cluster 당 sector 수 저장
	 SecPerClus = BootSector_ptr->BPB_SecPerClus;
}
/************************************************************************
Directory Entry는 하나의 섹터에 16개 존재
Directory Entry는 파일의 개수만큼 생성됨
Directory Entry는 32바이트로 구성
Directory Entry는 Cluster Number(파일의 시작위치)를 가지고 있음
Directory Entry에서 Cluster Number 추출
return : 
numOfFile : 파일 수
출력(전역변수) : 
fileStartClust[num_File] : 각 파일의 시작 클러스터
FileName[] : 파일명
file_len : 파일길이 섹터 수
************************************************************************/
uint16_t fatGetDirEntry(void)
{
	struct DirEntry *DE_ptr = 0;
	uint16_t num_File = 0;			// 파일 개수
	uint16_t clusterHI;				// 디렉터리 엔트리 HI
	uint16_t clusterLO;
	uint32_t clusterNumber;			// 클러스터 넘버
	uint16_t i, j, k, flag;
	uint32_t RD_clust, RD_sector;
	
	RD_clust = 2;		// 루트 디렉터리 시작 클러스터
	
	while(1){
		RD_sector = fatClustToSect(RD_clust);
		flag = 1;
		while(flag){
			for(k = 0; k < SecPerClus; k++){
				// 루트 디렉터리 섹터 읽기
				SD_Read(RD_sector);
				// 디렉터리 entry 저장
				DE_ptr = (struct DirEntry *)buffer;
			
				// 32바이트 단위로 디렉터리 entry 16개씩 조사 
				for(j = 0; j < 16; j++){
					if(DE_ptr->DIR_Name[0] == 0x00){
						flag = 0;
						break;
					}
					if(DE_ptr->DIR_Name[0] != 0xE5 &&	//삭제 파일
					DE_ptr->DIR_Attr == 0x20 &&			// 일반 파일
					DE_ptr->DIR_Ext[0] == 'B' &&		// bmp 파일일시
					DE_ptr->DIR_Ext[1] == 'M' &&	
					DE_ptr->DIR_Ext[2] == 'P'
					){
						// clusterHI:LO 설정
						clusterHI = DE_ptr->DIR_FstClusHI;
						clusterLO = DE_ptr->DIR_FstClusLO;
						
						clusterNumber = (uint32_t)(clusterHI);	// 16bit->32bit
						clusterNumber <<= 16;
						
						// cluster 넘버 추출
						clusterNumber |= (uint32_t)(clusterLO);
						
						// 시작 cluster 저장
						fileStartClust[num_File] = clusterNumber;
						
						// 파일명 저장
						for(i = 0; i< 8; i++){
							FileName[num_File][i] = DE_ptr->DIR_Name[i];
						}
						FileName[num_File][8] = '\0';
						
						// 파일길이 섹터수(연주 종료조건으로 사용)
						file_len[num_File] = (DE_ptr->DIR_FileSize / 512) + 1;
						
						num_File++;
						if(num_File >= MAX_NUM_FILE){
							return num_File;
						}
					}
					DE_ptr++;		// 디렉터리 entry pointer 증가
				}
				RD_sector++;
			}
		}
		RD_clust = FAT_NextCluster(RD_clust);
		if(RD_clust == 0)break;
	}
	return num_File;
}
/************************************************************************
클러스터 넘버를 실제주소로 변환
첫번째 파일의 클러스터 넘버는 언제나 3번 클러스터
************************************************************************/


uint32_t fatClustToSect(uint32_t cluster)
{
	return ((cluster - 2) * SecPerClus + RootDirSector);
}
/************************************************************************
현재 cluster로부터 다음 cluster계산
Arguments	: 현재 cluster 번호
Returns		: 다음 cluster 번호
************************************************************************/

uint32_t FAT_NextCluster(uint32_t cluster)
{
	uint32_t fatOffset, sector, offset, I_off;
	uint32_t *I_buf, next_cluster;
	
	fatOffset = cluster << 2;		// 각 cluster마다 4Byte 차지
	
	// 현재의 클러스터 값으로부터 FAT 에서의 sector와 offset값 산출
	sector = First_FAT_Sector + (fatOffset / u_charPerSector);
	// 바이트 단위로 몇번째인지 계산
	offset = fatOffset % u_charPerSector;
	// 4바이트씩 나눌 때 몇 번째인지 계산
	I_off = offset >> 2;
	
	SD_Read(sector);			// sector  값 읽기
	I_buf = (uint32_t *)buffer;	// 읽은 데이터 4바이트 단위로 처리
	next_cluster = I_buf[I_off];	// next cluster값 읽기
	
	if(next_cluster == EndOfCluster) next_cluster = 0;		// 클러스터 종료
	
	return next_cluster;
}