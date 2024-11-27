# Tool macros
CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
LD := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
OBJDUMP := arm-none-eabi-objdump
OPT		:= -Os
DEBUG	:= -ggdb3
CSTD	?= -std=c99
CXXSTD 	?= -std=c++14

# Project settings
BINARY = main
BUILD_PATH = ./build

# Directories
OPENCM3_DIR ?= ./libopencm3
AI_DIR ?= ./AI   # Include the entire AI directory

ifeq ($(strip $(OPENCM3_DIR)),)
$(error "Cannot find libopencm3 library. Please set OPENCM3_DIR.")
endif

ifeq ($(strip $(AI_DIR)),)
$(error "Cannot find AI directory. Please set AI_DIR.")
endif

# Define STM32 family
STM32_FAMILY ?= STM32F4
DEFS += -D$(STM32_FAMILY)

# Architecture and FPU flags
ARCH_FLAGS = -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 

# Include paths
DEFS += -I$(OPENCM3_DIR)/include
DEFS += -I$(AI_DIR)


# Compiler flags
CFLAGS += -g -fno-unwind-tables
CFLAGS += -fno-exceptions -fno-asynchronous-unwind-tables

CXXFLAGS += -g -fno-unwind-tables
CXXFLAGS +=   -fno-exceptions  -fno-asynchronous-unwind-tables -fno-use-cxa-atexit

###############################################################################
# C flags

TGT_CFLAGS	+= $(OPT) $(CSTD) $(DEBUG)
TGT_CFLAGS	+= $(ARCH_FLAGS)
TGT_CFLAGS	+= -Wextra -Wshadow -Wimplicit-function-declaration
TGT_CFLAGS	+= -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
TGT_CFLAGS	+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C++ flags

TGT_CXXFLAGS	+= $(OPT) $(CXXSTD) $(DEBUG)
TGT_CXXFLAGS	+= $(ARCH_FLAGS)
TGT_CXXFLAGS	+= -Wextra -Wshadow -Wredundant-decls  -Weffc++
TGT_CXXFLAGS	+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C & C++ preprocessor common flags

TGT_CPPFLAGS	+= -MD
TGT_CPPFLAGS	+= -Wall -Wundef
TGT_CPPFLAGS	+= $(DEFS)

###############################################################################
# Linker flags




# Library linking
LDFLAGS += $(ARCH_FLAGS) -L$(OPENCM3_DIR)/lib -Wl,--gc-sections --specs=nosys.specs
LDFLAGS += --static -nostartfiles -Wl,-Map=$(BUILD_PATH)/$(BINARY).map -Wl,--cref

LDLIBS += -lopencm3_stm32f4 -lm -lstdc++
LDLIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group


# Linker script
ifeq ($(strip $(DEVICE)),)
LDSCRIPT = stm32f429i-discovery.ld
else
include $(OPENCM3_DIR)/mk/genlink-config.mk
endif

LDFLAGS += -T$(LDSCRIPT)

# Collect all source files
# Include C source code for required libraries

# Include C++ source code for required libraries
CXXSOURCES += 	$(wildcard tflite-model/*.cpp)
CSOURCES += ./AI/edge-impulse-sdk/porting/ei_classifier_porting.c

CORE = ./AI/edge-impulse-sdk/CMSIS/Core
CSOURCES += $(shell find $(CORE) -name "*.c")
CXXSOURCES += $(shell find $(CORE) -name "*.cpp")

SOURCEDSP = ./AI/edge-impulse-sdk/CMSIS/DSP
CSOURCES += $(shell find $(SOURCEDSP) -name "*.c")
CXXSOURCES += $(shell find $(SOURCEDSP) -name "*.cpp")

NN = ./AI/edge-impulse-sdk/CMSIS/NN
CSOURCES += $(shell find $(NN) -name "*.c")
CXXSOURCES += $(shell find $(NN) -name "*.cpp")

DSP = ./AI/edge-impulse-sdk/dsp
CSOURCES += $(shell find $(DSP) -name "*.c")
CXXSOURCES += $(shell find $(DSP) -name "*.cpp")

TENSORFLOW = ./AI/edge-impulse-sdk/tensorflow/
CSOURCES += $(shell find $(TENSORFLOW) -name "*.c")
CCSOURCES += $(shell find $(TENSORFLOW) -name "*.cc")

CLASSIFIER = ./AI/edge-impulse-sdk/classifier 
CSOURCES += $(shell find $(CLASSIFIER) -name "*.c")
CXXSOURCES += $(shell find $(CLASSIFIER) -name "*.cpp") main.cpp

# Object files
C_OBJECTS = $(patsubst %.c,%.o,$(CSOURCES)) 
CXX_OBJECTS = $(patsubst %.cpp,%.o,$(CXXSOURCES))
CC_OBJECTS = $(patsubst %.cc,%.o,$(CCSOURCES))

OBJS = $(C_OBJECTS) $(CXX_OBJECTS) $(CC_OBJECTS)



# Build rules
.PHONY: all
all: $(BUILD_PATH)/$(BINARY).elf

$(BUILD_PATH)/$(BINARY).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p $(BUILD_PATH)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	@#printf "  CC      $(*).c\n"
	$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).c

%.o: %.cxx
	@#printf "  CXX     $(*).cxx\n"
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cxx

%.o: %.cpp
	@#printf "  CXX     $(*).cpp\n"
	$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $(*).o -c $(*).cpp

# Clean build files
.PHONY: clean
clean:
	rm -rf $(BUILD_PATH) $(C_OBJECTS) $(CXX_OBJECTS)
	find . -type f -name '*.d' -exec rm -f {} +
	find . -type f -name 'firmware.bin' -exec rm -f {} +


# Flash binary to the device
.PHONY: flash
flash: $(BUILD_PATH)/$(BINARY).elf
	st-flash write $(BUILD_PATH)/$(BINARY).elf 0x8000000
