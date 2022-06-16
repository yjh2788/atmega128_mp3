#ifndef FAT32.h
#define FAT32.h
#include "C:\Users\spc\Desktop\microprocessor\spi.h"
#include "C:\Users\spc\Desktop\microprocessor\global.h"
#include "C:\Users\spc\Desktop\microprocessor\TFT.h"

#include "C:\Users\spc\Desktop\microprocessor\SD_card.h"
//#include "C:\Users\spc\Desktop\microprocessor\SD_FAT32.h"
#include "C:\Users\spc\Desktop\microprocessor\vs1003b.h"

// FAT32 정의
#define MBR               0              // Master Boot Record(SD 카드의 0번섹터)
#define EndOfCluster        0x0FFFFFFF       // 클러스터의 끝
#define u_charPerSector       512             // 512바이트

#define MAX_NUM_FILE    100              // 파일의 최대 개수 설정

typedef  unsigned char u_char;
typedef  unsigned int u_int;
typedef  unsigned long u_long;

void    SD_Read(u_long sector); 
// SD Card FAT 관련
void    FAT_Init(void);                            // 루트 디렉터리 추출
u_int   fatGetDirEntry(void);                       // MP3 파일 개수
u_long  fatClustToSect(u_long cluster);           // 현재 cluster의 시작 Sector 계산
u_long  FAT_NextCluster(u_long cluster);         // 현재 cluster로부터 다음 cluster계산

// MP3 연주 관련
void    MP3_Play(int);                              // MP3 플레이 출력
void    FName_display(u_int n_pg);                  // n_pg 페이지 파일명 표시
void    next_play(void);
/********************************************************************************
  변수 및 구조체 선언
********************************************************************************/
// MBR 섹터에 있는 파티션 테이블 구조
struct PartTable
{
    u_char        Bootable;            // 446    0x1BE
    u_char        StartHead;        // 447    0x1EF
    u_int        StartCylSec;        // 448    0x1C0
    u_char        Type;            // 450    0x1C2
    u_char        EndHead;            // 451    0x1C3
    u_int        EndCylSec;        // 452    0x1C4
    u_long        LBA_Begin;        // 454    0x1C6
    u_long        Size;            // 458    0x1CA
};

// PBR(Partition Boot Record) 섹터의 전체 구조
struct BootSector
{
    u_char        BS_jmpBoot[3];        // 0    0x00
    char        BS_OEMName[8];        // 3    0x03
    u_int        BPB_u_charsPerSec;    // 11    0x0B
    u_char        BPB_SecPerClus;        // 13    0x0D
    u_int        BPB_RsvdSecCnt;        // 14    0x0E
    u_char        BPB_NumFATs;        // 16    0x10
    u_int        BPB_RootEntCnt;        // 17    0x11
    u_int        BPB_TotSec16;        // 19    0x13
    u_char        BPB_Media;        // 21    0x15
    u_int        BPB_FATSz16;        // 22    0x16
    u_int        BPB_SecPerTrk;        // 24    0x18
    u_int        BPB_NumHeads;        // 26    0x1A
    u_long        BPB_HiddSec;        // 28    0x1C
    u_long        BPB_TotSec32;        // 32    0x20

    u_long        BPB_FATSz32;        // 36    0x24
    u_int        BPB_ExtFlags;        // 40    0x28
    u_int        BPB_FSVer;        // 42    0x2A
    u_long        BPB_RootClus;        // 44    0x2C
    u_int        BPB_FSInfo;        // 48    0x30
    u_int        BPB_BkBootSec;        // 50    0x32
    u_char        BPB_Reserved[12];    // 52    0x34
    u_char        BS_DrvNum;        // 64    0x40
    u_char        BS_Reserved1;        // 65    0x41
    u_char        BS_BootSig;        // 66    0x42
    u_long        BS_VolID;        // 67    0x43
    char        BS_VolLab[11];        // 71    0x47
    char        BS_FilSysType[8];    // 82    0x52
    u_char        boot_code[422];        // 90    0x5A

};

