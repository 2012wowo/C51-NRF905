#include<reg51.h>

#define WC		0x00
#define RC		0x10
#define WTP		0x20
#define RTP		0x21
#define WTA		0x22
#define RTA		0x23
#define RRP		0x24
#define BYTE_BIT0	0x01
#define uchar unsigned char
#define uint unsigned int
unsigned char idata RFConf[11]=
{
  0x00,                             //配置命令//
  0x4c,                             //CH_NO,配置频段在423MHZ
  0x0c,                             //输出功率为10db,不重发，节电为正常模式
  0x44,                             //地址宽度设置，为4字节
  0x03,0x03,                        //接收发送有效数据长度为3字节
  0xCC,0xCC,0xCC,0xCC,              //接收地址
  0x58,                             //CRC充许，8位CRC校验，外部时钟信号不使能，16M晶振
};
unsigned char rx_Buffer[]={0};          //用于存放接收到的数据
uchar position[3]={0xFE,0xFC,0xFB};
bdata unsigned  char DATA_BUF;
sbit flag=DATA_BUF^7;

sbit AM=P3^7;
sbit CD=P3^6;
sbit CSN=P1^0;
sbit SCK=P1^1;
sbit MISO=P1^2;
sbit MOSI=P1^3;
sbit DR=P1^4;
sbit PWR=P1^5;
sbit TRX_CE=P1^6;
sbit TXEN=P1^7;
sbit send_reset =P3^4;

sbit lcden=P3^3;		//LCD使能端接的哪个端口就写哪个
sbit lcdrs=P3^2;		//LCD数据命令选择端


uint temp;
//---------------延时--------------//
void Delay(unsigned char n)
{
	unsigned int i;
	while(n--)
	for(i=0;i<80;i++);
}

void Delay_ms(uchar n)
{ 
	unsigned int i,j,k;

	for(i=0;i<n;i++)
	{
		for(k=0;k<5;k++)
		{
			for(j=0;j<200;j++);	
		}
	}
}

void Delay_us(uchar n)
{
	uint i;
	for(i=0;i<n;i++);
}



//------------SPI写操作代码---------//

void SpiWrite(unsigned char send)
{
	unsigned char i;
	DATA_BUF=send;
	for (i=0;i<8;i++)
	{
		if (flag)	//总是发送最高位
		{
			MOSI=1;
		}
		else
		{
			MOSI=0;
		}
		SCK=1;
		DATA_BUF=DATA_BUF<<1;
		SCK=0;
	}
}


//------------SPI读操作代码---------//
unsigned char SpiRead(void)
{
	unsigned char j;
	for (j=0;j<8;j++)
	{
        DATA_BUF=DATA_BUF<<1;
		SCK=1;
		if (MISO)	//读取最高位，保存至最末尾，通过左移位完成整个字节
		{
			DATA_BUF|=BYTE_BIT0;
		}
		else
		{
			DATA_BUF&=~BYTE_BIT0;
		}
		SCK=0;
	 }
	 return DATA_BUF;
}
//-----------resive 状态设置------------------//
void SetRxMode(void)
{
	TXEN=0;
	TRX_CE=1;
	Delay(1); 					// delay for mode change(>=650us)
}
//-------------初始化设置---------------//
void nRF905Init(void)
{
    CSN=1;						// Spi 	disable
	SCK=0;						// Spi clock line init low
	DR=1;						// 激活单片机IO
	AM=0;						// Init AM for input
	CD=0;						// Init CD for input
	PWR=1;					    // nRF905 power on
	TRX_CE=0;					// Set nRF905 in standby mode
	TXEN=0;					    // set radio in Rx mode
}


//----------设置配置寄存器-------//
void Config905(void)
{
	unsigned char i;
	CSN=0;						
	//SpiWrite(WC);				
	for (i=0;i<11;i++)	
	{
	   SpiWrite(RFConf[i]);
	}
	CSN=1;					
}

