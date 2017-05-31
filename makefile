CC=gcc
CFLAGS=-std=c99 -lm
IDIR = include
LODEDIR = lodepng

DEPS = $(IDIR)/*.h

OBJ = CannyEdge.o Common.o GaussBlur.o Grayscale.o Log.o NonMaxSuppression.o SobelOperator.o Threshold.o Utils.o lodepng.o

$%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

CannyEdge: $(OBJ)
	gcc -o $@ $(OBJ) $(CFLAGS) 

lodepng.o: lodepng/lodepng.c lodepng/lodepng.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ core $(INCDIR)/*~ 
