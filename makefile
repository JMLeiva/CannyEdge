ASM = nasm
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	ASMFLAGS64 = -f elf64 -g -F DWARF
endif
ifeq ($(UNAME_S),Darwin)
	ASMFLAGS64 = -fmacho64 -g -DDARWIN
endif

ASMFLAGS = $(ASMFLAGS64)

CC=gcc
CFLAGS=-std=c99 -lm -ggdb -Wall -Wextra -pedantic -m64 -O3
IDIR = include
LODEDIR = lodepng
OUTPUT = CannyEdge
DEPS = $(IDIR)/*.h
OBJ = CannyEdge.o Common.o GaussBlur.o Grayscale.o Log.o NonMaxSuppression.o SobelOperator.o Threshold.o Utils.o lodepng.o \
	  Common_asm.o  Grayscale_asm.o Threshold_asm.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o: %.asm $(DEPS)
	$(ASM) $(ASMFLAGS) -o $@ $<

lodepng.o: lodepng/lodepng.c lodepng/lodepng.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTPUT): $(OBJ) lodepng.o
	gcc -o $@ $(OBJ) $(CFLAGS) 

all:    $(OUTPUT)

.PHONY: clean

clean:
	rm -f *.o $(OUTPUT) *~ core $(INCDIR)/*~ 
