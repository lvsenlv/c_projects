INC_DIR  = ./include
SRC_DIR  = ./src
OBJ_DIR  = ./object
LIB_NAME = extra
ifeq ($(TAR_DIR),)
TAR_DIR  = ./
endif

OBJS = extra.o

PREFIX =
CC     = $(PREFIX)gcc
CFLAGS = -I$(INC_DIR) -pedantic -Wall -O3

ifeq ($(DLL),Y)
	CFLAGS    += -DPDC_DLL_BUILD
	LIB_EXE   = $(PREFIX)gcc
	LIB_FLAGS = -Wl,--out-implib,lib$(LIB_NAME).a -O3 -shared -o
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

all : $(LIBS) clean

$(LIBS) : $(LIB_DEPS)
	$(LIB_EXE) $(LIB_FLAGS) $(TAR_DIR)/$@ $(wildcard $(OBJ_DIR)/*.o)
	
%.o : $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $(OBJ_DIR)/$@
	
.PHONY: clean
	
clean :
	-clean.bat
