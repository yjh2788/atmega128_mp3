/*
 * GccApplication2.cpp
 *
 * Created: 2022-05-26 오전 12:01:20
 * Author : spc
 */ 


#include <avr\io.h>
#include <avr\interrupt.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#define pwm 0x0200
extern "C"	// C로 만든 파일들의 헤더를 넣는다.
{

	#include "pin_magic.h"
	#include "registers.h"
	#include "TFT.h"
	#include "TFT_font.h"
	#include "SDcard.h"
	#include "spi.h"
	#include "vs1003b.h"
	#include "global.h"

	
	#include "Universial_Parameter.h"
};
//////-----------------------------------------------------화면-----------------------------
void setting_page(void);
void menu_page(void);
void play_page(void);
void list_page(void);
void booting_page(void);
void start_page(void);
void reset_flags(void);





///-----------------------------------------------------------------------------------------
void    MP3_Play(int);                              // MP3 플레이 출력
void initial_mcu(void);
void set_vol(bool flag);


//void    FName_display(unsigned int n_pg);                  // n_pg 페이지 파일명 표시

//long size = (long)TFTWIDTH * (long)TFTHEIGHT;
bool stop_flag=0;
bool volume_flag=0;
bool change_flag=0;
bool home_flag=0;
int music_no=0;
bool play_again=0;
 bool p_flag=0;
bool up=0,down=0,select=0;

bool play_state=0;

ISR(INT0_vect)//이전곡//위
{

	if(music_no==0) music_no=numOfFile-2;
	else music_no=music_no-2;
	change_flag=1;
	up=1;
	
}
ISR(INT2_vect)//재생 정지/ 선택
{
	if(play_state)	stop_flag=(stop_flag==1) ?0:1;
	select=1;
}
ISR(INT1_vect)//다음곡//아래
{
	if(music_no==numOfFile-1) music_no=0;
	music_no++;
	change_flag=1;
	down=1;
	
}
ISR(INT6_vect)//볼륨 증가
{
	
	 volume_flag=1;
	 
	if(volume<20) volume=0;
	else volume-=10;


	
}
ISR(INT3_vect)//볼륨 감소
{
	volume_flag=1;
	
	if(volume>235) volume=254;
	else volume+=10;

	
}
ISR(INT4_vect)//
{
	//play_again=1;
	if(play_again==1) play_again=0;
	else play_again=1;
	p_flag=1;
	
}
ISR(INT7_vect)//홈버튼
{
	home_flag=1;
	
}





int main(void)
{
	
	 u_long  c_clust;
	 u_long  rd_sec;
	 int     i, n, format;
	 u_long  k = 0, kd;
	
	initial_mcu();

	
	cbi(PORTB,4);
	sbi(PORTB,5);
	sbi(PORTE,3);
	sbi(PORTF,0);
	
	TFT_init();
	flood(BLACK, size);
	vs1003b_set_dcs();
	vs1003b_set_cs();
	sbi(PORTB,SD_CS);
	//while(1);
	frame();
	SD_Init();
	FAT_Init();
	vs1003b_init();
	numOfFile = fatGetDirEntry();
	TFT_string(0,250,Cyan,Black,"numof file");
	TFT_hexadecimal(numOfFile,2);
	stop_play=stop_flag;
	_delay_ms(500);
	
    while (1) 
    {
		//for(int i=0;i<numOfFile;i++)		MP3_Play(i);  
		menu_page();
    }
	 
}
//-------------------------------------------------------------------------------
//description:mcu초기화
//외부인터럽트0,1,2,3,4,6,7사용
//--------------------------------------------------------------------------------
void initial_mcu()
{
	DDRA = 0xFF;			// 포트 A 출력 설정
	DDRB = 0b10110111;			// 포트 B 출력 설정
	DDRC = 0xFF;			// 포트 C 출력 설정
	DDRF=0x01;
	DDRE=0x2f;
	DDRD=0xf0;
	EIMSK=0b11011111;
	EICRA=0xff;
	EICRB=0xff;
	TCCR1A=0b00001011;
	TCCR1B=0x05;
	TCCR1C=0x0;
	TCNT1=0x0;
	OCR1CH=pwm>>8;
	OCR1CL=pwm;
	
	sei();

	
}

