//����V_FREEֵΪ5V modfied by LC 2015.08.05 23:10

#include "reg_24.h"


#define cli()  _asm("cli")
#define sei()  _asm("sei")
#define nop()  _asm("nop")
#define stop() _asm("stop")

#define uchar unsigned char
#define uint  unsigned int

//#define debug 1

#define DG_LED1   P04
#define DG_LED2   P05
#define DG_LED3   P06
#define DG_LED4   P26

/* uint dg_ad0;
uint dg_ad1;
uint dg_ad2; */

//LED
#define LED_PORT      P04 
#if debug
#define LED_ON        LED_PORT=1
#define LED_OFF       LED_PORT=0  //  modfied by LC 2015.07.21 11:55
#else
#define LED_ON        LED_PORT=0
#define LED_OFF       LED_PORT=1  //  modfied by LC 2015.07.21 11:55   	
#endif

//���ƽ�
#define P12V6_CTRL    P26
#define P12V6_ON      P12V6_CTRL=1
#define P12V6_OFF     P12V6_CTRL=0

#define P8V4_CTRL     P06
#define P8V4_ON       P8V4_CTRL=1
#define P8V4_OFF      P8V4_CTRL=0

#define P8V4_CTRL1    P20
#define P8V4C1_ON     P8V4_CTRL1=0
#define P8V4C1_OFF    P8V4_CTRL1=1

#define P4V2_CTRL     P05
#define P4V2_ON       P4V2_CTRL=1
#define P4V2_OFF      P4V2_CTRL=0

#define P4V2_CTRL1    P21
#define P4V2C1_ON     P4V2_CTRL1=0
#define P4V2C1_OFF    P4V2_CTRL1=1

//AD��
#define P12V6_ADCH    2
#define P8V4_ADCH     1
#define P4V2_ADCH     0


//ϵͳ״̬
volatile uchar sys_status;
#define FREE          0//δ������״̬
#define FALUT         1//����
#define PRE_CHARGE    2//Ԥ��
#define FULL_CHARGE   3//ȫ��
#define ALONE_CHARGE  4//����
#define FULL          5//����

#define STATUS_MAX 1//����� ���������л�
volatile uchar status_count;//ͬһ״̬��������ֵ
volatile uchar last_status;

//��ѹ��ֵ����,��ֵ��ʵ�ʵ�ѹֵƫ��Լ0.12V
#define V_FAULT_MIN   0  //0V 
#define V_FREE_MIN    205//1*1024/5 1V С�ڸ�ֱֵ�ӿ��ж�Ϊ���߻��·
#define V_FAULT_MID   471// 2.3*1024/5 2V
#define V_FAULT_MID_ADD 492//2.4V ��ѹ���ϵ�Ԥ����0.1V����
#define V_PRE_CHARGE  676// 3.3V*1024/5 3V

// #define V_FREE_MIN    205//1*1024/5 1V С�ڸ�ֱֵ�ӿ��ж�Ϊ���߻��·
// #define V_FAULT_MID   717// 3.5*1024/5 2V
// #define V_FAULT_MID_ADD 737//3.6V ��ѹ���ϵ�Ԥ����0.1V����
// #define V_PRE_CHARGE  809// 3.95*1024/5 3V

#define V_FULL        864//4.24*1024/5 4.24V //��Ϊ4.22V modfied by LC 2015.07.30 22:23
#define V_FULL_CHARGE 874   //4.27���������ѹ�����ӳ���ٶ�add by LC 2015.07.30 22:22
#define V_FULL_ADD    885//4.32V ���������״̬��0.08V����
#define V_FAULT_SUB   926//4.44V ���ϵ�ȡ������ 0.08V   //��Ϊ4.52V modfied by LC 2015.07.30 22:07
#define V_FAULT_MAX   946//4.52*1024/5 4.52V //���ڸ�ֵ�ж�Ϊ��ѹ����  //��Ϊ4.62V 946 modfied by LC 2015.07.30 22:07
#define V_FREE        1024// 6*1024/5  6V  //��ԭ����6V��ֵ��Ϊ5V 6V����2S��3S�ж��ٽ���ϣ�̫Σ���� add by LC 2015.08.05 23:07

#define AD_4V2_OFF    82// 0.4*1024/5  ���һ����ѹ�㶨ƫ����
#define AD_NEXT_OFF    10//0.05*1024/5  0.05V ���β�����ƽ��ֵ��ѹ��ܳ���0.05V
//ϵͳ����
volatile uint AD_12V6_value;
volatile uint AD_8V4_value;
volatile uint AD_4V2_value;

