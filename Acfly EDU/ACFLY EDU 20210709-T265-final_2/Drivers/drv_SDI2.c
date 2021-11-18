#include "AC_Math.h"
#include "Quaternion.h"
#include "MeasurementSystem.h"

#include "STS.h"
#include "drv_Uart2.h"

static bool SDI_RCTrigger( unsigned int Task_ID );
static void SDI_Server( unsigned int Task_ID );

/*-----��ȡ������------*/
float CON1,CON2,CON3;
uint8_t HM_Mode = 'a';

void init_drv_SDI2()
{
	STS_Add_Task( STS_Task_Trigger_Mode_Custom , 0 , SDI_RCTrigger , SDI_Server );
}

static bool SDI_RCTrigger( unsigned int Task_ID )
{
	if( UART2_DataAvailable() )
		return true;
	return false;
}
float SDI2_Point[32];
TIME SDI2_Time = {0};
static void SDI_Server( unsigned int Task_ID )
{
	//�򵥶��ο���Э�����
	
	/*״̬������*/
		static uint8_t rc_step1 = 0;	//0�����հ�ͷ'A' 'C'
																	//1������1�ֽ���Ϣ���
																	//2������1�ֽ���Ϣ����
																	//3���������ݰ�����
																	//4������2�ֽ�У��
		static uint8_t rc_step2 = 0;
	
		#define MAX_SDI_PACKET_SIZE 4*6
		static uint8_t msg_type;
		static uint8_t msg_length;
		ALIGN4 static uint8_t msg_pack[MAX_SDI_PACKET_SIZE];
		static uint8_t sumA;
		static uint8_t sumB;
		
		#define reset_SDI_RC ( rc_step1 = rc_step2 = 0 )
	/*״̬������*/
	
	uint8_t rc_buf[20];
	uint8_t length = read_UART2( rc_buf , 20 );
	for( uint8_t i = 0 ; i < length ; ++i )
	{
		uint8_t r_data = rc_buf[i];
		
		switch( rc_step1 )
		{
			case 0 :
				//���հ�ͷ'A''C'
				if( rc_step2 == 0 )
				{
					if( r_data == 'A' )
					{
						rc_step2 = 1;
					}
				}
				else
				{
					if( r_data == 'C' )
					{
						rc_step1 = 1;
						rc_step2 = 0;
						sumA = sumB = 0;
					}
					else
						rc_step2 = 0;
				}
				break;
				
			case 1:
				//������Ϣ���
				msg_type = r_data;
				sumA += r_data;
				rc_step1 = 2;
				rc_step2 = 0;
				break;
				
			case 2:
				//�������ݰ�
				msg_pack[ rc_step2 ] = r_data;
				sumA += r_data;
				++rc_step2;
				if( rc_step2 >= 12 )
				{
					rc_step1 = 4;
					rc_step2 = 0;
				}
				break;
				
			case 4:
				//����У��λ
				if( sumA != r_data )
				{
					rc_step1 = 0;
				}
				else
				{
							HM_Mode = msg_type;
							CON1 = *(float*)&msg_pack[0];    
							CON2 = *(float*)&msg_pack[4];  
							CON3 = *(float*)&msg_pack[8];
							SDI2_Time = get_TIME_now();
					reset_SDI_RC;
				}			
					
				break;
		}
	}
}

void Send_Float(float f)
{
	unsigned char s[5];
	unsigned char *p;
	p = (unsigned char *)&f;
	*s = 'A';
	*(s+1) = *p;
	*(s+2) = *(p+1);
	*(s+3) = *(p+2);
	*(s+4) = *(p+3);
	Uart2_Send(s,5);
}
