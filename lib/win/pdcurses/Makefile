# Corrected by lvsenlv, copyright: https://github.com/Bill-Gray/PDCurses
# GNU MAKE Makefile for PDCurses library - WIN32 MinGW GCC

WIDE = Y
UTF8 = Y

INC_DIR  = ./include
SRC_DIR  = ./src
OBJ_DIR  = ./object
LIB_NAME = curses
ifeq ($(TAR_DIR),)
TAR_DIR  = ./
endif

OBJS = \
	pdcclip.o pdcdisp.o pdcgetsc.o pdckbd.o pdcscrn.o pdcsetsc.o pdcutil.o \
	addch.o addchstr.o addstr.o attr.o beep.o bkgd.o border.o \
	clear.o color.o delch.o deleteln.o deprec.o getch.o getstr.o \
	getyx.o inch.o inchstr.o initscr.o inopts.o insch.o insstr.o \
	instr.o kernel.o keyname.o mouse.o move.o outopts.o overlay.o \
	pad.o panel.o printw.o refresh.o scanw.o scr_dump.o scroll.o \
	slk.o termattr.o terminfo.o touch.o util.o window.o debug.o

PREFIX =
CC     = $(PREFIX)gcc
CFLAGS = -I$(INC_DIR) -pedantic -Wall -O3

ifdef CHTYPE_32
	CFLAGS += -DCHTYPE_32
endif

ifdef CHTYPE_16
	CFLAGS += -DCHTYPE_16
endif

ifeq ($(WIDE),Y)
	CFLAGS += -DPDC_WIDE
endif

ifeq ($(UTF8),Y)
	CFLAGS += -DPDC_FORCE_UTF8
endif

ifeq ($(DLL),Y)
	CFLAGS    += -DPDC_DLL_BUILD
	LIB_EXE   = $(PREFIX)gcc
	#LIB_FLAGS = -Wl,--out-implib,lib$(LIB_NAME).a -O3 -shared -o
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
	
clean :
	-clean.bat
