#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct
{
	unsigned char* data;
	unsigned int width;
	unsigned int height;
	unsigned char bpp;
} Image;

typedef struct 
{
	unsigned char size;
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
