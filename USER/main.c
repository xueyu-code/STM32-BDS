#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "usart2.h"
#include "timer.h"
#include "gizwits_product.h"
#include "math.h"
#include "bmp.h"
//声明
void errorLog(int num);
void parseGpsBuffer(void);
void printGpsBuffer(void);
int string_num(char* num);

char a[10];
int weidu=0;
int jingdu=0;
int time;
float weidu_1;
float jingdu_1;
int warn_flag=0;//区域越界标志位

//默认划定的区域是北京市的区域
//起始点的经纬度
float start_jing=115.25;
float start_wei=39.26;
//终止点的经纬度
float end_jing=117.30;
float end_wei=41.03;
//只要纬度大于起始点的纬度或者经度大于终止点的经度就是超出划定区域
float time_1;
int main(void)
{	

	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
// 	Init_LEDpin();			     //LED端口初始化
	 OLED_Init();
	 
	TIM3_Int_Init(71,999);				//一毫秒定时
    
	memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));//设备状态结构体初始化
	usart3_init((u32)9600);//WIFI初始化
	gizwitsInit();//缓冲区初始化
//	gizwitsSetMode(WIFI_RESET_MODE);
	delay_ms(500);
//	gizwitsSetMode(WIFI_AIRLINK_MODE);
	gizwitsSetMode(WIFI_SOFTAP_MODE);
//	 delay_ms(1000);

	clrStruct();
	weidu_1=3.51;
	jingdu_1=73.33;
	
	while(1)
	{
		weidu=string_num(Save_Data.latitude);
		jingdu=string_num(Save_Data.longitude);
		time=string_num(Save_Data.UTCTime);
		time+=80000;
		weidu_1=(float)weidu/100.0;
		jingdu_1=(float)jingdu/100.0;
		
		time_1=(float)time/10000.0;
		
		parseGpsBuffer();
		printGpsBuffer();
	if(warn_flag==0)
	{
//		OLED_Clear();
		OLED_ShowPicture(0,0,128,64,BMP2,1);
		OLED_ShowString(0,0,(u8*)"BDS",16,1);
		//定位系统
		OLED_ShowChinese(24,0,0,16,1);			//定位系统
		OLED_ShowChinese(40,0,1,16,1);
		OLED_ShowChinese(56,0,2,16,1);
		OLED_ShowChinese(72,0,3,16,1);
		
		OLED_ShowChinese(0,16,4,16,1);
		OLED_ShowChinese(16,16,5,16,1);
	
		sprintf(a,"%.2f",weidu_1);
		OLED_ShowString(32,16,(u8*)a,16 ,1);

		
		OLED_ShowChinese(0,32,6,16,1);
		OLED_ShowChinese(16,32,7,16,1);
		sprintf(a,"%.2f",jingdu_1);
		OLED_ShowString(32,33,(u8*)a,16 ,1);

//		OLED_Refresh();
		
		OLED_ShowChinese(0,48,8,16,1);
		OLED_ShowChinese(16,48,9,16,1);
		OLED_ShowChinese(32,48,10,16,1);
		OLED_ShowChinese(48,48,11,16,1);
		sprintf(a,"%.4f",time_1);
		OLED_ShowString(64,48,(u8*)a,16 ,1);
		OLED_Refresh();
		
	}
		
		//进行区域是否越界的判断
		if((weidu_1>start_wei)||(jingdu_1>end_jing))
		{
			warn_flag=1;
			OLED_Refresh();
			OLED_ShowPicture(0,0,128,64,BMP1,1);
			
		}
		if((weidu_1<start_wei)&&(jingdu_1<end_jing))
			warn_flag=0;
			
		userHandle();
    gizwitsHandle((dataPoint_t *)&currentDataPoint);
    
	}
}
int string_num(char* num)			//该函数将接收到的字符串转成int型
{
	int i;
	int j;
	int temp;
	i=0;
	while(num[i]!=NULL)
	{
		if(num[i]=='.')
		{
			break;
		}
		i++;
	}
	for(j=i-1;j>=0;j--)
	{
		temp+=(int)(num[j]-'0')*pow(10,i-j-1);
	}
	return temp;
}






/***********GPS模块的函数**************************/
/***********编写：2022年3月25日23:33:15************/
/*************************************************/
void errorLog(int num)
{
	
	while (1)
	{
	  	printf("ERROR%d\r\n",num);
	}
}
void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
//		printf("**************\r\n");
//		printf(Save_Data.GPS_Buffer);
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}
					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;
				}
				else
				{
					errorLog(2);	//解析错误
				}
			}
		}
	}
}

void printGpsBuffer()				//通过串口调试助手显示数据
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
//	printf("Save_Data.UTCTime = ");
//	printf(Save_Data.UTCTime);
//	printf("\r\n");

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
//			printf("Save_Data.latitude = ");
//			printf(Save_Data.latitude);
//			printf("\r\n");


//			printf("Save_Data.N_S = ");
//			printf(Save_Data.N_S);
//			printf("\r\n");

//			printf("Save_Data.longitude = ");
//			printf(Save_Data.longitude);
//			printf("\r\n");

//			printf("Save_Data.E_W = ");
//			printf(Save_Data.E_W);
//			printf("\r\n");
		}
		else
		{
//			printf("GPS DATA is not usefull!\r\n");
		}
		
	}
}
/**********************************/
/**********************************/
/***********GPS模块的函数***********/





