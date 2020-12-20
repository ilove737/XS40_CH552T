#include "CH552.H"
#include "scanKey.h"
#include "Debug.H"
#include "GPIO.H"
#include "DataFlash.H"
#include "keyMap.h"
#include "CompositeKM.H"

#include <string.h>
#include <stdio.h>

UINT8X len = 0;

// UINT8X i = 0;
// UINT8X j = 0;
// UINT8X f = 0;

UINT8 beforeAllKey[5]; // 40个位，保存上一次所有40个建的状态
UINT8 allKey[5];	   // 40个位，保存当前所有40个建的状态

// UINT8 k = 5;

UINT8 kCode;
UINT8 HIDFrames[8];
UINT8 HIDFramesPointer = 2; // 从帧的第三个字节开始添加普通按键的KeyCode

UINT8X Fn0_Status = 0;
UINT8X Fn1_Status = 0;

// sbit col1 = P1 ^ 0;
// sbit col2 = P1 ^ 1;
// sbit col3 = P1 ^ 2;
// sbit col4 = P1 ^ 3;
// sbit col5 = P1 ^ 4;
// sbit col6 = P1 ^ 5;
// sbit col7 = P1 ^ 6;
// sbit col8 = P1 ^ 7;

sbit row1 = P3 ^ 0;
sbit row2 = P3 ^ 1;
sbit row3 = P3 ^ 2;
sbit row4 = P3 ^ 3;
sbit row5 = P3 ^ 4;

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

void FnKey1(UINT8 R, UINT8 C)
{
	// HIDFrames[7] = C;
	HIDFrames[6] = R;
	// if (HIDFramesPointer < 8)
	// {
	// 	if (mainKeyMap[R * 8 + C][1])
	// 	{
	// 		if (mainKeyMap[R * 8 + C][0] != 0xff)
	// 		{

	// 			HIDFrames[0] = mainKeyMap[R * 8 + C][0];
	// 			HIDFrames[HIDFramesPointer] = mainKeyMap[R * 8 + C][1];
	// 			HIDFramesPointer++;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		HIDFrames[0] = mainKeyMap[R * 8 + C][0];
	// 	}
	// }
}

