#include "CH552.H"
#include "scanKey.h"
#include "Debug.H"
#include "GPIO.H"
#include "keyMap.h"
#include "CompositeKM.H"

UINT8 allKey[5];       // 当前扫描原始结果
UINT8 stableKey[5];    // 防抖确认后的稳定按键状态
UINT8 debounceBuf[5];  // 防抖缓冲区，保存上一次扫描值
UINT8 debounceCnt[5];  // 每个字节的连续一致计数器（达到阈值才确认稳定）
UINT8 HIDFrames[8];
UINT8 HIDFramesPointer = 2; // 从帧的第三个字节开始添加普通按键的KeyCode
UINT8 HIDFrames0 = 0;

// 将变量映射到指定的位地址。0xB0~0xB4 对应 P3 口的第 0~4 位（P3.0~P3.4）
__sbit __at (0xB0) row1;
__sbit __at (0xB1) row2;
__sbit __at (0xB2) row3;
__sbit __at (0xB3) row4;
__sbit __at (0xB4) row5;

void initGPIO(void)
{
	// 开漏输入输出，有上拉，内部电路可以加速由低到高的电平爬升	
	Port1Cfg(3, 0);
	Port1Cfg(3, 1);
	Port1Cfg(3, 2);
	Port1Cfg(3, 3);
	Port1Cfg(3, 4);
	// 推挽输入输出
	Port3Cfg(1, 0);
	Port3Cfg(1, 1);
	Port3Cfg(1, 2);
	Port3Cfg(1, 3);
	Port3Cfg(1, 4);
	Port3Cfg(1, 5);
	Port3Cfg(1, 6);
	Port3Cfg(1, 7);
}

/*
 * 生成 USB HID 报告帧并通过端点 1 发送
 *
 * 处理流程：
 *   1. 清零 HID 报告帧
 *   2. 第一轮遍历：检测 Fn 功能键，设置 PCON 标志位（必须在普通键之前）
 *   3. 第二轮遍历：根据当前键位映射（普通/Fn0）生成 HID 键码
 *   4. 通过 USB 端点 1 发送报告
 *
 * 注意：scanKeyChange() 中的防抖逻辑确保只有稳定状态变化时才调用此函数。
 */
void makeHIDFrames(void)
{
    UINT8 i;       // 行索引（0~4，共 5 行）
    UINT8 bit;     // 列索引（0~7，共 8 列）

    /*
     * 步骤 1：清零 HID 报告帧
     * HID 键盘报告格式：8 字节
     *   [0] 修饰键位 (Modifier)
     *   [1] 保留字节 ( Reserved)
     *   [2~7] 普通按键键码 (最多 6 个按键)
     */
    for (i = 0; i < 8; i++)
    {
        HIDFrames[i] = 0;
    }
    HIDFrames0 = 0;          // 修饰键累加器清零
    HIDFramesPointer = 2;    // 普通键码从第 3 字节开始写入

    /*
     * 步骤 2：处理 Fn 功能键
     * Fn 键的键位映射中 [0] = 0xff，[1] = 0/1 表示 Fn0/Fn1。
     * 检测到 Fn 键按下时设置 PCON 中的 GF0/GF1 标志位，
     * 后续步骤 3 将根据这些标志选择对应的键位映射表。
     * 必须在步骤 3 之前完成，因为普通键需要知道当前使用哪个层。
     *
     * 这里将原 FnKey() 函数内联展开，与 AllKey 的处理方式保持一致。
     */
    PCON &= (~GF0); // 清零 Fn0 层标志位
    PCON &= (~GF1); // 清零 Fn1 层标志位
    for (i = 0; i < 5; i++)
    {
        if (allKey[i])	// 如果当前行有按键按下
        {
            for (bit = 0; bit < 8; bit++)
            {
                if (allKey[i] & (1 << bit))
                {
                    UINT8 idx = i * 8 + bit;
                    /*
                     * Fn 键的识别方式：
                     *   mainKeyMap[idx][0] == 0xff 表示该键是功能键
                     *   mainKeyMap[idx][1] == 0    表示 Fn0 层
                     *   mainKeyMap[idx][1] == 1    表示 Fn1 层
                     */
                    if (mainKeyMap[idx][0] == KEY_FnX)
                    {
                        if (mainKeyMap[idx][1] == 0) PCON |= GF0;
                        if (mainKeyMap[idx][1] == 1) PCON |= GF1;
                    }
                }
            }
        }
    }

    /*
     * 步骤 3：处理普通按键
     * 根据当前 Fn 层状态（PCON.GF0/GF1），从对应的键位映射表中
     * 查出 HID 键码和修饰键，填入 HIDFrames。
     */
    {
        UINT8C (*keyMap)[2];  // 指向当前选中的键位映射表（Flash 地址）
        UINT8 index;          // 按键索引（0~39）
        UINT8 mod;            // 修饰键位值
        UINT8 code;           // 按键键码

        // 根据 Fn 层状态选择键位映射表
        if (PCON & GF0)
            keyMap = Fn0_keyMap;
        else
            keyMap = mainKeyMap;

        for (i = 0; i < 5; i++)
        {
            if (allKey[i])
            {
                for (bit = 0; bit < 8; bit++)
                {
                    if (allKey[i] & (1 << bit))
                    {
                        index = i * 8 + bit;
                        mod = keyMap[index][0];
                        code = keyMap[index][1];

                        if (mod != KEY_FnX)	// 修饰键位累加（KEY_FnX 表示该键不是修饰键）
                            HIDFrames0 += mod;

                        if (HIDFramesPointer < 8 && code > 0)	// 写入普通键码，最多 6 个（HIDFrames[2] ~ HIDFrames[7]）
                        {
                            HIDFrames[HIDFramesPointer] = code;
                            HIDFramesPointer++;
                        }
                    }
                }
            }
        }
    }

    /*
     * 步骤 4：发送 HID 报告
     * HIDFrames0 记录了所有修饰键（Ctrl/Shift/Alt/Win）的累加值，
     * 填到帧的第一个字节，然后通过 USB 端点 1 发送。
     */
    HIDFrames[0] = HIDFrames0;
    Enp1IntInSend(HIDFrames);
    // sendKeyHID(HIDFrames);
}
/*
 * 扫描矩阵键盘 + 按键防抖处理
 * 该函数由定时器 T0 中断（每 2ms）调用，执行以下步骤：
 *   1. 逐行拉低 P3.0~P3.4，读取 P1 口列状态，存入 allKey[5]
 *   2. 对每个字节进行防抖处理：
 *      - 若本次采样值与 debounceBuf 一致，递增 debounceCnt
 *      - 若不一致，重置 debounceBuf 和 debounceCnt
 *      - 当 debounceCnt 达到 DEBOUNCE_THRESHOLD 时，将稳定状态
 *        更新到 stableKey，并记录变化标志
 *   3. 只有当 stableKey 有变化时，才调用 makeHIDFrames() 发送 HID 报告
 */
