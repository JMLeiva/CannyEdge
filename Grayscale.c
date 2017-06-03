#include "include/Grayscale.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

extern void applyGrayscale_asm(const Image* src, Image* dst);

void applyGrayscale_c(const Image* src, Image* dst)
{
	unsigned char* newData = (unsigned char*)malloc(src->width * src->height);

	unsigned int grayIndex = 0;
	unsigned short val = 0;

	for (unsigned int i = 0; i < src->width * src->height * src->bpp; i += src->bpp)
	{
		val = 0;

		for (unsigned char b = 0; b < src->bpp || b < 3; b++)
		{
			val += src->data[i + b];
		}

		newData[grayIndex] = val / src->bpp;
		grayIndex++;
	}


	dst->data = newData;
	dst->width = src->width;
	dst->height = src->height;
	dst->bpp = 1;
}

void applyGrayscale(const Image* src, Image* dst)
{
	applyGrayscale_asm(src, dst);
	applyGrayscale_c(src, dst);
}


void applyGrayscaleMax(const Image* src, Image* dst)
{
	unsigned char* newData = (unsigned char*)malloc(src->width * src->height);

	unsigned grayIndex = 0;
	unsigned char max = 0;

	for (unsigned int i = 0; i < src->width * src->height * src->bpp; i += src->bpp)
	{
		max = 0;

		for (unsigned char b = 0; b < src->bpp; b++)
		{
			if (src->data[i + b] > max)
			{
				max = src->data[i + b];
			}

		}

		newData[grayIndex] = max;
		grayIndex++;
	}


	dst->data = newData;
	dst->width = src->width;
	dst->height = src->height;
	dst->bpp = 1;
}
