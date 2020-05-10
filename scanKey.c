#include "CH552.H"
#include "scanKey.H"
#include "UART1.H"

#include "GPIO.H"

#include "keyMap.h"

#include <string.h>


UINT8X i = 0;
UINT8X j = 0;

UINT8X beforeAllKey[2]; // 16个位，保存上一次所有40个建的状态
UINT8X allKey[2];		 // 16个位，保存当前所有40个建的状态

UINT8X kCode;
UINT8X HIDFrames[8];
UINT8X HIDFramesPointer = 2; // 从帧的第三个字节开始添加普通按键的KeyCode


UINT8X Fn0_Status = 0;
UINT8X Fn1_Status = 0;

sbit col1 = P1^0;
sbit col2 = P1^1;
sbit col3 = P1^2;
sbit col4 = P1^3;
sbit row1 = P3^0;
sbit row2 = P3^1;
sbit row3 = P3^2;
sbit row4 = P3^3;


void initGPIO(void)
{
    // 推挽输入输出
    Port1Cfg(1,0);
    Port1Cfg(1,1); 
    Port1Cfg(1,2);
    Port1Cfg(1,3); 
    // 浮空输入，无上拉
    Port3Cfg(0,0);
    Port3Cfg(0,1); 
    Port3Cfg(0,2);
    Port3Cfg(0,3);

}

void makeHIDFrames(void)
{
	Fn0_Status = 0;
	HIDFramesPointer = 2;
	for (i = 0; i < 8; i++)
	{
		HIDFrames[i] = 0;
	}
	if (allKey[0] == 0 && allKey[1] == 0)
	{ // 所有按键都是松开的状态
		for (i = 0; i < 8; i++)
		{
			// Send_Data_To_UART0(0x00);
			UART1SendByte(0x00);

		}
		UART1SendByte('\n');

	}
	else
	{ // 有按键按下的状态
//		for (i = 0; i < 5; i++)
//		{
//			if (allKey[i] != 0)
//			{
//				for (j = 0; j < 8; j++)
//				{
//					if (allKey[i] >> j & 1)
//					{
//						if (keyMap[i * 8 + j] == KEY_Fn0)
//						{
//							Fn0_Status = 1;
//						}
//						if (keyMap[i * 8 + j] == KEY_Fn1)
//						{
//							Fn1_Status = 1;
//						}
//					}
//				}
//			}
//		}

		for (i = 0; i < 2; i++)
		{
			if (allKey[i] != 0)
			{
				for (j = 0; j < 8; j++)
				{
					if (allKey[i] >> j & 1)
					{
//						if (Fn0_Status == 1)
//						{
//							kCode = Fn0_keyMap[i * 8 + j];
//						}
//						else
//						{
//							kCode = keyMap[i * 8 + j];
//						}
						kCode = keyMap[i * 8 + j];
						// if (kCode == KEY_LCTRL | kCode == KEY_LSHIFT | kCode == KEY_LALT | kCode == KEY_LGUI | kCode == KEY_RCTRL | kCode == KEY_RSHIFT | kCode == KEY_RALT | kCode == KEY_RGUI)
						if (kCode >= 0xE0) // Control
						{
							HIDFrames[0] += 0X01 << (kCode & 0X0F);
						}
						else if (kCode >= 0xC0) // 处理shift组合键
						{
							HIDFrames[0] += 0x02;
							HIDFrames[HIDFramesPointer] = kCode - 0xa2;
							HIDFramesPointer++;
						}
						else
						{
							HIDFrames[HIDFramesPointer] = kCode;
							HIDFramesPointer++;
						}
					}
				}
			}
		}
		for (i = 0; i < 8; i++)
		{
			// Send_Data_To_UART0(HIDFrames[i]);
			UART1SendByte(0x00);
		}
		UART1SendByte('\n');
	}
}


UINT8 tttt = 0;
void scanKeyChange(void)
{
	P1 = 0x0f;
	if (tttt == 4) //所有键扫描完成
	{
		tttt = 0;
		if (memcmp(beforeAllKey, allKey, 5) != 0)
		{
			memcpy(beforeAllKey, allKey, 5);

			makeHIDFrames();
			// Send_Data_To_UART0(allKey[i]);
		}
	}
	else
	{
		if (tttt == 0)
			col1 = 0;
		if (tttt == 1)
			col2 = 0;
		if (tttt == 2)
			col3 = 0;
		if (tttt == 3)
			col4 = 0;


		allKey[tttt] = ~P3;

		tttt++;
	}
}