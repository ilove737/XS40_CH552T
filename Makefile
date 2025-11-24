# Makefile for CH552 USB Keyboard/Mouse Project
# Using SDCC (Small Device C Compiler) for Linux

# Output directory
OUTDIR = out

# Compiler settings
CC = sdcc
AS = sdas8051
OBJCOPY = objcopy
PACKIHX = packihx

# Target microcontroller
MCU = CH552

# Compiler flags
CFLAGS = -mmcs51
CFLAGS += --xram-loc 0x0000 --xram-size 0x0400 --code-size 0x3800
CFLAGS += --iram-size 256
# CFLAGS += --no-xinit-opt --xram-movc

# Source files
SOURCES_C := main.c \
             CompositeKM.C \
             Debug.C \
             Timer.C \
             GPIO.C \
			 UART1.C \
             DataFlash.C \
             scanKey.c

# Object files (in out directory)
OBJECTS_C := $(SOURCES_C:.c=.rel)
OBJECTS_C := $(OBJECTS_C:.C=.rel)
OBJECTS := $(addprefix $(OUTDIR)/,$(notdir $(OBJECTS_C)))

# Target name
TARGET = XS40_CH552T

# Default target
all: $(OUTDIR) $(OUTDIR)/$(TARGET).hex

# Compile C files
$(OUTDIR)/%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C files (uppercase extension)
$(OUTDIR)/%.rel: %.C
	$(CC) $(CFLAGS) -c $< -o $@

# Link all objects into hex file
$(OUTDIR)/$(TARGET).hex: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUTDIR)/$(TARGET).ihx $(OBJECTS)
	$(PACKIHX) $(OUTDIR)/$(TARGET).ihx > $(OUTDIR)/$(TARGET).hex

# Alternative output format
$(OUTDIR)/$(TARGET).bin: $(OUTDIR)/$(TARGET).hex
	$(OBJCOPY) -I ihex -O binary $(OUTDIR)/$(TARGET).ihx $(OUTDIR)/$(TARGET).bin

# Clean build files
clean:
	rm -rf $(OUTDIR)/*

# Install dependencies (SDCC)
install-deps:
	sudo apt-get update
	sudo apt-get install sdcc

# Flash the device (if USBasp or similar programmer is available)
flash: $(OUTDIR)/$(TARGET).hex
	wchisp flash $(OUTDIR)/$(TARGET).hex

# Show build information
info:
	@echo "Building $(TARGET) for CH552 microcontroller"
	@echo "Source files: $(SOURCES_C)"
	@echo "Output directory: $(OUTDIR)"
	@echo "SDCC version: $(shell sdcc -v | head -n1)"

.PHONY: all clean install-deps flash info $(OUTDIR)
