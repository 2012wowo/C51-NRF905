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
  0x00,                             //��������//
  0x4c,                             //CH_NO,����Ƶ����423MHZ
  0x0c,                             //�������Ϊ10db,���ط����ڵ�Ϊ����ģʽ
  0x44,                             //��ַ������ã�Ϊ4�ֽ�
  0x03,0x03,                        //���շ�����Ч���ݳ���Ϊ3�ֽ�
  0xCC,0xCC,0xCC,0xCC,              //���յ�ַ
  0x58,                             //CRC����8λCRCУ�飬�ⲿʱ���źŲ�ʹ�ܣ�16M����
};

//unsigned char tempreture[9]={0X00,0X01,0X42,0X22,0X34,0X28,0X08,0X33,0X00,0X20};//Ԥ����������
unsigned char send_bruff[2]={0X01,0X01};
unsigned char TxAddress[4]={0xcc,0xcc,0xcc,0xcc};       //��ַ��
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

//---------------��ʱ--------------//

//---------------��ʱ--------------//
void Delay(unsigned char n)
{
	unsigned int i;
	while(n--)
	for(i=0;i<80;i++);

}
//--------------------------------11.0592M����  n=100 868us ����-------------------------------
//------------------------------------------  ������ʱ         -----------------------------------------
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
��������DS18B20��λ����ʼ������
��  �ã�dsreset()
��  ����
����ֵ����
��  ����
ע  �ͣ�������̣��ο�DS18B20�Ĺ���ʱ��ͼ
*******************************************************************************************/
void dsreset (void)
{
	uint i;
	ds=0;	   		// ���������͵��͵�ƽ0
	i=103;
	while(i>0)		// ��ʱ��480~960us�����ӳ��೤ʱ�䣬�ο�ָ�����ڣ����Լ��������
		i--;
	ds=1;			//	���������͵��ߵ�ƽ1
	i=4;
		while(i>0) 	// ��ʱ�ȴ���15~60us
		i--;
}
/*******************************************************************************************
����������λ���ݺ���
��  �ã�tempreadbit()
��  ����
����ֵ��dat
��  ����
ע  �ͣ�������̣��ο�DS18B20�Ķ����ݹ���ʱ��ͼ
*******************************************************************************************/
bit tempreadbit(void)
{
 	uint i;
	bit dat;
	ds=0;	   	//	���������͵�0
	i++;		//	��ʱ
	ds=1;		//	���������ߵ�1
	i++;i++;   	//	��ʱ
	dat=ds;		//�������ߵõ�һ��״̬λ
	i=8;
	while(i>0)	//������ʱ
		i--;
	return (dat);
}
 /*******************************************************************************************
�����������ֽ����ݺ���
��  �ã�tempread()
��  ����
����ֵ��dat
��  ����
ע  �ͣ�������̣��ο�DS18B20�Ķ����ݹ���ʱ��ͼ
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
��������д�ֽ����ݺ���
��  �ã�tempwritebyte()
��  ����
����ֵ��
��  ����
ע  �ͣ�������̣��ο�DS18B20��д���ݹ���ʱ��ͼ
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
		if(testb)//	д1
		{
		 	ds=0;
			i++;i++;
			ds=1;
			i=8;
			while(i>0)i--;
		}
		else
		{
			ds=0;//д0
			i=8;
			while(i>0)i--;
			ds=1;
			i++;i++;
		}
	}
}
 /*******************************************************************************************
����������ȡ�¶Ȳ�ת������
��  �ã�tempchange()
��  ����
����ֵ��
��  ����
ע  �ͣ�����ָ��ο�DS18B20�ֲ�
*******************************************************************************************/
void tempchange(void)
{
	dsreset();
	Delay_ms(1);
	tempwritebyte(0xcc);   	//
	tempwritebyte(0x44);	//
}
 /*******************************************************************************************
�����������Ĵ����д洢���¶�����
��  �ã�get_temp()
��  ����
����ֵ��temp
��  ����
ע  �ͣ�����ָ��ο�DS18B20�ֲ�
*******************************************************************************************/
uint get_temp()
{
	uchar a,b;
	dsreset();
	Delay_ms(1);
	tempwritebyte(0xcc);
	tempwritebyte(0xbe);
	a=tempread(); 		// ���Ͱ�λ
	b=tempread();		// ���߰�λ
	temp=b;
	temp<<=8;			//�����ֽ����һ����
	temp=templa;
	f_temp=temp*0.0625;	//�¶��ڼĴ�����Ϊ12λ���ֱ���0.0625��
	temp=f_temp*10+0.5;//����10��ʾС�������ֻȡ1λ����0.5���������룻
	f_temp=f_temp+0.05;
	return temp;
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

//-------------��ʼ������---------------//
void nRF905Init(void)
{
    CSN=1;						// Spi 	disable
	SCK=0;						// Spi clock line init low
	DR=0;						//  DR=1  ����׼����DR��0 ����׼��  
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


//------------��������-------------//
void TxPacket()
{
   unsigned char i,j;
   CSN=0;
   
   SpiWrite(WTP);
		for(j=0;j<2;j++)
		  {
             SpiWrite(send_bruff[j]); //ѭ��2�η���2���ֽ�
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

//-----------------�������ͺ���---------------------------------
void sender_bruff() 
{
                            
                  SetTxMode();
				  Delay_ms(10);
                  TxPacket();    //��������
              	  Delay_ms(100);    
}

//---------------������һ����--------------------------//
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
