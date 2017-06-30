#ifndef STRUCTS_H
#define STRUCTS_H

#define FALSE 0
#define TRUE 1

#include <stdint.h>

typedef char bool;

typedef struct
{
	unsigned int width;
	unsigned int height;
	unsigned char bpp;
	__attribute__((__aligned__(16)))
	unsigned char* data;
} Image;

typedef struct 
{
	unsigned char size;
	__attribute__((__aligned__(16)))
	float* data;
} SquareMatrix;

typedef struct
{
	unsigned short width;
	unsigned short height;
	unsigned char* data;
} UC_Matrix;

typedef struct
{
	unsigned short width;
	unsigned short height;
	unsigned int* data;
} UI_Matrix;

typedef struct
{
	uint64_t grayscale_t, gauss_t, sobel_t, nonMax_t, lowHigh_t, hysteresis_t;
	bool valid;
} Benchmark;

#endif // !STRUCTS_H
