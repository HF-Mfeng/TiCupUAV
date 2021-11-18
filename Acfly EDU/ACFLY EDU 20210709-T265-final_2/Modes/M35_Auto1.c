#include "Modes.h"
#include "Basic.h"
#include "stdlib.h"
#include <stdio.h>

#include <math.h>

#include "M35_Auto1.h"

#include "AC_Math.h"
#include "Receiver.h"
#include "InteractiveInterface.h"
#include "ControlSystem.h"
#include "MeasurementSystem.h"

#include "drv_SDI.h"
#include "drv_Uart2.h"
#include "drv_SDI2.h"

#include "HM_control_mode.h"
#include "drv_OpticalFlow.h"

	int cc = 0;
extern float TFMiniHigh;
static float debug_sum = 0;

static void M35_Auto1_MainFunc();
static void M35_Auto1_enter();
static void M35_Auto1_exit();

	unsigned char sta[3] = {'a','S','S'};
	unsigned char end[3] = {'a','E','E'};
unsigned char err[3] = {'e','r','r'};

const Mode M35_Auto1 = 
{
	50 , //mode frequency
	M35_Auto1_enter , //enter
	M35_Auto1_exit ,	//exit
	M35_Auto1_MainFunc ,	//mode main func
};

typedef struct
{
	//�˳�ģʽ������
	uint16_t exit_mode_counter;
	
	//�Զ�����״̬��
	uint8_t auto_step1;	//0-��¼��ťλ��
											//1-�ȴ���ť������� 
											//2-�ȴ������� 
											//3-�ȴ�2��
											//4-����
											//5-�ȴ��������
	uint16_t auto_counter;
	float last_button_value;
}MODE_INF;
static MODE_INF* Mode_Inf;
float nowHightorpi = -1;
unsigned char sendBuffToRpi[5];
static void M35_Auto1_enter()
{
	Led_setStatus( LED_status_running1 );
	
	//��ʼ��ģʽ����
	Mode_Inf = malloc( sizeof( MODE_INF ) );
	Mode_Inf->exit_mode_counter = 0;
	Mode_Inf->auto_step1 = Mode_Inf->auto_counter = 0;
	Altitude_Control_Enable();
}

static void M35_Auto1_exit()
{
	Altitude_Control_Disable();
	Attitude_Control_Disable();
	
	free( Mode_Inf );
}

