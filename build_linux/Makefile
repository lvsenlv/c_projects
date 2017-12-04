TARGET_BASE_NAME 	= encrypt
CC 					= gcc
LIBS 				= -Wl,-Bstatic -L../lib/linux -lncursesw -static -lpthread 
DEF 				= -D__LINUX -D__64BIT 
INC 				= -I../include -I../include/linux
CFLAGS 				= -g -Wall -Wno-unused-function
SRC_DIR     		= ../src
OBJ_DIR 			= ./object
OBJS 				= common.o str.o control.o main.o encryption.o core_code.o extra.o decoder.o
PLATFORM_NAME 		= _linux_$(shell arch)

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
TARGET  = $(TARGET_BASE_NAME)$(PLATFORM_NAME)$(PTHREAD_NUM_MAX_NAME)$(FREE_MEMORY_NAME)

ifndef TARGET_DIR
	TARGET_DIR = ./output
endif

ifdef DISABLE_COMPILE_INFO
	DEF += -D__DISABLE_COMPILE_INFO
endif

$(shell mkdir -p $(TARGET_DIR))
$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $(TARGET_DIR)/$@ $(LIBS)

%.o : $(SRC_DIR)//%.c
	$(CC) -c $<  -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS) 

clean :
	rm -rf $(TARGET_DIR) $(OBJ_DIR) $(TARGET_BASE_NAME)* *log*
