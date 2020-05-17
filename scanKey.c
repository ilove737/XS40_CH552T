#include "CH552.H"
#include "scanKey.h"
#include "UART1.H"
#include "Debug.H"
#include "GPIO.H"
#include "keyMap.h"
#include "CompositeKM.H"

#include <string.h>
#include <stdio.h>

UINT8X i = 0;
UINT8X j = 0;

UINT8 beforeAllKey[2]; // 16个位，保存上一次所有16个建的状态
UINT8 allKey[2];	   // 16个位，保存当前所有16个建的状态

int q = 0;
UINT8 k = 5;

UINT8 kCode;
UINT8 HIDFrames[8];
UINT8 HIDFramesPointer = 2; // 从帧的第三个字节开始添加普通按键的KeyCode

// UINT8X Fn0_Status = 0;
// UINT8X Fn1_Status = 0;

sbit col1 = P1 ^ 0;
sbit col2 = P1 ^ 1;
sbit col3 = P1 ^ 2;
sbit col4 = P1 ^ 3;
sbit row1 = P3 ^ 0;
sbit row2 = P3 ^ 1;
sbit row3 = P3 ^ 2;
sbit row4 = P3 ^ 3;

void initGPIO(void)
{
	// 推挽输入输出
	Port1Cfg(3, 0);
	Port1Cfg(3, 1);
	Port1Cfg(3, 2);
	Port1Cfg(3, 3);
	// 浮空输入，无上拉
	Port3Cfg(1, 0);
	Port3Cfg(1, 1);
	Port3Cfg(1, 2);
	Port3Cfg(1, 3);
}

void makeHIDFrames(void)
{
	// Fn0_Status = 0;
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
		// UART1SendByte(0xf1);
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
		// for (q = 0; q < 8; q++)
		// {
		// 	UART1SendByte(0x44);
		// 	UART1SendByte(q);
		// 	UART1SendByte(k);
		// 	UART1SendByte(k >> 1);
		// 	UART1SendByte(k >> 2);
		// 	UART1SendByte((k >> q) & 1);
		// }
		for (i = 0; i < 2; i++)
		{
			if (allKey[i] != 0)
			{
				if ((allKey[i] >> 0) & 1)
				{
					kCode = keyMap[i * 8 + 0];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 1) & 1)
				{
					kCode = keyMap[i * 8 + 1];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 2) & 1)
				{
					kCode = keyMap[i * 8 + 2];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 3) & 1)
				{
					kCode = keyMap[i * 8 + 3];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 4) & 1)
				{
					kCode = keyMap[i * 8 + 4];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 5) & 1)
				{
					kCode = keyMap[i * 8 + 5];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 6) & 1)
				{
					kCode = keyMap[i * 8 + 6];
					HIDFrames[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}
				if ((allKey[i] >> 7) & 1)
				{
					kCode = keyMap[i * 8 + 7];
					HIDFrames[HIDFramesPointer] = kCode;
					// HIDKey[HIDFramesPointer] = kCode;
					if (HIDFramesPointer < 7)
						HIDFramesPointer++;
				}

				// for (j = 0; j < 8; j++)
				// {
				// 	if ((allKey[i] >> j) & 1 == 1)
				// 	{
				// 		//						if (Fn0_Status == 1)
				// 		//						{
				// 		//							kCode = Fn0_keyMap[i * 8 + j];
				// 		//						}
				// 		//						else
				// 		//						{
				// 		//							kCode = keyMap[i * 8 + j];
				// 		//						}
				// 		kCode = keyMap[i * 8 + j];

				// 		// if (kCode == KEY_LCTRL | kCode == KEY_LSHIFT | kCode == KEY_LALT | kCode == KEY_LGUI | kCode == KEY_RCTRL | kCode == KEY_RSHIFT | kCode == KEY_RALT | kCode == KEY_RGUI)
				// 		// if (kCode >= 0xE0) // Control
				// 		// {
				// 		// 	HIDFrames[0] += 0X01 << (kCode & 0X0F);
				// 		// }
				// 		// else if (kCode >= 0xC0) // 处理shift组合键
				// 		// {
				// 		// 	HIDFrames[0] += 0x02;
				// 		// 	HIDFrames[HIDFramesPointer] = kCode - 0xa2;
				// 		// 	HIDFramesPointer++;
				// 		// }
				// 		// else
				// 		// {
				// 		HIDFrames[HIDFramesPointer] = kCode;
				// 		if (HIDFramesPointer < 7)
				// 			HIDFramesPointer++;
				// 		// }
				// 	}
				// }
			}
		}
		for (i = 0; i < 8; i++)
		{
			// Send_Data_To_UART0(HIDFrames[i]);
			UART1SendByte(HIDFrames[i]);
			// HIDKey[i] = HIDFrames[i];
		}
		// UART1SendByte('\n');
	}
}

void scanKeyChange(void)
{
	allKey[0] = 0;
	allKey[1] = 0;
	for (i = 0; i < sumCol; i++)
	{
		row1 = 1;
		row2 = 1;
		row3 = 1;
		row4 = 1;

		switch (i)
		{
		case 0:
			row1 = 0;
			allKey[0] |= (~P1 & 0x0f) << 0;
			break;
		case 1:
			row2 = 0;
			allKey[0] |= (~P1 & 0x0f) << 4;
			break;
		case 2:
			row3 = 0;
			allKey[1] |= (~P1 & 0x0f) << 0;
			break;
		case 3:
			row4 = 0;
			allKey[1] |= (~P1 & 0x0f) << 4;
			break;
			// default:
			// break;
		}
		// mDelayuS(200);
	}

	if (memcmp(beforeAllKey, allKey, 2) != 0)
	{
		// UART1SendByte(beforeAllKey[0]);
		// UART1SendByte(beforeAllKey[1]);
		// UART1SendByte(allKey[0]);
		// UART1SendByte(allKey[1]);
		// UART1SendByte(0xff);

		memcpy(beforeAllKey, allKey, 2);
		// beforeAllKey[0] = allKey[0];
		// beforeAllKey[1] = allKey[1];

		makeHIDFrames();
		// UART1SendByte(beforeAllKey[0]);
		// UART1SendByte(beforeAllKey[1]);
		// UART1SendByte(0xfe);
	}
}