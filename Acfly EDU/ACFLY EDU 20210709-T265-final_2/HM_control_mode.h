/*------��������ݮ�ɷ��͵���Ϣ�У�������������----------*/
#ifndef _CONTROL_MODE_
#define _CONTROL_MODE_

#define KEEP								'a'		//����ԭλ����
#define TAKE_OFF         	  'b'		//CON1Ϊ��ɸ߶ȣ�������������
#define GROUND             	'c'		//CON1Ϊ�����ٶȣ�������������
#define VELOCITY_CONTROL  	'd'		//CON1=Vx,CON2=Vy,CON3=Vz
#define POSITION_CONTROL		'e'		//CON1=Sx,CON2=Sy,CON3=Sz
#define POSTURE_CONTROL			'f'		//CON1=row,CON2=pitch,CON3=yaw�����ǵĽǶ����ݣ�
#define CIRCLE							'g'		//CON1=Vx,CON2=Vy,CON3=yaw��ƫ������ת�ٶȣ�
#define PLANE_FLIGHT        'h'   //CON1=Vx,CON2=Vy,��סZ��
#define LOCK								'i'		//�������˻�������M01
#define UNLOCK							'j'		//�������˻�������M35
#define DEBUG               'k'   //�������˻�
#define XYLOCK_VZ						'l'		//CON1 CON2��Ч��CON3Ϊz�ٶ�
#define VZ_TAKEOFF					'm'		// CON1 ��z���ٶȣ�CON2 �Ǹ߶�, CON3 ���������,ֻҪ ��ǰ�߶� > CON2 - CON3 ����
#define Get_Z       				'n'   //���͵�ǰ�߶ȸ���ݮ��
#define EMPTY								'o'		// ��״̬  ʲô����ִ�е�

#endif
