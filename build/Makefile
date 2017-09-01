CC = gcc
OBJS = common.o control.o encryption.o main.o str.o
OBJS_DIR = ./objects
LIBS = -lncursesw -lpthread
DEF = -D__OS -D__LINUX -D__64BIT 
$(shell mkdir -p $(OBJS_DIR))

test : $(OBJS)
	$(CC) $(wildcard $(OBJS_DIR)/*.o) -o $@ $(LIBS)

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJS_DIR)/$@ $(DEF) -I../include $(LIBS) -g -Wall -Wno-unused-function

clean :
	rm -rf $(OBJS_DIR) test *.*crypt
