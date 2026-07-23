# CH552 USB 键盘/鼠标项目 Makefile
# 使用 SDCC (Small Device C Compiler) 在 Linux 下编译
# 生成两个变体：左手布局和右手布局

# 输出目录
OUTDIR_LEFT  = out_left
OUTDIR_RIGHT = out_right

# 编译器设置
CC      = sdcc
AS      = sdas8051
OBJCOPY = objcopy
PACKIHX = packihx

# 目标微控制器
MCU = XS40_CH552T
TARGET = XS40_CH552T

# 基础编译选项（不含布局宏定义）
CFLAGS_BASE  = -mmcs51
CFLAGS_BASE += --xram-loc 0x0000 --xram-size 0x0400 --code-size 0x3800
CFLAGS_BASE += --iram-size 256

# 源文件
SOURCES_C := main.c \
             CompositeKM.C \
             Debug.C \
             Timer.C \
             GPIO.C \
             UART1.C \
             DataFlash.C \
             scanKey.c

# 目标文件列表（不含目录路径）
OBJECTS_C := $(SOURCES_C:.c=.rel)
OBJECTS_C := $(OBJECTS_C:.C=.rel)

# ============================================================================
# 模板：为一个变体生成编译规则
# $(1) = 变体名称 (left / right)
# $(2) = KEYBOARD_LAYOUT 值 (0 / 1)
# ============================================================================
define BUILD_VARIANT

OUTDIR_$(1)  = out_$(1)
CFLAGS_$(1)  = $$(CFLAGS_BASE) -DKEYBOARD_LAYOUT=$(2)
OBJECTS_$(1) = $$(addprefix $$(OUTDIR_$(1))/,$(OBJECTS_C))

# 编译 .c 文件
$$(OUTDIR_$(1))/%.rel: %.c | $$(OUTDIR_$(1))
	$$(CC) $$(CFLAGS_$(1)) -c $$< -o $$@

# 编译 .C 文件（大写扩展名）
$$(OUTDIR_$(1))/%.rel: %.C | $$(OUTDIR_$(1))
	$$(CC) $$(CFLAGS_$(1)) -c $$< -o $$@

# 链接生成 hex 固件
$$(OUTDIR_$(1))/$$(TARGET).hex: $$(OBJECTS_$(1))
	$$(CC) $$(CFLAGS_$(1)) -o $$(OUTDIR_$(1))/$$(TARGET).ihx $$(OBJECTS_$(1))
	$$(PACKIHX) $$(OUTDIR_$(1))/$$(TARGET).ihx > $$@

# 创建输出目录
$$(OUTDIR_$(1)):
	mkdir -p $$@

# 便捷目标
$(1): $$(OUTDIR_$(1))/$$(TARGET).hex

endef

# 为两个变体生成规则
$(eval $(call BUILD_VARIANT,left,0))
$(eval $(call BUILD_VARIANT,right,1))

# ============================================================================
# 顶层目标
# ============================================================================

all: left right

clean:
	rm -rf $(OUTDIR_LEFT) $(OUTDIR_RIGHT)

install-deps:
	sudo apt-get update
	sudo apt-get install sdcc

flash: left
	wchisp flash $(OUTDIR_LEFT)/$(TARGET).hex

flash-right: right
	wchisp flash $(OUTDIR_RIGHT)/$(TARGET).hex

info:
	@echo "编译 $(TARGET)，目标芯片 CH552"
	@echo "源文件: $(SOURCES_C)"
	@echo "左手布局: $(OUTDIR_LEFT)/"
	@echo "右手布局: $(OUTDIR_RIGHT)/"
	@echo "SDCC 版本: $(shell sdcc -v 2>&1 | head -n1)"

.PHONY: all clean install-deps flash flash-right info left right