# Corrected by lvsenlv, copyright: https://github.com/Bill-Gray/PDCurses
# GNU MAKE Makefile for pthreads library - WIN32 MinGW GCC

INC_DIR  = ./include
SRC_DIR  = ./src
OBJ_DIR  = ./object
LIB_NAME = pthread
ifeq ($(TAR_DIR),)
TAR_DIR  = ./
endif

OBJS = \
	attr.o \
	barrier.o \
	cancel.o \
	cleanup.o \
	condvar.o \
	create.o \
	dll.o \
	errno.o \
	exit.o \
	fork.o \
	global.o \
	misc.o \
	mutex.o \
	nonportable.o \
	private.o \
	rwlock.o \
	sched.o \
	semaphore.o \
	signal.o \
	spin.o \
	sync.o \
	tsd.o

PREFIX =
CC     = $(PREFIX)gcc
CFLAGS = -D__CLEANUP_C -DHAVE_PTW32_CONFIG_H -I$(INC_DIR) -Wall -O3

ifeq ($(DLL),Y)
	LIB_EXE   = $(PREFIX)gcc
	#LIB_FLAGS = -Wl,--out-implib,lib$(LIB_NAME).a -D__CLEANUP_C -O3 -shared -o
	LIB_FLAGS = -O3 -shared -o
	LIBS      = lib$(LIB_NAME).dll
	LIB_DEPS  = $(OBJS)
else
	LIB_EXE   = $(PREFIX)ar
ifeq ($(PREFIX),)
	LIB_FLAGS = rcs
else
	LIB_FLAGS = rv
endif
	LIBS      = lib$(LIB_NAME).a
	LIB_DEPS  = $(OBJS)
endif

all : before_make $(LIBS) clean

$(LIBS) : $(LIB_DEPS)
	$(LIB_EXE) $(LIB_FLAGS) $(TAR_DIR)/$@ $(wildcard $(OBJ_DIR)/*.o)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $(OBJ_DIR)/$@

.PHONY : before_make clean

before_make :
	-before_make.bat

clean:
	-clean.bat
