TARGET = encrypt
CC = gcc
OBJS = common.o control.o encryption.o core_code.o main.o str.o
OBJS_DIR = ./objects
LIBS := -lcurses -lpthread -L../libs/linux -lextra
DEF := -D__LINUX -D__64BIT -D__DEBUG
INCLUDES := -I../include
CFLAGS := -g -Wall -Wno-unused-function -O2
$(shell mkdir -p $(OBJS_DIR))

all : libextra.so $(TARGET)

libextra.so :
	$(CC) -fPIC -shared ../src/extra.c -o ../libs/linux/libextra.so $(INCLUDES) $(CFLAGS)
	cp ../libs/linux/libextra.so /usr/lib

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJS_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJS_DIR)/$@ $(DEF) $(INCLUDES) $(LIBS) $(CFLAGS)

clean :
	rm -rf $(OBJS_DIR) $(TARGET) *log* /usr/lib/libextra.so
