#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET	:=	$(shell basename $(CURDIR))
export TOPDIR	:=	$(CURDIR)

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITRO_FILES	:=

# These set the information text in the nds file
GAME_TITLE     := uxnds v0.3.6
GAME_SUBTITLE1 := tiny virtual machine
GAME_SUBTITLE2 := 18/02/2023

include $(DEVKITARM)/ds_rules

.PHONY: checkarm7 checkarm9 checkarm9debug checkarm9profile clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkarm7 checkarm9 checkarm9debug checkarm9profile $(TARGET).nds $(TARGET)_debug.nds $(TARGET)_profile.nds

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9 DEBUG=false PROFILE=false

checkarm9debug:
	$(MAKE) -C arm9 DEBUG=true PROFILE=false

checkarm9profile:
	$(MAKE) -C arm9 DEBUG=false PROFILE=true

#---------------------------------------------------------------------------------
$(TARGET).nds	: $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf assets/uxn32.bmp
	ndstool	-c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-b assets/uxn32.bmp "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)

$(TARGET)_debug.nds	: $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET)_debug.elf assets/uxn32.bmp
	ndstool	-c $(TARGET)_debug.nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET)_debug.elf \
	-b assets/uxn32.bmp "$(GAME_TITLE) (debug);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)

$(TARGET)_profile.nds	: $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET)_profile.elf assets/uxn32.bmp
	ndstool	-c $(TARGET)_profile.nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET)_profile.elf \
	-b assets/uxn32.bmp "$(GAME_TITLE) (profile);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
	$(_ADDFILES)

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9 DEBUG=false PROFILE=false

arm9/$(TARGET)_debug.elf:
	$(MAKE) -C arm9 DEBUG=true PROFILE=false

arm9/$(TARGET)_profile.elf:
	$(MAKE) -C arm9 DEBUG=false PROFILE=true

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