// 디렉터리 엔트리 구조(32u_chars)
// 도스의 8u_chars 파일명과 3u_chars 확장자 구조를 그대로 유지
struct DirEntry
{
    u_char        DIR_Name[8];        // 00    0x00
    u_char        DIR_Ext[3];        // 00    0x00
    u_char        DIR_Attr;            // 11    0x0B
    u_char        DIR_NTRes;        // 12    0x0C
    u_char        DIR_CrtTimeTenth;        // 13    0x0D
    u_int        DIR_CrtTime;        // 14    0x0E
    u_int        DIR_CrtDate;        // 16    0x10
    u_int        DIR_LastAccDate;        // 18    0x12
    u_int        DIR_FstClusHI;        // 20    0x14
    u_int        DIR_WrtTime;        // 22    0x16
    u_int        DIR_WrtDate;        // 24    0x18
    u_int        DIR_FstClusLO;        // 26    0x1A
    u_long        DIR_FileSize;        // 28    0x1C
};

u_long  fileStartClust[MAX_NUM_FILE];         // 파일의 시작 클러스터 저장
u_long  file_len[MAX_NUM_FILE];               // 파일 길이(섹터수)
u_long  RootDirSector;                          // 루트 디렉터리 섹터
u_int   numOfFile;                              // 파일의 총 개수
u_long  First_FAT_Sector;                        // FAT 시작 섹터
u_char  SecPerClus;                       // 클러스터당 섹터수
u_char  sd_type;                              // SD 카드 버전
u_char  FileName[MAX_NUM_FILE][9];            // 파일명(쇼트 디렉터리 영역)
u_char  buffer[512];                            // SD 카드로부터 읽은 데이터 저장

u_char  stop_play = 0;                    // 0 : 정지, 1 : 연주
u_char  repeat = 0;                        // 순차연주(0)/반복연주(1)
u_int   sel_pg = 1;                         // 표시 페이지(1~tot_pg)
u_int   sel_no = 0;                          // 선택 번호
u_int   sel_tno;                            // 선택할 수 있는 파일 수
u_int   tot_pg;                             // 전체 페이지수
u_int   tot_no;                              // 마지막 페이지 표시할 (파일 수 - 1)
u_int   play_no = 0;                        // 연주곡 번호
u_char sbuf[20];

//*****************************************************************************
// 함수
//****************************************************************************

void FAT_Init(void)
{
	u_long  StartLBA;                       // PBR 섹터 주소
	struct  PartTable *PartTable_ptr;         // MBR 섹터에 있는 파티션 테이블 구조체
	struct  BootSector *BootSector_ptr;       // PBR 섹터 구조체
		TFT_string(0,27,Green,  Black,"******************************");
		TFT_string(0,29,Magenta,Black,"         FAT32 initialize...  ");
		TFT_string(0,31,Green,  Black,"******************************");
        

        SD_Read(MBR);                // MBR 섹터(0번섹터)를 읽어 버퍼에 저장

    // MBR 섹터의 첫 번째 파티션 테이블을 저장
      PartTable_ptr = (struct PartTable*)(buffer + 446);


        StartLBA = PartTable_ptr->LBA_Begin;        // PBR 섹터주소

        SD_Read(StartLBA);                            // PBR 섹터 읽기
        BootSector_ptr = (struct BootSector *)buffer;       // PBR 섹터를 구조체에 저장

        // 루트디렉터리 섹터 계산
        First_FAT_Sector = StartLBA + BootSector_ptr->BPB_RsvdSecCnt;
        RootDirSector = First_FAT_Sector +
        BootSector_ptr->BPB_FATSz32 * BootSector_ptr->BPB_NumFATs;
		TFT_hexadecimal(StartLBA,2);
		TFT_English(' ');
		TFT_hexadecimal(RootDirSector,2);
		TFT_English(' ');
        // 클러스터당 섹터수 저장, SD 카드 용량에 따라 다름
        SecPerClus = BootSector_ptr->BPB_SecPerClus;
		TFT_hexadecimal(SecPerClus,2);
		TFT_English(' ');
        TFT_string(8,35,Green,Black,"initialized!");
}

