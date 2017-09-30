TARGET = encrypt
CC = gcc
OBJS = common.o control.o encryption.o core_code.o main.o str.o
OBJS_DIR = objects
#LIBS := -l../libs/win/pdcurses.lib -l../libs/win/libpthreadGC2.a
LIBS :=
DEF := -D__WINDOWS -D__64BIT -D__DEBUG
INCLUDES := -I../include -I../include/win
CFLAGS := -g -Wall -Wno-unused-function -O2

all : before_make $(TARGET)

before_make :
	copy ..\libs\win\pdcurses.dll .
	copy ..\libs\win\pthreadGC2.dll .
	
	$(CC) -shared ../src/extra.c -o extra.dll -I../include

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJS_DIR)/*.o) -o $@ pdcurses.dll pthreadGC2.dll extra.dll

%.o : ../src/%.c
	$(CC) -c $< -o $(OBJS_DIR)/$@ $(DEF) $(INCLUDES) $(LIBS) $(CFLAGS)

clean :
	del /f /q pdcurses.dll pthreadGC2.dll extra.dll
	del /q /f $(OBJS_DIR)