//--------------读取RF905接收数据------------//
void RxPacket(void)						
{
	unsigned char i;
    Delay(1);
//	TRX_CE=0;					// Set nRF905 in standby mode
    Delay(100);
    TRX_CE=0;
	CSN=0;						// Spi enable for write a spi command
    Delay(1);
	SpiWrite(RRP);
	for (i = 0 ;i < 3 ;i++)
	    { 
		 rx_Buffer[i]=SpiRead();		// Read data and save to buffer       
	     }
       	CSN=1;
     Delay(100);
	 Delay(100);
     Delay(10);
	 Delay_ms(1000); 
	 TRX_CE=1;							
}
//-------检查是否有新数据传入-----------//
unsigned char CheckDR(void)		//检查是否有新数据传入 Data Ready
{
	if (DR)                    //当数据完全接收DR为1
	{
      
		return 1;
	}
	else
	{
		return 0;
	}
}


//------------------------------数据接收------------------//
void  RX(void)
{
          SetRxMode();			      // Set nRF905 in Rx mode
          while (CheckDR()==0);       //RX wait
		  Delay(100);
	  	  RxPacket();
		  Delay_ms(1000);  
		 
		  Delay_ms(1000); 
}
     	
 /*******************************************************************************************
函数名：write_com
调  用：write_com(com)
参  数：uchar com
返回值：
结  果：
注  释：写命令函数，设置液晶光标显示/不显示、光标闪烁/不闪烁、需不需要移屏、在什么位置显示，
	   具体命令数值，参考数据手册
*******************************************************************************************/
void write_com (uchar com)
{
	lcdrs=0;		//选择写命令模式
	P0=com;			//把命令送到P0口，连接的哪个口，写哪个口
	Delay_ms(5);		//看时序图，可以得出，写程序需要时间
	lcden=1;		//液晶屏使能端
	Delay_ms(5);		
	lcden=0;
}

 /*******************************************************************************************
函数名：write_data
调  用：write_data(data)
参  数：uchar data
返回值：
结  果：
注  释：写数据函数，显示什么内容
*******************************************************************************************/
void write_data(uchar date)
{
	lcdrs=1;//选择写数据模式
	P0=date;//把命令送到P0口，连接的哪个口，写哪个口
	Delay_ms(5);//看时序图，可以得出，写程序需要时间
	lcden=1;//液晶屏使能端
	Delay_ms(5);		
	lcden=0;
}

/*******************************************************************************************
函数名：LCD初始化
调  用：init
参  数：
返回值：
结  果：
注  释：初始化LCD
*******************************************************************************************/
void init()
{
	lcden=0;
	write_com(0x38);	//初始化设置（1）：显示模式设置。0011 1000，编码代表什么功能，查手册，想实现什么功能，写什么编码
	write_com(0x0c);	//初始化设置（2）：显示开关及光标设置。0000 1100，编码代表什么功能，查手册，想实现什么功能，写什么编码
	write_com(0x06);	//
	write_com(0x01);	//其他设置

}
/*******************************************************************************************
函数名：把接收的数据转换成温度
调  用：get_RXtemp()
参  数：
返回值：
结  果：
注  释：
*******************************************************************************************/
uint get_RXtemp()
{
	uchar a,b;
	a=rx_Buffer[0];
	b=rx_Buffer[1];
	temp=b;
	temp<<=8;
	temp=temp|a;	//
	return temp;
}

/*******************************************************************************************
函数名：显示温度数据
调  用：write_temp_to_lcd(add,dat)
参  数：uchar add,uchar dat
返回值：
结  果：
注  释：在那里显示，显示什么内容
*******************************************************************************************/
void write_temp_to_lcd(uchar add,uchar dat)
{
	int i,j,k;
	i=dat/100+'0';
	j=dat%100/10+'0';
	k=dat%100%10+'0';
 	write_com(0x80+add);
//	write_data(init_display3[i]);
//	write_data(init_display3[j]);
	write_data(i);
	write_data(j);
	write_data(k);
	Delay_ms(5);
}


//-------------主函数---------------// 
void main()
 {
 //P0=0X00;
  P2^0=1;
  Delay_ms(100); 
  nRF905Init();
  Config905();
  Delay_ms(5);
  init();		//LCD初始化
  while(1)
    {           
        RX();
		Delay_ms(5);
		get_RXtemp();
	 	init();		//LCD初始化
		write_com(0x0c);
		write_temp_to_lcd(0x80,temp);	
		Delay_ms(100); 
		P2^0=0; 
     }

}