CC = gcc
OBJS = common.o control.o encryption.o main.o str.o
OBJS_DIR = ./objects
LIBS := -lcurses -lpthread
DEF := -D__LINUX -D__64BIT 
INCLUDES := -I../include
CFLAGS := -g -Wall -Wno-unused-function
$(shell mkdir -p $(OBJS_DIR))

test : $(OBJS)
	$(CC) $(wildcard $(OBJS_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJS_DIR)/$@ $(DEF) $(INCLUDES) $(LIBS) $(CFLAGS)

clean :
	rm -rf $(OBJS_DIR) test *.*crypt log* file_list
