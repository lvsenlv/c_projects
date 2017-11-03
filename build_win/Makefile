TARGET      = encrypt
OBJ_DIR     = ./object
SRC_DIR     = ../src
ifeq ($(DLL), Y)
LIB_DIR     = .
#relative directory of libraries, used in building libraries
LIB_REL_DIR = ../../build_win 
else
LIB_DIR     = ../lib/win
LIB_REL_DIR = ../win
endif

OBJS = common.o control.o encryption.o core_code.o main.o str.o extra.o decoder.o

CC        = gcc
DEF       = -D__WINDOWS \
			-D__64BIT \
			-D__DEBUG \
			-DPDC_WIDE \
			-DPDC_FORCE_UTF8 \
			-D__CLEANUP_C \
			-DHAVE_PTW32_CONFIG_H
INCS      = -I../include \
			-I../lib/pthreads/include \
			-I../lib/pdcurses/include
CFLAGS    = -Wall -O3
LIBS      = -L$(LIB_DIR) -lpthread -lcurses
LIBS_DEPS =

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

all : before $(TARGET)

before : $(LIBS_DEPS)

libcurses.a libcurses.dll :
	$(MAKE) DLL="$(DLL)" TAR_DIR="$(LIB_REL_DIR)" -C ../lib/pdcurses
	
libpthread.a libpthread.dll :
	$(MAKE) DLL="$(DLL)" TAR_DIR="$(LIB_REL_DIR)" -C ../lib/pthreads
	
$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(DEF) $(INCS) $(CFLAGS)
	
.PHONY : clean
	
clean :
	-clean.bat