volatile uchar time_125us;
volatile uchar time_10ms;
// volatile uchar time_free_10ms;//��ȱģʽ�µļ�����0 -- 100
// volatile uchar time_full_10ms;//�����͹���ģʽ�µļ�����
volatile uchar time_charge_10ms;//ȫ��ģʽ�µļ�����
// volatile uchar time_alcharge_10ms;//����ģʽ�µļ�����
// volatile uchar time_pre_10ms;//Ԥ��ģʽ�µļ�ʱ��

volatile uchar get_ad_flag;//AD�ɼ���־λ
volatile uchar AD12V6_FLAG;//��һ�ŵ���Ƿ���ڱ�־λ 0 û�� 1 ��

#include "int_24.h"

//0x01 40us
void Delay(uint ms) 
{       
       	uchar i;
       	while(ms--)
       	for(i = 0; i<150; i++);           
}

void SysInit(void)
{
       	//IO�ڶ��� P00 P01 P02����ΪAD���룬����ȫ��Ϊ��ͨ���
       	P0CONL = 0xBF;
       	P0CONH = 0xAA;
       	P1CON = 0x0A;
       	P2CONL = 0xAA;
       	P2CONH = 0x4A;

       	BTCON = 0XA0; //�ؿ��Ź�
       	MCR = 0x00;   //LVR��
       	
       	//AD�ڳ�ʼ��
       	ADCON = 0x82;//Fsys/4  8M/2/4=1M ADC����ʱ��
       	//��ʱ����ʼ��
       	T0CON = 0x80;//Fsys/8  8M/2/8=0.5M ��ʱ����ʱƵ��
       	T0E = 1;      //����ʱ��
       	
/*             	AD_12V6_value = V_FREE;
       	AD_8V4_value = 0;
       	AD_4V2_value = 0; */
       	sys_status = FREE;
       	
       	time_125us = 0;
       	time_10ms = 0;
       	time_charge_10ms = 0;
       	AD12V6_FLAG = 1;//Ĭ��3Sģʽ
       	get_ad_flag = 0;
       	       	
       	       	P12V6_OFF;
       	P8V4_OFF;
       	P8V4C1_OFF;
       	P4V2_OFF;
       	P4V2C1_OFF; 
       	       	LED_OFF;
       	
}

/*******************************************************************************
* �� �� �� ��AD_Check
* �������� �����ĳһ·�ĵ�ǰ��ѹ��һ�μ��Լ60us
* ��    �� ����
* ��    �� ����
*******************************************************************************/
int AD_Check(uchar AD_ch)
{
       	uint AD_Value;
       	ADCH=AD_ch; //ѡ��ͨ��
       	ADPS=1; //ADʱ��Ƶ��4��Ƶ 1MHz
       	nop();
       	nop();
       	nop();
       	ADCE=1; //����AD
       	while(EOC==0);//ADת����
       	AD_Value=ADDATAH*4 + (ADDATAL&0x03);//��ȡADֵ
       	return AD_Value;
}

/*******************************************************************************
* �� �� �� ��AbsSub
* �������� �����ľ���ֵ
* ��    �� ����
* ��    �� ����
*******************************************************************************/
int AbsSub(int a,int b)
{
       	if(a>=b) return a-b;
       	if(a<b) return b-a;
}

