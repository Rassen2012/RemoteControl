USER_DEFINES=
RM=rm
CC=gcc
LD=ld
CPU=linux
TARGET=obj/lzo.o
OBJ_DIR=obj/

USER_INCLUDES=../

SOURCES=        minilzo.c
        

USER_DEFINES=

VPATH=$(SOURCE_DIR)

all: create_err $(TARGET)

.PHONY: create_err 
create_err: 
	@echo "" >err

$(TARGET): $(addprefix $(OBJ_DIR),$(notdir $(patsubst %.c,%.o, $(SOURCES)))) 
	$(LD) $^ -r -o $@

$(OBJ_DIR)%.o: %.c
	$(CC) -Wall -Wimplicit -O3 -D"dbg=left_equ=flg_printf&&$(COMP)_print" $(foreach var,$(USER_DEFINES),-D$(var)) $(foreach var,$(USER_INCLUDES),-I$(var)) -o $@ -c -MMD $< >>err 2>&1


clean:
	$(RM) -f *.o *.d $(OBJ_DIR)*.o $(OBJ_DIR)*.d *.s $(OBJ_DIR)*.s err

build: clean all
include $(wildcard $(OBJ_DIR)*.d)

