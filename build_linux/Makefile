TARGET = encrypt

OBJ_DIR = ./objects
OBJS = common.o str.o control.o main.o encryption.o core_code.o

CC = gcc
LIBS := -lncursesw -lpthread -L../lib/linux -lextra
DEF := -D__LINUX -D__64BIT 
INC := -I../include -I../lib/extra/include
#CFLAGS := -Wall -Wno-unused-function -O3
CFLAGS := -D__DEBUG -g -Wall -Wno-unused-function

$(shell mkdir -p $(OBJ_DIR))

all : libextra.so $(TARGET)

libextra.so :
	$(CC) -fPIC -shared ../lib/extra/src/extra.c -o ../lib/linux/libextra.so $(INC) $(CFLAGS)
	cp ../lib/linux/libextra.so /usr/lib

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS)

clean :
	rm -rf $(OBJ_DIR) $(TARGET) *log* /usr/lib/libextra.so
