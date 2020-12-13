
/********************************** (C) COPYRIGHT *******************************
* File Name          : DataFlash.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH554 DataFlash�ֽڶ�д��������   
*******************************************************************************/

#include "CH552.H"
#include "Debug.H"
#include "DataFlash.H"

/*******************************************************************************
* Function Name  : WriteDataFlash(UINT8 Addr,PUINT8 buf,UINT8 len)
* Description    : DataFlashд
* Input          : UINT8 Addr��PUINT16 buf,UINT8 len
* Output         : None
* Return         : UINT8 i ����д�볤��
*******************************************************************************/
UINT8 WriteDataFlash(UINT8 Addr, PUINT8 buf, UINT8 len)
{
  UINT8 i;
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xAA;        //���밲ȫģʽ
  GLOBAL_CFG |= bDATA_WE; //ʹ��DataFlashд
  SAFE_MOD = 0;           //�˳���ȫģʽ
  ROM_ADDR_H = DATA_FLASH_ADDR >> 8;
  Addr <<= 1;
  for (i = 0; i < len; i++)
  {
    ROM_ADDR_L = Addr + i * 2;
    ROM_DATA_L = *(buf + i);
    if (ROM_STATUS & bROM_ADDR_OK)
    {                           // ������ַ��Ч
      ROM_CTRL = ROM_CMD_WRITE; // д��
    }
    if ((ROM_STATUS ^ bROM_ADDR_OK) > 0)
      return i; // ����״̬,0x00=success,  0x02=unknown command(bROM_CMD_ERR)
  }
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xAA;         //���밲ȫģʽ
  GLOBAL_CFG &= ~bDATA_WE; //����DataFlashд����
  SAFE_MOD = 0;            //�˳���ȫģʽ
  return i;
}

/*******************************************************************************
* Function Name  : ReadDataFlash(UINT8 Addr,UINT8 len,PUINT8 buf)
* Description    : ��DataFlash
* Input          : UINT8 Addr UINT8 len PUINT8 buf
* Output         : None
* Return         : UINT8 i ����д�볤��
*******************************************************************************/
UINT8 ReadDataFlash(UINT8 Addr, UINT8 len, PUINT8 buf)
{
  UINT8 i;
  ROM_ADDR_H = DATA_FLASH_ADDR >> 8;
  Addr <<= 1;
  for (i = 0; i < len; i++)
  {
    ROM_ADDR_L = Addr + i * 2; //Addr����Ϊż��ַ
    ROM_CTRL = ROM_CMD_READ;
    //     if ( ROM_STATUS & bROM_CMD_ERR ) return( 0xFF );                        // unknown command
    *(buf + i) = ROM_DATA_L;
  }
  return i;
}