void FnKey(int ii, int jj)
{
	UINT8 Index;
	Index = Index;
	HIDFrames[5] = ii;
	HIDFrames[6] = jj;

	HIDFrames[7] = Index;

	if (Fn0_Status)
	{
		if (HIDFramesPointer < 8 && Fn0_keyMap[Index][1] > 0 && Fn0_keyMap[Index][0] != 0xff)
		{
			HIDFrames[0] = Fn0_keyMap[Index][0];
			HIDFrames[HIDFramesPointer] = Fn0_keyMap[Index][1];
			HIDFramesPointer++;
		}
	}
	// else if (Fn1_Status){
	// 	HIDFrames[0] = Fn1_keyMap[i * 8 + 0][0];
	// 	HIDFrames[HIDFramesPointer] = Fn1_keyMap[i * 8 + 0][1];
	// 	if (HIDFramesPointer < 8 && Fn1_keyMap[i * 8 + 0][1] > 0)
	// 		HIDFramesPointer++;
	// }
	else
	{
		if (HIDFramesPointer < 8 && mainKeyMap[Index][1] > 0 && mainKeyMap[Index][0] != 0xff)
		{
			HIDFrames[0] = mainKeyMap[Index][0];
			HIDFrames[HIDFramesPointer] = mainKeyMap[Index][1];
			HIDFramesPointer++;
		}
	}
}
void makeHIDFrames(void)
{

	UINT8 i = 0;
	UINT8 f = 0;
	UINT8 Index;

	Fn0_Status = 0;
	Fn1_Status = 0;
	for (f = 0; f < 40; f++)
	{
		if ((allKey[f / 8] >> (f % 8)) & 1)
		{
			if (mainKeyMap[f][0] == 0xff)
			{
				if (mainKeyMap[f][1] == 0)
				{
					Fn0_Status = 1;
				}
				if (mainKeyMap[f][1] == 1)
				{
					Fn1_Status = 1;
				}
			}
		}
	}

	// Fn0_Status = 0;
	HIDFramesPointer = 2;
	for (i = 0; i < 8; i++)
	{
		HIDFrames[i] = 0;
	}

	for (i = 0; i < 5; i++)
	{
		// for (j = 0; j < 8; j++)
		// {
		// 	if ((allKey[i] >> j) & 1)
		// 	{
		// 		HIDFrames[0] = mainKeyMap[i * 8 + j][0];
		// 		kCode = mainKeyMap[i * 8 + j][1];
		// 		HIDFrames[HIDFramesPointer] = kCode;
		// 		if (HIDFramesPointer < 8 && kCode > 0)
		// 			HIDFramesPointer++;
		// 	}
		// }
		if ((allKey[i] >> 0) & 1)
		{
			Index = i * 8 + 0;
			if (Fn0_Status & Fn0_keyMap[Index][0] != 0xff)
			{
				if (HIDFramesPointer < 8)
				{
					HIDFrames[0] = Fn0_keyMap[Index][0];
					HIDFrames[HIDFramesPointer] = Fn0_keyMap[Index][1];
					HIDFramesPointer++;
				}
			}
			// else if (Fn1_Status){
			// 	if (HIDFramesPointer < 8 && Fn1_keyMap[Index][1] > 0 && Fn1_keyMap[Index][0] != 0xff)
			// 	{
			// 		HIDFrames[0] = Fn1_keyMap[Index][0];
			// 		HIDFrames[HIDFramesPointer] = Fn1_keyMap[Index][1];
			// 		HIDFramesPointer++;
			// 	}
			// }
			else
			{
				// if (mainKeyMap[Index][1] == 0){
				// 	HIDFrames[0] = mainKeyMap[Index][0];

				// }else
				// if (HIDFramesPointer < 8 )
				// {
				HIDFrames[0] = mainKeyMap[Index][0];
				HIDFrames[2] = mainKeyMap[Index][1];
				HIDFramesPointer++;
				// }
			}
			// HIDFrames[2] = mainKeyMap[Index][1];
			// HIDFrames[3] = HIDFramesPointer;

			// HIDFramesPointer++;
			// HIDFrames[4] = HIDFramesPointer;
		}
		if ((allKey[i] >> 1) & 1)
		{
			Index = i * 8 + 1;
			if (Fn0_Status && Fn0_keyMap[Index][0] != 0xff)
			{
				if (HIDFramesPointer < 8)
				{
					HIDFrames[0] = Fn0_keyMap[Index][0];
					HIDFrames[HIDFramesPointer] = Fn0_keyMap[Index][1];
					HIDFramesPointer++;
				}
			}
			// else if (Fn1_Status){
			// 	if (HIDFramesPointer < 8 && Fn1_keyMap[Index][1] > 0 && Fn1_keyMap[Index][0] != 0xff)
			// 	{
			// 		HIDFrames[0] = Fn1_keyMap[Index][0];
			// 		HIDFrames[HIDFramesPointer] = Fn1_keyMap[Index][1];
			// 		HIDFramesPointer++;
			// 	}
			// }
			else
			{
				if (HIDFramesPointer < 8 && mainKeyMap[Index][1] > 0)
				{
					HIDFrames[0] = mainKeyMap[Index][0];
					HIDFrames[HIDFramesPointer] = mainKeyMap[Index][1];
					HIDFramesPointer++;
				}
			}
		}
		if ((allKey[i] >> 2) & 1)
		{
			// FnKey(i, 2);

			HIDFrames[0] = mainKeyMap[i * 8 + 2][0];
			kCode = mainKeyMap[i * 8 + 2][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 8 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 3) & 1)
		{
			// FnKey(i, 3);

			HIDFrames[0] = mainKeyMap[i * 8 + 3][0];
			kCode = mainKeyMap[i * 8 + 3][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 8 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 4) & 1)
		{
			// FnKey(i, 4);

			HIDFrames[0] = mainKeyMap[i * 8 + 4][0];
			kCode = mainKeyMap[i * 8 + 4][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 8 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 5) & 1)
		{
			// FnKey(i, 5);

			HIDFrames[0] = mainKeyMap[i * 8 + 5][0];
			kCode = mainKeyMap[i * 8 + 5][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 8 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 6) & 1)
		{
			// FnKey(i, 6);

			HIDFrames[0] = mainKeyMap[i * 8 + 6][0];
			kCode = mainKeyMap[i * 8 + 6][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 8 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 7) & 1)
		{
			// FnKey(i, 7);

			HIDFrames[0] = mainKeyMap[i * 8 + 7][0];
			kCode = mainKeyMap[i * 8 + 7][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 8 && kCode > 0)
				HIDFramesPointer++;
		}
	}

	sendKeyHID(HIDFrames);
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
		memcpy(beforeAllKey, allKey, 5);
		// 使用通用标志位 0 标识按键有变动
		PCON |= GF0;
	}
	else
	{
		// 使用通用标志位 0 标识按键无变动
		PCON &= (~GF0);
	}
}

