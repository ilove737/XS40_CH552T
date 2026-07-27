#ifndef __FLASH_WRITE_H__
#define __FLASH_WRITE_H__

#include "CH552.H"

#define KEYMAP_MAIN_ADDR  0x3600
#define KEYMAP_FN0_ADDR   0x3650
#define KEYMAP_WORD_CNT   40    // 每个映射 40 个字（80 字节）

void writeKeymapToFlash(UINT8 __xdata *data);
void readKeymapFromFlash(UINT8 __xdata *buf);

#endif