/********************************************************************************
 Description : MP3 연주
 인수        : 파일 번호
********************************************************************************/

void MP3_Play(int f_no)
{
	SPCR = 0x50;
	SPSR = 0x01;
	play_state=1;
	stop_flag=0;
	//TFT_color_screen(Black);
	//frame();
	//TFT_string(0,0,Cyan, Black,"it is playing");
	//TFT_string(0,30,Cyan,Black,(char*)FileName[f_no]);
        u_long  c_clust;
        u_long  rd_sec;
        int     i, n, format;
        u_long  k = 0, kd;

        kd = file_len[f_no];                            // 파일 길이 섹터수
        c_clust = fileStartClust[f_no];                 // f_no 파일 시작 클러스터
        while(1)
		{
             rd_sec = fatClustToSect(c_clust);    // 현재 클러스터를 섹터로 변환
             for(n = 0;n < SecPerClus;n++)		// 클러스터당 섹터 수만큼 읽기
             {
	             SD_Read(rd_sec);			// 섹터(512바이트) 읽기
	             
	             // 읽은 데이터 VS1033칩에 출력
	             for(i = 0;i < 512;i++)
	             {
		             // VS1033 데이터수신가능 체크
		             while((vs1003b_is_busy()));
		             vs1003b_clear_dcs();//VS1033_xDCS = 0;     // xDCS = low, VS1033 활성화
		             SPI_write(buffer[i]);       // VS1033 칩으로 MP3 데이터 전송
		             vs1003b_set_dcs();//VS1033_xDCS = 1;      // xDCS = high, VS1033 비활성화
					 set_vol(volume_flag);
					 if(change_flag) {change_flag=0; return; }
				 }
	             k++;
	             if(k >= kd) return;

	             rd_sec++;                     // 섹터 번호 +1
	             //if(stop_play == 0) return;    // 연주 정지
				 if(home_flag==1) return;
				 
				 if(stop_flag==1)
				 {
					  while(1)
					  {
						  TFT_string_size(100,22,Magenta,Black," resume ",2,2);
						  if(stop_flag==0)
						  {
							  TFT_string_size(100,22,Magenta,Black," playing ",2,2);
							  break;
						  }
					  }
				 }
	
				 if((p_flag==1)&&(play_again)) 
				 {
					 TFT_string_size(100,230,Magenta,Black," repeat ",2,2);
					 p_flag=0;
				}
				if((p_flag==1)&&(!play_again))
				{
					TFT_string_size(100,230,Magenta,Black,"            ",2,2);
					p_flag=0;
					
				}
				
				
				
	             
             }
             c_clust = FAT_NextCluster(c_clust);    // 다음 cluster계산
             if(c_clust == 0) break;                   // 마지막 클러스터이면 종료
        }
		

}

void set_vol(bool flag)
{
	if(flag)
	{
		volume_flag=0;
		vs1003b_set_volume(volume,volume);	
	
	}
	
}
//------------------------------------------------페이지--------------------------------
void setting_page(void)
{
	TFT_clear_screen();
	frame();
	reset_flags();
	TFT_string_size(120,120,Black,Cyan," volume ",2,2);
	char n100,n10,n1,v;
	char vol[4];
	
	float ratio=15;
	while(1)
	{
		v=254-volume;
		n100=v/100;
		n10=(v%100)/10;
		n1=v%10;
		vol[0]=n100+30;
		vol[1]=n10+30;
		vol[2]=n1;
		vol[3]='%';		
		TFT_xy(120,190);
		//TFT_hexadecimal(volume,2);
		TFT_unsigned_decimal(volume,0,3);
		/*for(int i=0;i<20;i++)
		{
			drawSLine(50,220+i,(volume%12)*ratio,BLUE);
			
		}
		if(volume_flag==1)
		{
			volume_flag=0;
			for(int i=0;i<20;i++)
			{
				drawSLine(50,220+i,(volume%12)*ratio,Black);
				
			}
			
		}*/
	
		
		if(home_flag)
		{
			home_flag=0;
			break;
			
		}
		
	}
	reset_flags();
	
	
}

