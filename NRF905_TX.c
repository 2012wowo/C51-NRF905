#include<reg52.h>

#define WC		0x00
#define RC		0x10
#define WTP		0x20
#define RTP		0x21
#define WTA		0x22
#define RTA		0x23
#define RRP		0x24
//#define BYTE_BIT0	0x01
#define uchar unsigned char
#define uint  unsigned int
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

//unsigned char tempreture[9]={0X00,0X01,0X42,0X22,0X34,0X28,0X08,0X33,0X00,0X20};//预发送数据码
unsigned char send_bruff[2]={0X01,0X01};
unsigned char TxAddress[4]={0xcc,0xcc,0xcc,0xcc};       //地址码
//unsigned char rx_bruff;    
bdata unsigned  char DATA_BUF;             
sbit flag=DATA_BUF^7;

sbit AM=P3^7;	 //	 8
sbit CD=P3^6;	 //	 7
sbit CSN=P1^0;	 //13
sbit SCK=P1^1;	 //	12
sbit MISO=P1^2;	 //	10
sbit MOSI=P1^3;	 //	11
sbit DR=P1^4;	 //	9
sbit PWR=P1^5;	 //	5
sbit TRX_CE=P1^6;//	4
sbit TXEN=P1^7;	 //	3

sbit add_n=P3^2;        
sbit send=P3^3;
sbit led0 =P0^0;
sbit led1 =P0^1;
sbit led2 =P0^2;

sbit ds= P3^3;		//

uint temp;	//
uint t;
uint templa;
float f_temp;

//---------------延时--------------//

//---------------延时--------------//
void Delay(unsigned char n)
{
	unsigned int i;
	while(n--)
	for(i=0;i<80;i++);

}
//--------------------------------11.0592M晶振  n=100 868us 左右-------------------------------
//------------------------------------------  毫秒延时         -----------------------------------------
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
/*******************************************************************************************
函数名：DS18B20复位，初始化函数
调  用：dsreset()
参  数：
返回值：无
结  果：
注  释：具体过程，参考DS18B20的工作时序图
*******************************************************************************************/
void dsreset (void)
{
	uint i;
	ds=0;	   		// 数据线拉低到低电平0
	i=103;
	while(i>0)		// 延时，480~960us，想延长多长时间，参考指令周期，可以计算求出。
		i--;
	ds=1;			//	数据线拉低到高电平1
	i=4;
		while(i>0) 	// 延时等待，15~60us
		i--;
}
/*******************************************************************************************
函数名：读位数据函数
调  用：tempreadbit()
参  数：
返回值：dat
结  果：
注  释：具体过程，参考DS18B20的读数据工作时序图
*******************************************************************************************/
bit tempreadbit(void)
{
 	uint i;
	bit dat;
	ds=0;	   	//	数据线拉低到0
	i++;		//	延时
	ds=1;		//	数据线拉高到1
	i++;i++;   	//	延时
	dat=ds;		//读数据线得到一个状态位
	i=8;
	while(i>0)	//处理延时
		i--;
	return (dat);
}
 /*******************************************************************************************
函数名：读字节数据函数
调  用：tempread()
参  数：
返回值：dat
结  果：
注  释：具体过程，参考DS18B20的读数据工作时序图
*******************************************************************************************/
uchar tempread()
{
	uchar i,j,dat;
	dat=0;
	for(i=1;i<=8;i++)
	{
		j=tempreadbit();
		dat=(j<<7)|(dat>>1);
	}	
	return (dat);
}
 /*******************************************************************************************
函数名：写字节数据函数
调  用：tempwritebyte()
参  数：
返回值：
结  果：
注  释：具体过程，参考DS18B20的写数据工作时序图
*******************************************************************************************/
void tempwritebyte(uchar dat)
{
	uint i;
	uchar j;
	bit testb;
	for(j=1;j<=8;j++)
	{
		testb=dat&0x01;
		dat=dat>>1;
		if(testb)//	写1
		{
		 	ds=0;
			i++;i++;
			ds=1;
			i=8;
			while(i>0)i--;
		}
		else
		{
			ds=0;//写0
			i=8;
			while(i>0)i--;
			ds=1;
			i++;i++;
		}
	}
}
 /*******************************************************************************************
函数名：获取温度并转换函数
调  用：tempchange()
参  数：
返回值：
结  果：
注  释：具体指令，参考DS18B20手册
*******************************************************************************************/
void tempchange(void)
{
	dsreset();
	Delay_ms(1);
	tempwritebyte(0xcc);   	//
	tempwritebyte(0x44);	//
}
 /*******************************************************************************************
函数名：读寄存器中存储的温度数据
调  用：get_temp()
参  数：
返回值：temp
结  果：
注  释：具体指令，参考DS18B20手册
*******************************************************************************************/
uint get_temp()
{
	uchar a,b;
	dsreset();
	Delay_ms(1);
	tempwritebyte(0xcc);
	tempwritebyte(0xbe);
	a=tempread(); 		// 读低八位
	b=tempread();		// 读高八位
	temp=b;
	temp<<=8;			//两个字节组成一个字
	temp=templa;
	f_temp=temp*0.0625;	//温度在寄存器中为12位，分辨率0.0625；
	temp=f_temp*10+0.5;//乘以10表示小数点后面只取1位，加0.5是四舍五入；
	f_temp=f_temp+0.05;
	return temp;
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

//-------------初始化设置---------------//
void nRF905Init(void)
{
    CSN=1;						// Spi 	disable
	SCK=0;						// Spi clock line init low
	DR=0;						//  DR=1  接收准备。DR＝0 发送准备  
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


//------------发送数据-------------//
void TxPacket()
{
   unsigned char i,j;
   CSN=0;
   
   SpiWrite(WTP);
		for(j=0;j<2;j++)
		  {
             SpiWrite(send_bruff[j]); //循环2次发送2个字节
      	   }
  CSN=1;
  Delay(1);						
  CSN=0;	
  SpiWrite(WTA);				        // Write address command
  for (i=0;i<4;i++)			// Write 4 bytes address
	   {
		 SpiWrite(TxAddress[i]);
	    }
  CSN=1;
  Delay(1);
  TRX_CE=1;
  Delay(1);
  TRX_CE=0;
}
//-----------------set send  mode-----------------//
void SetTxMode(void)
{
	
    TRX_CE=0;
	TXEN=1;
	Delay(1); 					// delay for mode change(>=650us)
}

//-----------------按键发送函数---------------------------------
void sender_bruff() 
{
                            
                  SetTxMode();
				  Delay_ms(10);
                  TxPacket();    //发送数据
              	  Delay_ms(100);    
}

//---------------按键加一函数--------------------------//
void add_number(void) 
{
 // int i,j;
  
    if(!add_n)
     {
	   Delay_ms(100);
   	   while(add_n)
	   		t=get_temp();	     
            send_bruff[0]=t;
			send_bruff[1]=t>>8;
	        Delay(100);
				    
      } 
}
//************************main function/***********************// 
void main()
 {
   
   nRF905Init();
   Config905();
   while(1)
         {	     		 
			  led0=1;
			  led1=1;
			  led2=1;
			//  DR=1;   RX();   led1=0;

			  //if(rx_bruff==0x12)  led_ISD(); 
			    if(!add_n)  
				         add_number();
			  while(!send);
			       Delay_ms(10);
			        if(!send)
			          {
					   	led0=0;
			           // DR=0;					
				 		sender_bruff();
				  		led0=0;
			  			led1=0;
			  			led2=0;
				 	  	Delay_ms(100);
			           }									                    
         }			
}
