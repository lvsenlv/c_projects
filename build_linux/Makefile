TARGET = encrypt

OBJ_DIR = ./object
OBJS = common.o str.o control.o main.o encryption.o core_code.o extra.o decoder.o

CC = gcc
LIBS := -lncursesw -lpthread -L../lib/linux
DEF := -D__LINUX -D__64BIT 
INC := -I../include 
#CFLAGS := -Wall -Wno-unused-function -O3
CFLAGS := -D__DEBUG -g -Wall -Wno-unused-function

$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS)

clean :
	rm -rf $(OBJ_DIR) $(TARGET) *log*
