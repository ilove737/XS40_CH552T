
/********************************** (C) COPYRIGHT *******************************
* File Name          : UART1.C
* Author             : WCH
* Version            : V1.0
* Date               : 2018/07/25
* Description        : CH554 ����1�շ�  
*******************************************************************************/

#include "CH552.H"                                                          
#include "Debug.H"
#include "UART1.H"
#include "stdio.h"

#pragma  NOAREGS

/*******************************************************************************
* Function Name  : UART1Setup()
* Description    : CH554����1��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1Init( )
{
	U1SM0 = 0;                                                                   //UART1ѡ��8λ����λ
	U1SMOD = 1;                                                                  //����ģʽ
	U1REN = 1;                                                                   //ʹ�ܽ���
	SBAUD1 = 0 - FREQ_SYS/16/UART1_BUAD;
	U1TI = 0;
#if UART1_PINMAP	
    PIN_FUNC |= bUART1_PIN_X;                                                   //ӳ�䵽P34(R)��P32(T)
#endif

#if UART1_INTERRUPT                                                            //�����ж�ʹ��
	IE_UART1 = 1;	
	EA = 1;
#endif	
}
/*******************************************************************************
* Function Name  : CH554UART1RcvByte()
* Description    : CH554UART1����һ���ֽ�
* Input          : None
* Output         : None
* Return         : SBUF
*******************************************************************************/
UINT8 UART1RcvByte( )
{
    while(U1RI == 0);                                                           //��ѯ���գ��жϷ�ʽ�ɲ���
    U1RI = 0;
    return SBUF1;
}

/*******************************************************************************
* Function Name  : CH554UART1SendByte(UINT8 SendDat)
* Description    : CH554UART1����һ���ֽ�
* Input          : UINT8 SendDat��Ҫ���͵�����
* Output         : None
* Return         : None
*******************************************************************************/
void UART1SendByte(UINT8 SendDat)
{
	SBUF1 = SendDat;                                                             //��ѯ���ͣ��жϷ�ʽ�ɲ�������2�����,������ǰ��TI=0
	while(U1TI ==0);
	U1TI = 0;
}


#if UART1_INTERRUPT
/*******************************************************************************
* Function Name  : UART1Interrupt(void)
* Description    : UART1 �жϷ������
*******************************************************************************/
void UART1Interrupt( void ) interrupt INT_NO_UART1 using 1                       //����1�жϷ������,ʹ�üĴ�����1
{
	UINT8 dat;
	if(U1RI)
	{
		dat = SBUF1;
		U1RI = 0;
		
		UART1SendByte(dat);
	}
}
#endif

