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
  0x00,                             //��������//
  0x4c,                             //CH_NO,����Ƶ����423MHZ
  0x0c,                             //�������Ϊ10db,���ط����ڵ�Ϊ����ģʽ
  0x44,                             //��ַ������ã�Ϊ4�ֽ�
  0x03,0x03,                        //���շ�����Ч���ݳ���Ϊ3�ֽ�
  0xCC,0xCC,0xCC,0xCC,              //���յ�ַ
  0x58,                             //CRC����8λCRCУ�飬�ⲿʱ���źŲ�ʹ�ܣ�16M����
};
unsigned char rx_Buffer[]={0};          //���ڴ�Ž��յ�������
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

sbit lcden=P3^3;		//LCDʹ�ܶ˽ӵ��ĸ��˿ھ�д�ĸ�
sbit lcdrs=P3^2;		//LCD��������ѡ���


uint temp;
//---------------��ʱ--------------//
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



//------------SPIд��������---------//

void SpiWrite(unsigned char send)
{
	unsigned char i;
	DATA_BUF=send;
	for (i=0;i<8;i++)
	{
		if (flag)	//���Ƿ������λ
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


//------------SPI����������---------//
unsigned char SpiRead(void)
{
	unsigned char j;
	for (j=0;j<8;j++)
	{
        DATA_BUF=DATA_BUF<<1;
		SCK=1;
		if (MISO)	//��ȡ���λ����������ĩβ��ͨ������λ��������ֽ�
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
//-----------resive ״̬����------------------//
void SetRxMode(void)
{
	TXEN=0;
	TRX_CE=1;
	Delay(1); 					// delay for mode change(>=650us)
}
//-------------��ʼ������---------------//
void nRF905Init(void)
{
    CSN=1;						// Spi 	disable
	SCK=0;						// Spi clock line init low
	DR=1;						// ���Ƭ��IO
	AM=0;						// Init AM for input
	CD=0;						// Init CD for input
	PWR=1;					    // nRF905 power on
	TRX_CE=0;					// Set nRF905 in standby mode
	TXEN=0;					    // set radio in Rx mode
}


//----------�������üĴ���-------//
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

//--------------��ȡRF905��������------------//
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
//-------����Ƿ��������ݴ���-----------//
unsigned char CheckDR(void)		//����Ƿ��������ݴ��� Data Ready
{
	if (DR)                    //��������ȫ����DRΪ1
	{
      
		return 1;
	}
	else
	{
		return 0;
	}
}


//------------------------------���ݽ���------------------//
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
��������write_com
��  �ã�write_com(com)
��  ����uchar com
����ֵ��
��  ����
ע  �ͣ�д�����������Һ�������ʾ/����ʾ�������˸/����˸���費��Ҫ��������ʲôλ����ʾ��
	   ����������ֵ���ο������ֲ�
*******************************************************************************************/
void write_com (uchar com)
{
	lcdrs=0;		//ѡ��д����ģʽ
	P0=com;			//�������͵�P0�ڣ����ӵ��ĸ��ڣ�д�ĸ���
	Delay_ms(5);		//��ʱ��ͼ�����Եó���д������Ҫʱ��
	lcden=1;		//Һ����ʹ�ܶ�
	Delay_ms(5);		
	lcden=0;
}

 /*******************************************************************************************
��������write_data
��  �ã�write_data(data)
��  ����uchar data
����ֵ��
��  ����
ע  �ͣ�д���ݺ�������ʾʲô����
*******************************************************************************************/
void write_data(uchar date)
{
	lcdrs=1;//ѡ��д����ģʽ
	P0=date;//�������͵�P0�ڣ����ӵ��ĸ��ڣ�д�ĸ���
	Delay_ms(5);//��ʱ��ͼ�����Եó���д������Ҫʱ��
	lcden=1;//Һ����ʹ�ܶ�
	Delay_ms(5);		
	lcden=0;
}

/*******************************************************************************************
��������LCD��ʼ��
��  �ã�init
��  ����
����ֵ��
��  ����
ע  �ͣ���ʼ��LCD
*******************************************************************************************/
void init()
{
	lcden=0;
	write_com(0x38);	//��ʼ�����ã�1������ʾģʽ���á�0011 1000���������ʲô���ܣ����ֲᣬ��ʵ��ʲô���ܣ�дʲô����
	write_com(0x0c);	//��ʼ�����ã�2������ʾ���ؼ�������á�0000 1100���������ʲô���ܣ����ֲᣬ��ʵ��ʲô���ܣ�дʲô����
	write_com(0x06);	//
	write_com(0x01);	//��������

}
/*******************************************************************************************
���������ѽ��յ�����ת�����¶�
��  �ã�get_RXtemp()
��  ����
����ֵ��
��  ����
ע  �ͣ�
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
����������ʾ�¶�����
��  �ã�write_temp_to_lcd(add,dat)
��  ����uchar add,uchar dat
����ֵ��
��  ����
ע  �ͣ���������ʾ����ʾʲô����
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


//-------------������---------------// 
void main()
 {
 //P0=0X00;
  P2^0=1;
  Delay_ms(100); 
  nRF905Init();
  Config905();
  Delay_ms(5);
  init();		//LCD��ʼ��
  while(1)
    {           
        RX();
		Delay_ms(5);
		get_RXtemp();
	 	init();		//LCD��ʼ��
		write_com(0x0c);
		write_temp_to_lcd(0x80,temp);	
		Delay_ms(100); 
		P2^0=0; 
     }

}