/*******************************************************************************
* �� �� �� ��GetADValue
* �������� ����ȡ��·����ADֵ������ȫ�ֱ��� AD_12V6_value,AD_8V4_value,AD_4V2_value
* ��    �� ����
* ��    �� ����
*******************************************************************************/
void GetADValue(void)
{
       	int AD_12_temp;
       	int AD_4_temp;
       	int AD_8_temp;
       	
       	int AD_12_temp_avg=0;
       	int AD_4_temp_avg=0;
       	int AD_8_temp_avg=0;
       	
       	uchar i=0;
       	//��·�ֱ�10�ζ�ȡADֵ���������ƽ��ֵ������������ѹ��ƽ��ֵ�Ƚϣ�һ�����ֵ�ѹ��ƽ����Ĺ���
       	//�ͷ���ǰ��Ķ�ȡ�����¼���ƽ��ֵ
       	while(i<6)
       	{
       	       	//ȡAD
       	       	AD_4_temp = AD_Check(P4V2_ADCH);
       	       	AD_8_temp = AD_Check(P8V4_ADCH);
       	       	AD_12_temp = AD_Check(P12V6_ADCH);
       	       	
       	         //    	AD_12_temp = AD_12_temp*3;
       	       	AD_12_temp = AD_12_temp*2 + AD_12_temp*24/25;//��߾���
       	       	AD_8_temp = AD_8_temp<<1;
       	       	AD_4_temp = AD_4_temp<<1;
       	       	
       	       	//���
       	       	// AD_12_temp = AD_12_temp - AD_8_temp;
       	       	// AD_8_temp = AD_8_temp - AD_4_temp;
       	       	       	       	if(AD_12_temp > AD_8_temp)
       	       	       	       	{
       	       	       	       	       	AD_12_temp = AD_12_temp - AD_8_temp;
       	       	       	       	}else
       	       	       	       	{
       	       	       	       	       	AD_12_temp = 0;
       	       	       	       	}
       	       	       	       	if(AD_8_temp > AD_4_temp)
       	       	       	       	{
       	       	       	       	       	AD_8_temp = AD_8_temp - AD_4_temp;
       	       	       	       	}
       	       	       	       	else
       	       	       	       	{
       	       	       	       	       	AD_8_temp = 0;
       	       	       	       	}
       	       	if(AD_4_temp > AD_4V2_OFF)
       	       	{
       	       	       	AD_4_temp = AD_4_temp - AD_4V2_OFF;//��ȥ�㶨ƫѹ  
       	       	}else
       	       	{
       	       	       	AD_4_temp = 0;
       	       	}
       	       	
       	       	
       	       	if(i>0)
       	       	{
       	       	       	//һ��������������Ե�ǰ����Ϊ��ʼ�����²���
       	       	       	if((AbsSub(AD_12_temp_avg,AD_12_temp) > AD_NEXT_OFF) || (AbsSub(AD_8_temp_avg,AD_8_temp) > AD_NEXT_OFF) || (AbsSub(AD_4_temp_avg,AD_4_temp) > AD_NEXT_OFF)) 
       	       	       	{
       	       	       	       	i=1; 
       	       	       	       	AD_12_temp_avg = AD_12_temp;
       	       	       	       	AD_4_temp_avg = AD_4_temp;
       	       	       	       	AD_8_temp_avg = AD_8_temp;     	       	       	       	
       	       	       	       	continue;
       	       	       	}
       	       	}
       	       	//��ƽ��ֵ
       	       	AD_12_temp_avg = (AD_12_temp_avg + AD_12_temp)>>1;
       	       	AD_8_temp_avg = (AD_8_temp_avg + AD_8_temp)>>1;
       	       	AD_4_temp_avg = (AD_4_temp_avg + AD_4_temp)>>1;
       	       	i++;
       	}
       	
       	//���ݵ�ǰ���״̬������ЧADֵ      	
       	if(P12V6_CTRL == 1) 
       	AD_12V6_value = AD_12_temp_avg;
       	if(((P12V6_CTRL == 1) && (P8V4_CTRL1 == 1)) || (P8V4_CTRL == 1)) 
       	AD_8V4_value = AD_8_temp_avg;
       	if(((P12V6_CTRL == 1) && (P8V4_CTRL1 == 1) && (P4V2_CTRL1 == 1)) || ((P8V4_CTRL == 1) && (P4V2_CTRL1 == 1)) || (P4V2_CTRL == 1))
       	AD_4V2_value = AD_4_temp_avg;  	
}      	

