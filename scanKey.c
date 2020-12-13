#include "CH552.H"
#include "scanKey.h"
// #include "UART1.H"
#include "Debug.H"
#include "GPIO.H"
#include "DataFlash.H"
#include "keyMap.h"
#include "CompositeKM.H"

#include <string.h>
#include <stdio.h>

UINT8X len = 0;

UINT8X i = 0;
UINT8X j = 0;

UINT8 beforeAllKey[5]; // 40个位，保存上一次所有40个建的状态
UINT8 allKey[5];	   // 40个位，保存当前所有40个建的状态

int q = 0;
// UINT8 k = 5;

UINT8 kCode;
UINT8 HIDFrames[8];
UINT8 HIDFramesPointer = 2; // 从帧的第三个字节开始添加普通按键的KeyCode

// UINT8X Fn0_Status = 0;
// UINT8X Fn1_Status = 0;

sbit col1 = P1 ^ 0;
sbit col2 = P1 ^ 1;
sbit col3 = P1 ^ 2;
sbit col4 = P1 ^ 3;
sbit col5 = P1 ^ 4;
sbit col6 = P1 ^ 5;
sbit col7 = P1 ^ 6;
sbit col8 = P1 ^ 7;

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

void makeHIDFrames(void)
{
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
		// 		HIDFrames[0] = keyMap[i * j * 8 + 0][0];
		// 		kCode = keyMap[i * j * 8 + 0][1];
		// 		HIDFrames[HIDFramesPointer] = kCode;
		// 		if (HIDFramesPointer < 7 && kCode > 0)
		// 			HIDFramesPointer++;
		// 	}
		// }
		
		if ((allKey[i] >> 0) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 0][0];
			kCode = keyMap[i * 8 + 0][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 1) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 1][0];
			kCode = keyMap[i * 8 + 1][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 2) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 2][0];
			kCode = keyMap[i * 8 + 2][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 3) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 3][0];
			kCode = keyMap[i * 8 + 3][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 4) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 4][0];
			kCode = keyMap[i * 8 + 4][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 5) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 5][0];
			kCode = keyMap[i * 8 + 5][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 6) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 6][0];
			kCode = keyMap[i * 8 + 6][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		if ((allKey[i] >> 7) & 1)
		{
			HIDFrames[0] = keyMap[i * 8 + 7][0];
			kCode = keyMap[i * 8 + 7][1];
			HIDFrames[HIDFramesPointer] = kCode;
			if (HIDFramesPointer < 7 && kCode > 0)
				HIDFramesPointer++;
		}
		
	}

	sendKeyHID(HIDFrames);
}

void scanKeyChange(void)
{
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
		makeHIDFrames();
	}
}

void readDataFlash(void)
{

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
			len = ReadDataFlash(i * 2 + j, 1, &keyMap[i][j]);
			if (len != 1)
			{
				// printf("Write Err 次 = %02x,m = %02x\n",j,(UINT16)m);                //写出错打印
			}
		}
	}
}