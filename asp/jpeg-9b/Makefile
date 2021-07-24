
ROOT_PATH = ..

#
#  オブジェクトファイル名の定義
#
OBJNAME = libjpeg
OBJFILE = $(OBJNAME).a

ifeq ($(TARGET),)
TARGET = stm32f7discovery_gcc
endif

CC  =	arm-none-eabi-gcc
CXX	=	arm-none-eabi-g++
AR  =	arm-none-eabi-ar
NM  =	arm-none-eabi-nm
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

LDSCRIPT = stm32F7_rom.ld

ifeq ($(MCPU),)
MCPU = cortex-m7
endif

ifeq ($(FPU),)
FPU = fpv4
endif

COPTS   = -mcpu=$(MCPU) -Wa,--gstabs -mthumb -mthumb-interwork -mlittle-endian

ifeq ($(FPU),fpv4)
COPTS  := $(COPTS) -mfloat-abi=softfp -mfpu=fpv4-sp-d16
else
COPTS  := $(COPTS) -mfloat-abi=hard -mfpu=fpv5-d16
endif

COPTS  := $(COPTS) -nostdlib -O2 -g -Wall
CDEFS   = -DNEED_BSD_STRINGS -DTRUE=1


INCLUDES = -I.

TARGET_DIR = .
RTOS_DIR = $(ROOT_PATH)
UTASK_DIR = .
vpath %.c $(UTASK_DIR)
vpath %.S $(UTASK_DIR)

COBJS = jaricom.o jcapimin.o jcapistd.o jcarith.o jccoefct.o jccolor.o \
        jcdctmgr.o jchuff.o jcinit.o jcmainct.o jcmarker.o jcmaster.o \
        jcomapi.o jcparam.o jcprepct.o jcsample.o jctrans.o jdapimin.o \
        jdapistd.o jdarith.o jdatadst.o jdatasrc.o jdcoefct.o jdcolor.o \
        jddctmgr.o jdhuff.o jdinput.o jdmainct.o jdmarker.o jdmaster.o \
        jdmerge.o jdpostct.o jdsample.o jdtrans.o jerror.o jfdctflt.o \
        jfdctfst.o jfdctint.o jidctflt.o jidctfst.o jidctint.o jquant1.o \
        jquant2.o jutils.o jmemmgr.o jmemnobs.o \
		wrppm.o

AOBJS = 

INCLUDES := -I$(UTASK_DIR) -I$(RTOS_DIR)/monitor  -I$(RTOS_DIR) \
	-I$(RTOS_DIR)/target/$(TARGET) -I$(RTOS_DIR)/arch $(INCLUDES)
CFLAGS = $(COPTS) $(CDEFS) $(INCLUDES)
LDFLAGS := -nostdlib $(LDFLAGS)

ifdef LDSCRIPT
    LDFLAGS := $(LDFLAGS) -T $(LDSCRIPT)
endif

all:	$(OBJFILE)

$(OBJFILE):	$(COBJS) $(AOBJS)
	$(AR) -rcs $(OBJFILE) $(COBJS) $(AOBJS)

$(COBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $<

$(AOBJS): %.o: %.S
	$(CC) -c $(CFLAGS) $<

# rules

monmain.o	: $(UTASK_DIR)/monmain.c \
		$(TARGET_DIR)/sys_defs.h $(UTASK_DIR)/comdefs.h $(UTASK_DIR)/mondefs.h $(UTASK_DIR)/serial.h

monitor.o	: $(UTASK_DIR)/monitor.c \
		$(TARGET_DIR)/sys_defs.h $(UTASK_DIR)/comdefs.h $(UTASK_DIR)/mondefs.h $(UTASK_DIR)/serial.h

common.o	: $(UTASK_DIR)/common.c \
		$(TARGET_DIR)/sys_defs.h $(UTASK_DIR)/comdefs.h $(UTASK_DIR)/serial.h

serial.o	: $(UTASK_DIR)/serial.c \
		$(TARGET_DIR)/sys_defs.h $(UTASK_DIR)/comdefs.h $(UTASK_DIR)/serial.h

clean:
	rm -f *.o
	rm -f $(OBJFILE)