void scanKeyChange(void)
{
    UINT8 i = 0;
    UINT8 changed = 0;  // 稳定状态变化标志

    /*
     * 步骤 1：逐行扫描矩阵键盘
     * 先将 5 行全部拉高，再依次将当前行拉低，
     * 读取 P1 口（8 位列）的状态，取反后存入 allKey。
     */
    for (i = 0; i < 5; i++)
    {
        P3 |= 0x1f;			// 五行全部拉高
		P3 &= (~(1 << i));	// 逐行拉低
		
        allKey[i] = 0;
        allKey[i] |= ~P1;
    }

    /*
     * 步骤 2：对每个字节做防抖确认
     *
     * 防抖原理：连续 N 次（DEBOUNCE_THRESHOLD）采样结果一致，
     * 才认为该字节的按键状态已经稳定，避免触点弹跳导致的误触发。
     *
     * 以定时器 2ms 中断一次为例：
     *   DEBOUNCE_THRESHOLD = 3 → 约 6ms 后确认状态
     *   DEBOUNCE_THRESHOLD = 5 → 约 10ms 后确认状态
     */
    for (i = 0; i < 5; i++)
    {
        if (allKey[i] == debounceBuf[i])
        {
            /*
             * 本次采样与上一次一致：递增计数器
             * 限制最大值防止溢出，同时确保只有达到阈值时才更新
             */
            if (debounceCnt[i] < DEBOUNCE_THRESHOLD)
            {
                debounceCnt[i]++;
            }

            /*
             * 连续采样达到阈值，且与当前稳定状态不同时，
             * 更新稳定状态并标记变化
             */
            if (debounceCnt[i] >= DEBOUNCE_THRESHOLD && stableKey[i] != allKey[i])
            {
                stableKey[i] = allKey[i];
                changed = 1;
            }
        }
        else
        {
            /*
             * 本次采样与上一次不一致：说明可能处于抖动中，
             * 重置缓冲区为当前值，计数器归零，重新开始计数
             */
            debounceBuf[i] = allKey[i];
            debounceCnt[i] = 0;
        }
    }

    /*
     * 步骤 3：只有稳定状态发生变化时，才生成并发送 HID 报告
     * 这比原始代码（每次扫描变化都触发）减少了大量因抖动引起的冗余报告
     */
    if (changed)
        makeHIDFrames();
}