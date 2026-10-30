
ifeq ($(strip $(DEVKITPRO)),)
$(error Please install devkitPro and libnds)
endif

TARGET := 99NightsDSi
BUILD := build
SOURCES := source

include $(DEVKITPRO)/libnds/base_rules

.PHONY: all clean
all: $(TARGET).nds

$(TARGET).nds: $(SOURCES)/main.c
	$(CC) -g -Wall -O2 -mthumb -mthumb-interwork \
		-I$(DEVKITPRO)/libnds/include \
		-specs=ds_arm9.specs \
		$(SOURCES)/main.c \
		-L$(DEVKITPRO)/libnds/lib -lnds9 \
		-o $(TARGET).elf
	$(DEVKITPRO)/tools/bin/ndstool \
		-c $(TARGET).nds -9 $(TARGET).elf \
		-b icon.bmp "99 Nights" "99N"
