#include "CH552.H"
#include "UART1.H"
#include "Timer.H"
#include "Debug.H"
#include "scanKey.H"
#include "CompositeKM.H"
#include <string.h>
#include <stdio.h>

// extern UINT8 Ready;
// extern  Ep2InKey;

main()
{
    CfgFsys();   //CH559时钟选择配置
    mDelaymS(5); //修改主频等待内部晶振稳定,必加
    // mInitSTDIO(); //串口0初始化
#ifdef DE_PRINTF
    // printf("start ...\n");
#endif

    initGPIO();

    // UART1Init();

    // printf("T0 Test ...\n");
    mTimer0Clk12DivFsys();       //T0定时器时钟设置
    mTimer_x_ModInit(0, 1);      //T0 定时器模式设置
    mTimer_x_SetData(0, 0x5555); //T0定时器赋值
    mTimer0RunCTL(1);            //T0定时器启动
    ET0 = 1;                     //T0定时器中断开启
    EA = 1;


//读取芯片唯一ID号
#if DE_PRINTF
    printf("ID0 = %02x %02x \n", (UINT16) * (PUINT8C)(0x3FFA), (UINT16) * (PUINT8C)(0x3FFB));
    printf("ID1 = %02x %02x \n", (UINT16) * (PUINT8C)(0x3FFC), (UINT16) * (PUINT8C)(0x3FFD));
    printf("ID2 = %02x %02x \n", (UINT16) * (PUINT8C)(0x3FFE), (UINT16) * (PUINT8C)(0x3FFF));
#endif

    USBDeviceInit(); //USB设备模式初始化
    EA = 1;          //允许单片机中断
    UEP1_T_LEN = 0;  //预使用发送长度一定要清空
    UEP2_T_LEN = 0;  //预使用发送长度一定要清空
    FLAG = 0;
    // Ready = 0;


    while (1)
    {
        // if (Ready)
        // {
        //     HIDValueHandle(); //该函数会一直等待串口接收一个字节
        // }

        // if ((Ready) && (Ep2InKey == 0))
        // {
        //     Enp1IntIn(); //仅发送键盘键值“抬起”操作
        //     mDelaymS(10);
        // }
        // mDelaymS(10); //模拟单片机做其它事
    }

    //    for(i=0;i<128;i++){	                                                     //循环写入128字节
    //        len = WriteDataFlash(i,&i,1);                                          //向DataFlash区域偏移地址i写入i
    //        if(len != 1){
    //          printf("Write Err 次 = %02x,m = %02x\n",j,(UINT16)m);                //写出错打印
    //        }
    //      }
    //      for(i=0;i<128;i++){                                                      //读DataFlash区域偏移地址i并校验
    //        len = ReadDataFlash(i,1,&m);
    //        if((len != 1) ||(m != i)){
    //          printf("Read Err 次 = %02x, = %02x,addr =%02x ,值= %02x\n",j,(UINT16)(i*2),(UINT16)ROM_DATA_L,(UINT16)m);
    //        }                                                                      //读校验出错打印
    //      }
}