/*******************************************************************************
* �� �� �� ��Execute
* �������� ���ݵ�ǰ״ִ̬�����ģʽ,ע��P12V6��P8V4,P4V2ֻ����һ·��ͨ��10msִ��һ��
* ��    �� ����
* ��    �� ����
*******************************************************************************/
void Execute(void)
{
       	//�Ƚ�����IO��������㣬LED�ڳ���
       	P12V6_OFF;
       	P8V4_OFF;
       	P8V4C1_OFF;
       	P4V2_OFF;
       	P4V2C1_OFF; 
       	
       	//���ݵ�ǰ״ִ̬��
       	
       	time_charge_10ms++;
       	if(time_charge_10ms >= 100) time_charge_10ms = 0;//1s���ڶ�ʱ
       	if(time_charge_10ms == 95 && sys_status != FREE)//��ʱ���� ��900ms��λ�ö�ʱ����,��freeʱ��Ҫ����������
       	{
       	       	if(AD12V6_FLAG == 0)//û�е�һ�ŵ��
       	       	{
       	       	       	P8V4_ON; //P8V4��ͨ10ms          	
       	       	}else
       	       	{
       	       	       	P12V6_ON; //P12V6��ͨ10ms        	       	       	       	
       	       	}
       	       	get_ad_flag = 1;
       	}  
       	       	
       	       	
       	/*ȫ��ģʽ��P12V6��P8V4��ͨ����ռ�� 100%*/
       	if(sys_status == FULL_CHARGE)
       	{
       	       	if(AD12V6_FLAG == 0)//û�е�һ�ŵ��
       	       	{
       	       	       	P8V4_ON;
       	       	}else
       	       	{
       	       	       	P12V6_ON;
       	       	}
       	       	LED_ON;
       	       	return;
       	}
       	
       	/*����ģʽ,������·��һ·��25%ռ�ձȳ䣬ռ�� 200ms/1s*/
       	if(sys_status == ALONE_CHARGE)
       	{
       	       	
       	       	if((AD_12V6_value < V_FULL_ADD) && (AD12V6_FLAG == 1))//��һ������
       	       	{
       	       	       	if(time_charge_10ms >= 0 && time_charge_10ms < 25) //0 -- 20
       	       	       	{
       	       	       	       	P12V6_ON;
       	       	       	       	P8V4C1_ON;
       	       	       	}
       	       	}
       	       	
       	       	if(AD_8V4_value < V_FULL_ADD)//�ڶ�������
       	       	{
       	       	       	if(time_charge_10ms >= 30 && time_charge_10ms < 55) //30 -- 50
       	       	       	{
       	       	       	       	P8V4_ON;
       	       	       	       	P4V2C1_ON;
       	       	       	}
       	       	}
       	       	
       	       	if(AD_4V2_value < V_FULL_ADD)//����������
       	       	{
       	       	       	if(time_charge_10ms >= 60 && time_charge_10ms < 85) // 60 -- 80
       	       	       	{
       	       	       	       	P4V2_ON;
       	       	       	}
       	       	}
       	       	LED_ON;
       	       	return;
       	}
       	
       	/*Ԥ��״̬��P12V6��P8V4��ͨ��ռ�� 200ms/2s*/
       	if(sys_status == PRE_CHARGE)
       	{
       	       	if(AD12V6_FLAG == 0)//û�е�һ�ŵ��
       	       	{
       	       	       	if(time_charge_10ms >= 0 && time_charge_10ms < 10) P8V4_ON; //P8V4��ͨ200ms              	       	
       	       	}else
       	       	{
       	       	       	if(time_charge_10ms >= 0 && time_charge_10ms < 10) P12V6_ON; //P12V6��ͨ200ms            	       	       	       	
       	       	}
       	       	       	       	LED_ON;
       	       	return;
       	}
       	
       	/*��ȱ̬,P12V6,P8V4������ͨ��ռ�� 10ms/1s */
       	if(sys_status == FREE)
       	{
       	       	if(time_charge_10ms == 95) 
       	       	{
               	       	P8V4_ON; //P8V4��ͨ10ms
               	 get_ad_flag = 1;
       	       	}
       	       	LED_OFF;
       	       	return;
       	}
       	
       	/*�����Լ�����״̬��P12V6��P8V4��ͨ��ռ�� 10ms/1s*/
       	if(sys_status == FALUT || sys_status == FULL)
       	{
       	       	if(sys_status == FALUT)
       	       	{
               	       	       	if(time_charge_10ms%50 == 0) LED_PORT = !LED_PORT;//��������
       	       	}else
       	       	{
       	       	       	LED_OFF;
       	       	}
       	       	
       	       	return;
       	}
}

