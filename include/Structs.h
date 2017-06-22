#ifndef STRUCTS_H
#define STRUCTS_H

#define FALSE 0
#define TRUE 1

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
	unsigned char width;
	unsigned char height;
	unsigned char* data;
} UC_Matrix;

typedef struct
{
	unsigned char width;
	unsigned char height;
	unsigned int* data;
} UI_Matrix;

#endif // !STRUCTS_H
