
TARGET := 99NightsDSi
BUILD := build
SOURCES := source

ARCH := -mthumb -mthumb-interwork
CFLAGS := -g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH)
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,--gc-sections
LIBS := -lnds9

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/$(SOURCES)
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(notdir $(wildcard $(SOURCES)/*.c))
export OFILES := $(CFILES:.c=.o)
export INCLUDE := -I$(CURDIR)/$(SOURCES)
.PHONY: all clean
all: $(BUILD)
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	rm -rf $(BUILD) $(TARGET).nds $(TARGET).elf
else
DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).nds: $(OUTPUT).elf
	$(NDSTOOL) -c $@ -9 $<

$(OUTPUT).elf: $(OFILES)
	$(CC) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

-include $(DEPENDS)
endif
