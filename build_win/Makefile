TARGET_BASE_NAME 	= encrypt
CC 					= gcc
LIBS 				= -lpthread -lcurses
CFLAGS 				= -Wall -Wno-unused-function -O3
SRC_DIR     		= ../src
OBJ_DIR 			= ./object
OBJS 				= common.o str.o decoder.o control.o encryption.o core_code.o extra.o main.o
DEF       			= -D__WINDOWS \
					  -D__64BIT \
					  -D__DEBUG \
					  -DPDC_WIDE \
					  -DPDC_FORCE_UTF8 \
					  -D__CLEANUP_C \
					  -DHAVE_PTW32_CONFIG_H
INCS      			= -I../include \
					  -I../lib/win/pthreads/include \
					  -I../lib/win/pdcurses/include
LIBS_DEPS 			=

ifeq ($(DLL), Y)
LIB_DIR     		= .
#relative directory of libraries, used in building libraries
LIB_REL_DIR 		= ../../../build_win
else
LIB_DIR     		= ../lib/win
LIB_REL_DIR 		= ../
endif

LIBS 				+= -L$(LIB_DIR)

ifneq ($(PLATFORM),)
	PLATFORM_NAME = _$(PLATFORM)
else
	PLATFORM_NAME = _win
endif
ifneq ($(PTHREAD_NUM_MAX),)
	DEF += -DPTHREAD_NUM_MAX=$(PTHREAD_NUM_MAX)
	PTHREAD_NUM_MAX_NAME = _$(PTHREAD_NUM_MAX)th
else
	PTHREAD_NUM_MAX_NAME = _1th
endif
ifneq ($(FREE_MEMORY),)
	DEF += -DFREE_MEMORY=$(FREE_MEMORY)
	FREE_MEMORY_NAME = _$(FREE_MEMORY)Mb
else
	ifneq ($(findstring $(PTHREAD_NUM_MAX), 2 4 6 8),)
		FREE_MEMORY_NAME = _$(PTHREAD_NUM_MAX)0Mb
	else
		FREE_MEMORY_NAME = _10Mb
	endif
endif
TARGET = $(TARGET_BASE_NAME)$(PLATFORM_NAME)$(PTHREAD_NUM_MAX_NAME)$(FREE_MEMORY_NAME)

ifndef TARGET_DIR
	TARGET_DIR = ./
endif

ifdef DISABLE_COMPILE_INFO
	DEF += -D__DISABLE_COMPILE_INFO
endif

ifeq ($(DLL), Y)
	DEF += -DPDC_DLL_BUILD
	ifeq ($(wildcard $(LIB_DIR)/libcurses.dll),)
		LIBS_DEPS += libcurses.dll
	endif
	ifeq ($(wildcard $(LIB_DIR)/libpthread.dll),)
		LIBS_DEPS += libpthread.dll
	endif
else
	ifeq ($(wildcard $(LIB_DIR)/libcurses.a),)
		LIBS_DEPS += libcurses.a
	endif
	ifeq ($(wildcard $(LIB_DIR)/libpthread.a),)
		LIBS_DEPS += libpthread.a
	endif
endif

all : before_make $(LIBS_DEPS) $(TARGET)

libcurses.a libcurses.dll :
	$(MAKE) DLL="$(DLL)" TAR_DIR="$(LIB_REL_DIR)" -C ../lib/win/pdcurses

libpthread.a libpthread.dll :
	$(MAKE) DLL="$(DLL)" TAR_DIR="$(LIB_REL_DIR)" -C ../lib/win/pthreads

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $(TARGET_DIR)/$@ $(LIBS)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(DEF) $(INCS) $(CFLAGS)

.PHONY : before_make clean

before_make :
	-before_make.bat

clean :
	-clean.bat