void menu_page(void)
{
	while(1)
	{
		reset_flags();
		bool loc=1;
		TFT_color_screen(Black);
		frame();
		TFT_string_size(5,5,Green,  Black,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * " , 1 , 1);
		TFT_string_size(5,22,Magenta,Black,"		  		 menu 		           ",2,2);
		TFT_string_size(5,44,Green,  Black,"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  ",1,1);
		

		TFT_string_size(100,400,Black,Green," music ",2,2);
		TFT_string_size(100,200,Black,WHITE," setting ",2,2);
		
		while(1)
		{
			//TFT_hexadecimal(select,2);
			if(loc)//위에 있을때
			{
				if(up) up=0;
				else if(down)
				{
					down=0;
					loc=0;
					TFT_string_size(100,400,Black,WHITE," music ",2,2);
					TFT_string_size(100,200,Black,Green," setting ",2,2);
					
					
				}
				if(select)
				{
					select=0;
					list_page();///노래 목록으로 이동
					
					break;
				}
			}
			else//아래 있을때
			{
				if(down) down=0;
				else if(up)
				{
					down=0;
					loc=1;
					TFT_string_size(100,400,Black,Green," music ",2,2);
					TFT_string_size(100,200,Black,WHITE," setting ",2,2);
					
				}
				if(select)
				{
					select=0;
					setting_page();
					
					break;
				}
			}
		}
	}
	
}
void play_page(void)
{
	reset_flags();
	//char f_no=music_no;
	music_no=0;
	char f_next=music_no+1;
	char f_prev=numOfFile-1;
	
	select=1;
	TFT_clear_screen();
	frame();
	TFT_string_size(5,5,Green,  Black,"*************************************************************",1,1);
	TFT_string_size(100,22,Magenta,Black,"playing",2,2);
	TFT_string_size(5,44,Green,  Black,"*************************************************************",1,1);

	TFT_string_size(10,93,Magenta,Black,"music now:",2,2);
	
	TFT_string_size(140,93,Magenta,Black,(char*)FileName[music_no],2,2);
	
	TFT_string_size(50,170,Magenta,Black,"next:",2,2);
	TFT_string_size(140,170,Magenta,Black,(char*)FileName[f_next],2,2);
	
	TFT_string_size(10,210,Magenta,Black,"previous:",2,2);
	TFT_string_size(140,210,Magenta,Black,(char*)FileName[f_prev],2,2);
	

	while(1)
	{
		if(select==1)
		{
			while(1)
			{
				
				MP3_Play(music_no);
				music_no++;
				if(play_again==1)	music_no--;
				if(music_no==numOfFile) music_no=0;
				
				f_next=(music_no==numOfFile-1)? 0:music_no+1;
				f_prev=(music_no==0)?	numOfFile-1:music_no-1;
				
				//TFT_string_size(100,93,Magenta,Black,"                                          ",2,2);
				//TFT_string_size(100,170,Magenta,Black,"                                          ",2,2);
				//TFT_string_size(100,210,Magenta,Black,"                                          ",2,2);
				TFT_string_size(140,93,Magenta,Black,(char*)FileName[music_no],2,2);
				TFT_string_size(140,170,Magenta,Black,(char*)FileName[f_next],2,2);
				TFT_string_size(140,210,Magenta,Black,(char*)FileName[f_prev],2,2);
				
				
				if(home_flag)
				{
					home_flag=0;
					return;
				}
			}
		}
		
	}
	
	play_state=0;
	reset_flags();	 
		 
	
}
void list_page(void)
{
	
	int w=20,pos=70;
	reset_flags();
	TFT_clear_screen();
	frame();
	TFT_string_size(5,5,Green,  Black,"*************************************************************",1,1);
	TFT_string_size(120,22,Magenta,Black,"musics",2,2);
	TFT_string_size(5,44,Green,  Black,"*************************************************************",1,1);

	for(int i=0;i<numOfFile-1;i++)
	{
		TFT_string_size(5,pos+w*i,WHITE,Black,(char*)FileName[i],1,1);
		
	}
	while(1)
	{
		if(select)
		{
			select=0;
			play_page();
			break;
		}
		if(home_flag)
		{
			home_flag=0;
			break;			
		}	
	}
	reset_flags();
	
}
void booting_page(void)
{
	
}
void start_page(void)
{
	
	
}

void reset_flags(void)
{
	stop_flag=1;
	volume_flag=0;
	change_flag=0;
	home_flag=0;
	play_again=0;
	up=0;
	down=0;
	select=0;
}