/********************************************************************************
 Description : Directory Entry는 하나의 섹터(512 u_chars)에 16개가 있다.
               Directory Entry는 파일의 개수만큼 생성된다.
               Directory Entry는 32u_chars로 구성되어 있다.
               Directory Entry는 Cluster Number(파일의 시작위치)를 가지고 있다.
               Directory Entry에서 Cluster Number만 추출한다.
 Returns     : numOfFile(MP3파일 수)
********************************************************************************/
u_int fatGetDirEntry()
{
        struct DirEntry *DE_ptr = 0;                // 디렉터리 엔트리
        u_int       num_File = 0;                  // 파일 개수
        u_int       clusterHI;             // 디렉터리 엔트리의 Cluster High
        u_int       clusterLO;             // 디렉터리 엔트리의 Cluster Low
        u_long      clusterNumber;               // 클러스터 넘버
        u_int       i, j, k, flag;
        u_long      RD_clust, RD_sector;

        RD_clust = 2;                            // 루트 디렉터리 시작 클러스터

        while(1){
			
            RD_sector = fatClustToSect(RD_clust);
			TFT_string(0,40,Cyan,Black,"k");
            flag = 1;
            while(flag){TFT_string(0,40,Cyan,Black,"1");
				
                for(k = 0;k < SecPerClus;k++)
				{
                    SD_Read(RD_sector);              // 루트디렉터리 섹터 읽기
                    DE_ptr = (struct DirEntry *)buffer;    // 디렉터리엔트리 저장

                    // 32바이트 단위 디렉터리 엔트리 16개씩 조사
					
                   for(j = 0;j < 16;j++){
                        if(DE_ptr->DIR_Name[0] == 0x00) {    // 미사용
                                flag = 0;
                                break;
                        }
					TFT_string(10,40,Cyan,Black,"4");
                        if(DE_ptr->DIR_Name[0] != 0xE5 &&    // 삭제 파일
                           DE_ptr->DIR_Attr == 0x20 &&    // 일반 파일
                           DE_ptr->DIR_Ext[0] == 'M' &&      // MP3 파일이면
                           DE_ptr->DIR_Ext[1] == 'P' &&
                           DE_ptr->DIR_Ext[2] == '3')
                        {
                 // clusterHI:LO 설정
                                clusterHI = DE_ptr -> DIR_FstClusHI;
                                clusterLO = DE_ptr -> DIR_FstClusLO;

                                clusterNumber = (u_long)(clusterHI);  // 16bit->32bit 변환
                                clusterNumber <<= 16;

                  // 클러스터 넘버 추출
                                clusterNumber |= (u_long)(clusterLO);

                 // 시작클러스터 저장
                                fileStartClust[num_File] = clusterNumber;
                                TFT_string(20,40,Cyan,Black,"a");
                                // 파일명 저장
                                for(i = 0;i < 8;i++)
                                FileName[num_File][i] = DE_ptr->DIR_Name[i];
                                FileName[num_File][8] = '\0';

                 // 파일길이 섹터수(연주 종료조건으로 사용)
                                file_len[num_File] = (DE_ptr->DIR_FileSize / 512) + 1;

                                num_File++;
                                if(num_File >= MAX_NUM_FILE) {
                                        return num_File;
                                }
                         }//if
                         DE_ptr++;       // 디렉터리 엔트리 포인터 증가
                     }//for
                     RD_sector++;
                     TFT_string(30,40,Cyan,Black,"c");
                }//for
            }//while
            TFT_string(40,40,Cyan,Black,"b");
            RD_clust = FAT_NextCluster(RD_clust);
            if(RD_clust == 0) break;
        }//while
        return num_File;
}
/********************************************************************************
 Description : 클러스터 넘버를 실제 섹터주소로 변환
               Cluster Number -> Physical Sector Address
               첫번째 파일의 클러스터 넘버는 언제나 3번 클러스터
 Arguments   : cluster
  Returns     : physical sector address
********************************************************************************/
u_long fatClustToSect(u_long cluster)
{
    return ((cluster - 2) * SecPerClus) + RootDirSector;
}