void main(void)
{
       	uchar status_temp;
       	uint AD_value_test;//add by LC 2015.07.20 14:11
    //   Delay(100);
       	SysInit();
       	       	Delay(100);
       	cli();
       	while(1)
       	{
       	       	if(time_10ms >= 1)//ʱ��Ϊ10ms
       	       	{
       	       	       	time_10ms = 0;
       	       	       	//ִ��
       	       	       	Execute();
       	       	       	//AD����
       	       	       	if(get_ad_flag == 1)
       	       	       	{
               	       	       	      //       	LED_ON;
       	       	       	       	get_ad_flag = 0;
       	       	       	       	Delay(4);//��ʱ�ȴ���ѹ�ȶ�Լ2ms
       	       	       	       	GetADValue();

       	       	       	       	
       	       	       	       	//��ȱ�ж�,��·�����ж�
       	       	       	       	if((AD_8V4_value > V_FREE) || (AD_4V2_value > V_FREE) || ((AD_12V6_value > V_FREE) && (AD12V6_FLAG == 1)))
       	       	       	       	{
       	       	       	       	       	status_temp = FALUT;
       	       	       	       	       	if(P12V6_CTRL == 1)//3Sģʽ
       	       	       	       	       	{
       	       	       	       	       	       	if(AD_12V6_value > V_FREE && AD_8V4_value < V_FREE_MIN && AD_4V2_value < V_FREE_MIN)
       	       	       	       	       	       	{
       	       	       	       	       	       	       	status_temp = FREE;
       	       	       	       	       	       	}
       	       	       	       	       	}else //2Sģʽ
       	       	       	       	       	{
       	       	       	       	       	       	if(AD_8V4_value > V_FREE && AD_4V2_value < V_FREE_MIN)
       	       	       	       	       	       	{
       	       	       	       	       	       	       	status_temp = FREE;
       	       	       	       	       	       	       	//AD12V6_FLAG = 1;//Ĭ��3Sģʽ
       	       	       	       	       	       	}
       	       	       	       	       	}
       	       	       	       	       	
       	       	       	       	}
       	       	       	       	//�����ж�  ����һ·��ѹ���ִ��� V_FAULT_MAX ��ֵ
       	       	       	       	else if((AD_8V4_value>V_FAULT_MAX) || (AD_4V2_value>V_FAULT_MAX) || ((AD_12V6_value>V_FAULT_MAX) && (AD12V6_FLAG == 1)))
       	       	       	       	{
       	       	       	       	       	status_temp = FALUT;
       	       	       	       	}
       	       	       	       	//�����ж�  ����һ·��ѹ����С�� V_FAULT_MID ��ֵ
       	       	       	       	else if((AD_8V4_value<V_FAULT_MID) || (AD_4V2_value<V_FAULT_MID) || ((AD_12V6_value<V_FAULT_MID) && (AD12V6_FLAG == 1)))
       	       	       	       	{
       	       	       	       	       	status_temp = FALUT;
       	       	       	       	}
       	       	       	       	//Ԥ���ж�  ����һ·����С�� V_PRE_CHARGE ��ֵ
       	       	       	       	else if((AD_8V4_value<V_PRE_CHARGE) || (AD_4V2_value<V_PRE_CHARGE) || ((AD_12V6_value<V_PRE_CHARGE) && (AD12V6_FLAG == 1)))
       	       	       	       	{
       	       	       	       	       	if(sys_status == FALUT)
       	       	       	       	       	{
       	       	       	       	       	       	if((AD_8V4_value>V_FAULT_MID_ADD) && (AD_4V2_value>V_FAULT_MID_ADD) && ((AD_12V6_value>V_FAULT_MID_ADD) || (AD12V6_FLAG == 0)))//��Ҫ����Ԥ�����Сֵ
       	       	       	       	       	       	status_temp = PRE_CHARGE;      	       	       	       	       	       	       	       	       	       	
       	       	       	       	       	}else
       	       	       	       	       	{
       	       	       	       	       	       	status_temp = PRE_CHARGE;      	
       	       	       	       	       	}

       	       	       	       	}
       	       	       	       	//ȫ���ж�  ����·��ѹ��С�� V_FULL
       	       	       	       	else if((AD_8V4_value<V_FULL_CHARGE) && (AD_4V2_value<V_FULL_CHARGE) && ((AD_12V6_value<V_FULL_CHARGE) || (AD12V6_FLAG == 0)))
       	       	       	       	{
       	       	       	       	       	status_temp = FULL_CHARGE;
       	       	       	       	}
       	       	       	       	//�����ж�  ���е�ѹ�����ڵ��� V_FULL_ADD
       	       	       	       	else if((AD_8V4_value>=V_FULL_ADD) && (AD_4V2_value>=V_FULL_ADD) && ((AD_12V6_value>=V_FULL_ADD) || (AD12V6_FLAG == 0)))
       	       	       	       	{
       	       	       	       	       	if(sys_status == FALUT)//��һ״̬Ϊfault���������е�ѹֵ������ V_FAULT_SUB
       	       	       	       	       	{
       	       	       	       	       	       	if((AD_8V4_value<V_FAULT_SUB) && (AD_4V2_value<V_FAULT_SUB) && ((AD_12V6_value<V_FAULT_SUB) || (AD12V6_FLAG == 0)))
       	       	       	       	       	       	       	status_temp = ALONE_CHARGE;
       	       	       	       	       	}else
       	       	       	       	       	{
       	       	       	       	       	       	status_temp = FULL;
       	       	       	       	       	}
       	       	       	       	}
       	       	       	       	//�ֳ��ж� ĳһ·���ִ��� V_FULL �����
       	       	       	       	else if((AD_8V4_value>=V_FULL) || (AD_4V2_value>=V_FULL) || ((AD_12V6_value>=V_FULL) && (AD12V6_FLAG == 1)))
       	       	       	       	{
       	       	       	       	       	if(sys_status == FULL)//��һ״̬Ϊfull,Ϊ��ѹ����ģ������ж��Ƿ���һ·��ѹֵС��V_FULL
       	       	       	       	       	{
       	       	       	       	       	       	if((AD_8V4_value<V_FULL) || (AD_4V2_value<V_FULL) || ((AD_12V6_value<V_FULL) && (AD12V6_FLAG == 1)))
       	       	       	       	       	       	       	status_temp = ALONE_CHARGE;
       	       	       	       	       	}else if(sys_status == FALUT)//��һ״̬Ϊfault���������е�ѹֵ������ V_FAULT_SUB
       	       	       	       	       	{
       	       	       	       	       	       	if((AD_8V4_value<V_FAULT_SUB) && (AD_4V2_value<V_FAULT_SUB) && ((AD_12V6_value<V_FAULT_SUB) || (AD12V6_FLAG == 0)))
       	       	       	       	       	       	       	status_temp = ALONE_CHARGE;
       	       	       	       	       	}else
       	       	       	       	       	{
       	       	       	       	       	       	status_temp = ALONE_CHARGE;
       	       	       	       	       	}
       	       	       	       	       	
       	       	       	       	}
       	       	       	       	
       	       	       	       	if(status_temp == last_status)//��ǰ״̬���ϴ�״̬��ͬ����״ֵ̬�Լ�
       	       	       	       	{
       	       	       	       	       	status_count++;
       	       	       	       	}
       	       	       	       	
       	       	       	       	if(status_temp != last_status)//��ǰ״̬���ϴ�״̬��ͬ����״̬����Ϊ1�����޸��ϴ�״̬
       	       	       	       	{
       	       	       	       	       	status_count = 1;
       	       	       	       	       	last_status = status_temp;
       	       	       	       	}
       	       	       	       	//��ȱ̬��������̬,�ж� 3Sģʽ �� 2Sģʽ
       	       	       	       	if((sys_status == FREE) && (status_temp != FREE) && (status_count >= STATUS_MAX))
       	       	       	       	{
       	       	       	       	       	//�Ƚ�����IO��������㣬LED�ڳ���
       	       	       	       	       	P12V6_OFF;
       	       	       	       	       	P8V4_OFF;
       	       	       	       	       	P8V4C1_OFF;
       	       	       	       	       	P4V2_OFF;
       	       	       	       	       	P4V2C1_OFF; 
       	       	       	       	       	Delay(600);//��ʱ����ȫ����
       	       	       	       	       	
       	       	       	       	       	P12V6_ON;
       	       	       	       	       	Delay(20);
       	       	       	       	       	GetADValue();//��ȡ����ֵ
       	       	       	       	       	if(AD_12V6_value < V_FREE)//��ʾ��3S
       	       	       	       	       	{
       	       	       	       	       	       	AD12V6_FLAG = 1;
       	       	       	       	       	       	status_temp = FULL;//�����3Sģʽ����ϵͳ״̬��Ϊfull���²ɼ�������LED����
       	       	       	       	       	}else
       	       	       	       	       	{
       	       	       	       	       	       	AD12V6_FLAG = 0;
       	       	       	       	       	}
       	       	       	       	}
       	       	       	       	
       	       	       	       	if((status_count >= STATUS_MAX))               	       	       	       	
       	       	       	       	{
       	       	       	       	       	status_count = 0;
       	       	       	       	       	sys_status = status_temp;
       	       	       	       	}
       	       	       	       	       	       	       	       	
       	       	       	       	       	       	       	       	if(sys_status == FREE)
       	       	       	       	{
               	       	       	       	       	AD12V6_FLAG = 0;//��ΪĬ��2Sģʽ�������free״̬�����״̬����fault modfied by LC 2015.07.30 21:47
       	       	       	       	       //      	AD_12V6_value = V_PRE_CHARGE;
       	       	       	       	       	AD_12V6_value =  V_FREE;
       	       	       	       	}
       	       	       	}
       	       	       	
       	       	}
       	}
}
