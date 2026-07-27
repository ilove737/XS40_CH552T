#include "CH552.H"
#include "scanKey.h"
#include "Debug.H"
#include "GPIO.H"
#include "keyMap.h"
#include "CompositeKM.H"

#include <string.h>

UINT8 beforeAllKey[5]; // 40个位，保存上一次所有40个建的状态
UINT8 allKey[5];	   // 40个位，保存当前所有40个建的状态

UINT8 kCode;
UINT8 HIDFrames[8];
UINT8 HIDFramesPointer = 2; // 从帧的第三个字节开始添加普通按键的KeyCode
UINT8 HIDFrames0 = 0;

// UINT8X Fn0_Status = 0;
// UINT8X Fn1_Status = 0;

// sbit col1 = P1 ^ 0;
// sbit col2 = P1 ^ 1;
// sbit col3 = P1 ^ 2;
// sbit col4 = P1 ^ 3;
// sbit col5 = P1 ^ 4;
// sbit col6 = P1 ^ 5;
// sbit col7 = P1 ^ 6;
// sbit col8 = P1 ^ 7;

__sbit __at (0xB0) row1;
__sbit __at (0xB1) row2;
__sbit __at (0xB2) row3;
__sbit __at (0xB3) row4;
__sbit __at (0xB4) row5;

void initGPIO(void)
{
	// 推挽输入输出
	Port1Cfg(3, 0);
	Port1Cfg(3, 1);
	Port1Cfg(3, 2);
	Port1Cfg(3, 3);
	Port1Cfg(3, 4);
	// 浮空输入，无上拉
	Port3Cfg(1, 0);
	Port3Cfg(1, 1);
	Port3Cfg(1, 2);
	Port3Cfg(1, 3);
	Port3Cfg(1, 4);
	Port3Cfg(1, 5);
	Port3Cfg(1, 6);
	Port3Cfg(1, 7);
}

void AllKey(UINT8 Index)
{
	if (PCON & GF0)
	{
		if (Fn0_keyMap[Index][0] != 0xff)
			HIDFrames0 += Fn0_keyMap[Index][0];
		kCode = Fn0_keyMap[Index][1];
		HIDFrames[HIDFramesPointer] = kCode;
		if (HIDFramesPointer < 8 && kCode > 0)
			HIDFramesPointer++;
	}
	// else if (PCON & GF1)
	// {
	// 	if (Fn1_keyMap[Index][0] != 0xff)
	// 		HIDFrames0 += Fn1_keyMap[Index][0];
	// 	kCode = Fn1_keyMap[Index][1];
	// 	HIDFrames[HIDFramesPointer] = kCode;
	// 	if (HIDFramesPointer < 8 && kCode > 0)
	// 		HIDFramesPointer++;
	// }
	else
	{
		if (mainKeyMap[Index][0] != 0xff)
			HIDFrames0 += mainKeyMap[Index][0];
		kCode = mainKeyMap[Index][1];
		HIDFrames[HIDFramesPointer] = kCode;
		if (HIDFramesPointer < 8 && kCode > 0)
			HIDFramesPointer++;
	}
}

void FnKey(UINT8 Index)
{
	if (mainKeyMap[Index][0] == 0xff)
	{
		if (mainKeyMap[Index][1] == 0)
		{
			PCON |= GF0;
		}
		if (mainKeyMap[Index][1] == 1)
		{
			PCON |= GF1;
		}
	}
}

void makeHIDFrames(void)
{
	UINT8 i = 0;
	UINT8 f = 0;
	UINT8 Index;

	HIDFrames0 = 0;
	HIDFramesPointer = 2;

	for (i = 0; i < 8; i++)
	{
		HIDFrames[i] = 0;
	}

	// 用GF0和GF1表示Fn0和Fn1键的状态
	PCON &= (~GF0);
	PCON &= (~GF1);
	for (i = 0; i < 5; i++)
	{
		if (allKey[i] & 1)
		{
			Index = i * 8 + 0;
			FnKey(Index);
		}
		if (allKey[i] & 2)
		{
			Index = i * 8 + 1;
			FnKey(Index);
		}
		if (allKey[i] & 4)
		{
			Index = i * 8 + 2;
			FnKey(Index);
		}
		if (allKey[i] & 8)
		{
			Index = i * 8 + 3;
			FnKey(Index);
		}
		if (allKey[i] & 16)
		{
			Index = i * 8 + 4;
			FnKey(Index);
		}
		if (allKey[i] & 32)
		{
			Index = i * 8 + 5;
			FnKey(Index);
		}
		if (allKey[i] & 64)
		{
			Index = i * 8 + 6;
			FnKey(Index);
		}
		if (allKey[i] & 128)
		{
			Index = i * 8 + 7;
			FnKey(Index);
		}
	}

	for (i = 0; i < 5; i++)
	{
		if (allKey[i] & 1)
		{
			Index = i * 8 + 0;
			AllKey(Index);
		}
		if (allKey[i] & 2)
		{
			Index = i * 8 + 1;
			AllKey(Index);
		}
		if (allKey[i] & 4)
		{
			Index = i * 8 + 2;
			AllKey(Index);
		}
		if (allKey[i] & 8)
		{
			Index = i * 8 + 3;
			AllKey(Index);
		}
		if (allKey[i] & 16)
		{
			Index = i * 8 + 4;
			AllKey(Index);
		}
		if (allKey[i] & 32)
		{
			Index = i * 8 + 5;
			AllKey(Index);
		}
		if (allKey[i] & 64)
		{
			Index = i * 8 + 6;
			AllKey(Index);
		}
		if (allKey[i] & 128)
		{
			Index = i * 8 + 7;
			AllKey(Index);
		}
	}

	HIDFrames[0] = HIDFrames0;
	if (memcmp(beforeAllKey, allKey, 5) != 0)
	{
		Enp1IntInSend(HIDFrames);
		// sendKeyHID(HIDFrames);
	}
}
void scanKeyChange(void)
{
	UINT8 i = 0;
	for (i = 0; i < sumCol; i++)
	{
		// row1 = 1;
		// row2 = 1;
		// row3 = 1;
		// row4 = 1;
		// row5 = 1;
		// 五行全部拉高（以上等效）
		P3 |= 0x1f;

		if (i == 0)
			row1 = 0;
		if (i == 1)
			row2 = 0;
		if (i == 2)
			row3 = 0;
		if (i == 3)
			row4 = 0;
		if (i == 4)
			row5 = 0;
		// 逐行拉低（以上等效）
		// P3 &= (~(1 << i));

		allKey[i] = 0;
		allKey[i] |= ~P1;
	}

	if (memcmp(beforeAllKey, allKey, 5) != 0)
	{
		makeHIDFrames();
		memcpy(beforeAllKey, allKey, 5);
	}
}