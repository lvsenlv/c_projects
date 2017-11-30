TARGET_BASE_NAME 	= encrypt
TARGET_NAME 		= $(TARGET_BASE_NAME)
CC 					= gcc
LIBS 				= -Wl,-Bstatic -L../lib/linux -lncursesw -static -lpthread 
DEF 				= -D__LINUX -D__64BIT -D__DEBUG 
INC 				= -I../include -I../include/linux
CFLAGS 				= -g -Wall -Wno-unused-function

ifneq ($(PLATFORM),)
	ifneq ($(PTHREAD_NUM_MAX),)
		DEF += -DPTHREAD_NUM_MAX=$(PTHREAD_NUM_MAX)
		ifneq ($(FREE_MEMORY),)
			TARGET_NAME = $(TARGET_BASE_NAME)_$(PLATFORM)_$(PTHREAD_NUM_MAX)th_$(FREE_MEMORY)Mb
			DEF += -DFREE_MEMORY=$(FREE_MEMORY)
		else
			TARGET_NAME = $(TARGET_BASE_NAME)_$(PLATFORM)_$(PTHREAD_NUM_MAX)th_100Mb
		endif
	else
		TARGET_NAME = $(TARGET_BASE_NAME)_$(PLATFORM)_2th_100Mb
	endif
else
	TARGET_NAME = $(TARGET_BASE_NAME)_linux_2th_100Mb
endif
TARGET  = $(TARGET_NAME)

OBJ_DIR = ./object
OBJS = common.o str.o control.o main.o encryption.o core_code.o extra.o decoder.o

$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $<  -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS) 

clean :
	rm -rf $(OBJ_DIR) $(TARGET_BASE_NAME)* *log*