/******************************************************************************
 Description : 현재 cluster로부터 다음 cluster계산
 Arguments   : 현재 cluster 번호
  Returns     : 다음 cluster 번호
********************************************************************************/
u_long  FAT_NextCluster(u_long cluster)
{
        u_long  fatOffset, sector, offset, l_off;
        u_long  *l_buf, next_cluster;

        fatOffset = cluster << 2;         // 각 클러스터마다 4바이트 차지

        // 현재의 클러스터 값으로부터 FAT에서의  sector와 offset값 산출
        sector = First_FAT_Sector + (fatOffset / u_charPerSector);
        offset = fatOffset % u_charPerSector;    // 바이트 단위로 몇번째인지 계산
        l_off = offset >> 2;                // 4바이트씩 나눌 때 몇번째인지 계산

        SD_Read(sector);                      // sector 값 읽기
        l_buf = (u_long *)buffer;               // 읽은 데이터 4바이트 단위로 처리
        next_cluster = l_buf[l_off];            // next cluster값 읽기

        if(next_cluster == EndOfCluster) next_cluster = 0;        // 클러스터 종료

        return next_cluster;
}
//***********************************************************************************
//FAT끝
//********************************************************************************

//////***********************************연주
void MP3_Play(int f_no)
{
        u_long  c_clust;
        u_long  rd_sec;
        int     i, n;
        u_long  k = 0, kd;

        kd = file_len[f_no];                            // 파일 길이 섹터수
        c_clust = fileStartClust[f_no];                 // f_no 파일 시작 클러스터
        while(1){
                rd_sec = fatClustToSect(c_clust);    // 현재 클러스터를 섹터로 변환
                for(n = 0;n < SecPerClus;n++){        // 클러스터당 섹터 수만큼 읽기
                        SD_Read(rd_sec);         // 섹터(512바이트) 읽기

                        // 읽은 데이터 VS1033칩에 출력
                        for(i = 0;i < 512;i++){
                            // VS1033 데이터수신가능 체크
                            while(!(vs1003b_is_busy()));

                            vs1003b_clear_dcs();        // xDCS = low, VS1033 활성화
                            SPI_char(buffer[i]);       // VS1033 칩으로 MP3 데이터 전송
                            vs1003b_set_dcs();       // xDCS = high, VS1033 비활성화
                        }
                        k++;
                        if(k >= kd) return;

                        rd_sec++;                     // 섹터 번호 +1
                        if(stop_play == 0) return;    // 연주 정지
                }
                c_clust = FAT_NextCluster(c_clust);    // 다음 cluster계산
                if(c_clust == 0) break;                // 마지막 클러스터이면 종료
        }

        TFT_color_screen(Black);
        TFT_string(0,0,Cyan,Black,"it is playing");
}

void next_play(void)
{
        if(sel_no == sel_tno){
                sel_no = 0;
                if(sel_pg == tot_pg) sel_pg = 1;
                else sel_pg++;
        }
        else {     
                sel_no++;            
        }

        play_no = (sel_pg - 1) * 8 + sel_no;
}


void SD_Read(u_long sector)
{
        u_int   i;
        
        SPI_Mode0_4MHz();
TFT_string(0,33,Green,  Black,"******************************1");
    // High Capacity 카드가 아니면 섹터번호에 512를 곱한다.
        if(sd_type == VER1X || sd_type ==VER2X_SC) sector = sector << 9;
        SD_command(CMD17, sector);     // CMD 17(읽기명령) + 읽어올 섹터번호 전송
        while(SPI_char(0xFF) != 0x00);     // R1 정상 응답 대기
        TFT_string(0,35,Green,  Black,"******************************2");
        while( SPI_char(0xFF) != 0xFE );    // Start Block Token = 0xFE 수신하면 탈출
TFT_string(0,37,Green,  Black,"******************************3");
        for(i=0; i<512; i++) buffer[i] = SPI_char(0xFF);    // 데이터 수신시작 (512 u_chars)

        // CRC 수신 (2 u_chars)
        SPI_char(0xFF);
        SPI_char(0xFF);

        SPI_char(0xFF);                // 추가

        sbi(PORTB,CS_SD);                     // SD_CS = high (SD 카드 비활성화)
        SPI_Mode0_8MHz();
}
#endif