static void M35_Auto1_MainFunc()
{	
	RC_Type test = RC_Type_PPM;
	
	const Receiver* rctest = get_Receiver(RC_Type_PPM); 
	
	if( rctest->available == false )
	{
		//���ջ�������
		//����
		test = RC_Type_virtual;
	}
	
	const Receiver* rc = get_Receiver(test);
	
	float throttle_stick = rc->data[0];
	float yaw_stick = rc->data[1];
	float pitch_stick = rc->data[2];
	float roll_stick = rc->data[3];	
	
	/*�ж��˳�ģʽ*/
		if( throttle_stick < 5 && yaw_stick < 5 && pitch_stick < 5 && roll_stick > 95 )
		{
			if( ++Mode_Inf->exit_mode_counter >= 50 )
			{
				change_Mode( 1 );
				return;
			}
		}
		else
			Mode_Inf->exit_mode_counter = 0;
	/*�ж��˳�ģʽ*/
		
	//�ж�ҡ���Ƿ����м�
	bool sticks_in_neutral = 
		in_symmetry_range_offset_float( throttle_stick , 5 , 50 ) && \
		in_symmetry_range_offset_float( yaw_stick , 5 , 50 ) && \
		in_symmetry_range_offset_float( pitch_stick , 5 , 50 ) && \
		in_symmetry_range_offset_float( roll_stick , 5 , 50 );
	
	extern float SDI_Point[6];
	extern float SDI2_Point[32];
	extern TIME SDI_Time;
	/*-----��urat1��ȡ��������ݮ����Ϣ--------*/
	extern uint8_t HM_Mode;
	extern float CON1;
	extern float CON2;
	extern float CON3;
	
	const float EPSINON = 0.00001;
	float d = 3.14;
	const unsigned char h[1] = {'4'};
	const unsigned char k[1] = {'5'};
	const unsigned char z[1] = {'6'};
	static int myCtlMode = 0;
	if( sticks_in_neutral && get_Position_Measurement_System_Status() == Measurement_System_Status_Ready )  // ����һ��  ң�ؿ����ұ����
	//if( get_Position_Measurement_System_Status() == Measurement_System_Status_Ready )    // ����һ��  ң�ز������Ҳ���Ҫ��
	{
		//ҡ�����м�
		//ִ���Զ�����		
		//ֻ����λ����Чʱ��ִ���Զ�����
	
		//��ˮƽλ�ÿ���
		Position_Control_Enable();
		//�򿪸߶ȿ���
		Altitude_Control_Enable();
		//����̬����
		Attitude_Control_Enable();
		
		
		switch( HM_Mode )
		{
			case KEEP:
				//����ԭλ��ͣ
				Position_Control_set_XYLock();
				Position_Control_set_ZLock();
				Attitude_Control_set_YawLock();
				Mode_Inf->auto_step1 = 0;
				myCtlMode = 0;
				break;
			
			case TAKE_OFF:
				//CON1Ϊ��ɸ߶ȣ�������������
				if( myCtlMode == 0)
				{
						sta[0] = TAKE_OFF;
						Uart2_Send(sta,3);
						sta[0] = TAKE_OFF;
						Uart2_Send(sta,3);
						//Mode_Inf->last_button_value = rc->data[5];   // ��¼��ť��ʼλ��
						++Mode_Inf->auto_step1;
						Mode_Inf->auto_counter = 0;
						myCtlMode = 1;
				} 
				// else if(Mode_Inf->auto_step1 == 1)
				else if( myCtlMode == 1)
				{
					if(get_is_inFlight() == false )  //�жϷ���״̬
					{
						Position_Control_Takeoff_HeightRelative(CON1);
						Mode_Inf->auto_counter = 0;
						myCtlMode = 2 ;
					}
				}
				else //�ȴ�������
				{
					if( get_Altitude_ControlMode() == Position_ControlMode_Position ) 
					{
						Mode_Inf->last_button_value = rc->data[5];
						Mode_Inf->auto_counter = 0;
						myCtlMode = 0;
						end[0] = TAKE_OFF;
						Uart2_Send(end,3);
						end[0] = TAKE_OFF;
						Uart2_Send(end,3);
						HM_Mode = KEEP;
					}
				}
				break;
			case XYLOCK_VZ:   // ��סXY  ����Z���ٶ�
				Position_Control_set_XYLock();
				Position_Control_set_TargetVelocityZ(CON3);
				break;
			case GROUND:
				//CON1Ϊ�����ٶȣ�������������
				if(myCtlMode == 0)
				{
					Position_Control_set_TargetVelocityZ(-20); 
					++myCtlMode;
					Mode_Inf->auto_counter = 0;
					
				}
				else  //�ȴ��������
				{
					if( get_is_inFlight() == false )
					{
						myCtlMode = 0;
						Mode_Inf->auto_counter = 0;
						CON1=0;
						CON2=0;
						CON3=0;
						HM_Mode = KEEP;
						Attitude_Control_Disable();
						Altitude_Control_Disable();
						Position_Control_Disable();
						change_Mode( 1 );
					}
				}
				break;
			//	get_Position().z;
			case VZ_TAKEOFF:
				Position_Control_set_XYLock();
				Position_Control_set_TargetVelocityZ(CON1);
				if(TFMiniHigh > CON2){
					  end[0] = VZ_TAKEOFF;
						Uart2_Send(end,3);
						end[0] = VZ_TAKEOFF;
						Uart2_Send(end,3);
						HM_Mode = KEEP;
				}
				break;
				
			case VELOCITY_CONTROL:
				//CON1=Vx,CON2=Vy,CON3=Vz
				Position_Control_set_TargetVelocityBodyHeadingXY(CON1,CON2);
				Position_Control_set_TargetVelocityZ(CON3);
				break;
			
			case PLANE_FLIGHT:
				//ֻ����ˮƽ��X,Y�ٶȣ���סZ��
				Position_Control_set_TargetVelocityBodyHeadingXY(CON1,CON2);
				Position_Control_set_ZLock();
				break;
				
			case POSITION_CONTROL:
				//CON1=Sx,CON2=Sy,CON3=Sz
				if(myCtlMode == 0)
				{
					Position_Control_set_TargetPositionXYRelativeBodyHeading(CON1,CON2);
					Position_Control_set_TargetPositionZRelative(CON3);
					++myCtlMode;
				}
				else
				{
					if((get_Position_ControlMode() == Position_ControlMode_Position) && (get_Altitude_ControlMode() == Position_ControlMode_Position))
					{
						myCtlMode = 0;
						HM_Mode = KEEP;
					}
				}
			  break;
				
			case Get_Z:
				nowHightorpi = TFMiniHigh;
				sendBuffToRpi[0] = 'B';
				sendBuffToRpi[1] = *(((uint8_t *)(&nowHightorpi)) + 0);
				sendBuffToRpi[2] = *(((uint8_t *)(&nowHightorpi)) + 1);
				sendBuffToRpi[3] = *(((uint8_t *)(&nowHightorpi)) + 2);
				sendBuffToRpi[4] = *(((uint8_t *)(&nowHightorpi)) + 3);
				Uart2_Send(sendBuffToRpi,5);
				HM_Mode = EMPTY;
				break;
			
			case EMPTY:
					break;
						
			case POSTURE_CONTROL:
				//CON1=row,CON2=pitch,CON3=yaw�����ǵĽǶȣ�
				if(myCtlMode == 0)
				{
					Attitude_Control_set_Target_YawRelative(CON3);
					Position_Control_set_ZLock();
					Mode_Inf->auto_counter = 0;
					++myCtlMode;
				}
				else
				{
					if(++Mode_Inf->auto_counter == 200)
					{
						Attitude_Control_set_YawLock();
						myCtlMode = 0;
						HM_Mode = KEEP;
					}
				}
				break;
				
			case CIRCLE:
				//CON1=Vx,CON2=Vy,CON3=yaw��ƫ������ת�ٶȣ�
				Position_Control_set_TargetVelocityBodyHeadingXY(CON1,CON2);
				Attitude_Control_set_Target_YawRate(CON3);
				Position_Control_set_ZLock();
				break;
			
			case LOCK:
				//�˳�ģʽ
				Mode_Inf->auto_counter = 0;
				CON1=0;
				CON2=0;
				CON3=0;
				HM_Mode = KEEP;
				Attitude_Control_Disable();
				Altitude_Control_Disable();
				Position_Control_Disable();
				change_Mode( 1 );
				break;
			
			case DEBUG:
				//�������˻�
				debug_sum += CON1;
				if(CON1 == 100)
				{
					HM_Mode = KEEP;
					debug_sum = 0;
				}
			
			default:
				//�յ�������Ϣ�����޶�Ӧģʽ���ж�Ϊǿ����ͣ
				Attitude_Control_set_Target_YawRate(0);
				Position_Control_set_TargetVelocityZ(0);
				Position_Control_set_TargetVelocityBodyHeadingXY(0,0);
				static_write_uart1(err,3);
				HM_Mode = KEEP;
				break;
		}

	}
	else
	{
		//ҡ�˲����м�
		//�ֶ�����
		Mode_Inf->auto_step1 = Mode_Inf->auto_counter = 0;
		
		//�ر�ˮƽλ�ÿ���
		Position_Control_Disable();
		
		//�߶ȿ�������
		if( in_symmetry_range_offset_float( throttle_stick , 5 , 50 ) )
			Position_Control_set_ZLock();
		else
			Position_Control_set_TargetVelocityZ( ( throttle_stick - 50.0f ) * 6 );

		//ƫ����������
		if( in_symmetry_range_offset_float( yaw_stick , 5 , 50 ) )
			Attitude_Control_set_YawLock();
		else
			Attitude_Control_set_Target_YawRate( ( 50.0f - yaw_stick )*0.05f );
		
		//Roll Pitch��������
		Attitude_Control_set_Target_RollPitch( \
			( roll_stick 	- 50.0f )*0.015f, \
			( pitch_stick - 50.0f )*0.015f );
	}
}