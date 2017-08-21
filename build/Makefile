CC = gcc
OBJS = common.o control_str.o control.o encryption.o main.o
OBJS_DIR = ./objects
LIBS = -lncursesw
DEF = -D__OS -D__LINUX -D__64BIT 
$(shell mkdir -p $(OBJS_DIR))

test : $(OBJS)
	$(CC) $(wildcard $(OBJS_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJS_DIR)/$@ $(DEF) -I../include $(LIBS) -g -Wall -Wno-unused-function

clean :
	rm -rf $(OBJS_DIR) test *.*crypt
