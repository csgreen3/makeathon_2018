.PHONY: clean all serve

DEVICE := MSP432P401R

PROJECT         := printed_circuit_car
OBJ_DIR			:= build

################## GCC Root Variable ###################
INSTALL_DIR			:= $(HOME)/bin/msp432_gcc
GCC_MSP_INC_DIR 	?= $(INSTALL_DIR)/arm/include
GCC_CMSIS_INC_DIR 	?= $(GCC_MSP_INC_DIR)/CMSIS
LDDIR				:= $(GCC_MSP_INC_DIR)/$(shell echo $(DEVICE) | tr A-Z a-z)

######################################
GCC_BIN_DIR		?= $(INSTALL_DIR)/arm_compiler/bin/
GCC_INC_DIR		?= $(INSTALL_DIR)/arm_compiler/arm-none-eabi/include
######################################
CC              := $(GCC_BIN_DIR)arm-none-eabi-gcc
GDB			    := $(GCC_BIN_DIR)arm-none-eabi-gdb
######################################
INCLUDES		:= -I $(GCC_CMSIS_INC_DIR) -I $(GCC_MSP_INC_DIR) -I $(GCC_INC_DIR)
CFLAGS			:= -mcpu=cortex-m4 -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -D__$(DEVICE)__ -DTARGET_IS_MSP432P4XX -Dgcc -g -gstrict-dwarf -Wall -ffunction-sections -fdata-sections -MD -std=c99
LDFLAGS			:= -mcpu=cortex-m4 -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -D__$(DEVICE)__ -DTARGET_IS_MSP432P4XX -Dgcc -g -gstrict-dwarf -Wall -T$(LDDIR).lds -l'c' -l'gcc' -l'nosys'
######################################

OBJECTS += $(OBJ_DIR)/startup_msp432p401r_gcc.o
OBJECTS += $(OBJ_DIR)/system_msp432p401r.o
OBJECTS += $(OBJ_DIR)/msp432p401_euscia0_uart.o
all: $(OBJ_DIR)/$(PROJECT).out

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	@mkdir -p -- $@ $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	@echo ============================================
	@echo Generating $@
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/$(PROJECT).out: $(OBJECTS)
	@echo ============================================
	@echo Linking objects and generating output binary
	$(CC) $(LDFLAGS) $^ -o $@ $(INCLUDES)

EMULATION_PATH = $(INSTALL_DIR)/emulation/common/uscif
serve:
	$(EMULATION_PATH)/gdb_agent_console -f MSP432 $(EMULATION_PATH)/xds110_msp432_jtag.dat

debug: all
	$(GDB) $(OBJ_DIR)/$(PROJECT).out

flash: all
	$(GDB) $(OBJ_DIR)/$(PROJECT).out -ex load -batch

serial%:
	screen /dev/tty$* 9600,cs8,-parenb,-cstopb,-hupcl

clean:
	@rm -rf $(OBJ_DIR)