void scanKeyChange0(void)
{
	UINT8 i = 0;

	allKey[0] = 0;
	allKey[1] = 0;
	allKey[2] = 0;
	allKey[3] = 0;
	allKey[4] = 0;
	for (i = 0; i < sumCol; i++)
	{
		row1 = 1;
		row2 = 1;
		row3 = 1;
		row4 = 1;
		row5 = 1;

		switch (i)
		{
		case 0:
			row1 = 0;
			allKey[0] |= (~P1 & 0xff);
			break;
		case 1:
			row2 = 0;
			allKey[1] |= (~P1 & 0xff);
			break;
		case 2:
			row3 = 0;
			allKey[2] |= (~P1 & 0xff);
			break;
		case 3:
			row4 = 0;
			allKey[3] |= (~P1 & 0xff);
			break;
		case 4:
			row5 = 0;
			allKey[4] |= (~P1 & 0xff);
			break;
			// default:
			// break;
		}
		// mDelayuS(200);
	}

	if (memcmp(beforeAllKey, allKey, 5) != 0)
	{
		memcpy(beforeAllKey, allKey, 5);
		// makeHIDFrames();
		// 使用通用标志位 0 标识按键有变动
		PCON |= GF0;
	}
	else
	{
		// 使用通用标志位 0 标识按键无变动
		PCON &= (~GF0);
	}
}

void readDataFlash(void)
{

	UINT8X i, j;
	// for(k=0;k<128;k++){	                                                     //循环写入128字节
	// 	len = WriteDataFlash(k,&k,1);                                          //向DataFlash区域偏移地址i写入i
	// 	// len = ReadDataFlash(i,1,&m);
	// 	if(len != 1){
	// 		// printf("Write Err 次 = %02x,m = %02x\n",j,(UINT16)m);                //写出错打印
	// 	}
	// }

	// for (i = 0; i < 16; i++)
	// {
	//     for (j = 0; j < 2; j++)
	//     {
	//         len = WriteDataFlash(i*2+j, &keyMap[i][j], 1); //向DataFlash区域偏移地址i写入i     &keyMap[i][j]
	//         if (len != 1)
	//         {
	//             // printf("Write Err 次 = %02x,m = %02x\n",j,(UINT16)m);                //写出错打印
	//         }
	//     }
	// }

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 2; j++)
		{
			// len = WriteDataFlash(i*2+j, &keyMap[i][j], 1); //向DataFlash区域偏移地址i写入i     &keyMap[i][j]
			len = ReadDataFlash(i * 2 + j, 1, &mainKeyMap[i][j]);
			if (len != 1)
			{
				// printf("Write Err 次 = %02x,m = %02x\n",j,(UINT16)m);                //写出错打印
			}
		}
	}
}