#include "include/Grayscale.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

void applyGrayscale_c(const Image* src, Image* dst)
{
	unsigned char* newData = (unsigned char*)malloc(src->width * src->height);

	unsigned int grayIndex = 0;
	unsigned short val = 0;

	unsigned short colorChannels = src->bpp;
	if(colorChannels > 3)
	{
		colorChannels = 3;
	}

	for (unsigned int i = 0; i < src->width * src->height * src->bpp; i += src->bpp)
	{
		val = 0;

		for (unsigned char b = 0; b < colorChannels; b++)
		{
			val += src->data[i + b];
		}

		newData[grayIndex] = val / colorChannels;
		grayIndex++;
	}


	dst->data = newData;
	dst->width = src->width;
	dst->height = src->height;
	dst->bpp = 1;
}

void applyGrayscale_asm(const Image* src, Image* dst)
{
	applyGrayscale_asm_impl(src, dst);



	unsigned int totalSize = src->width * src->height * src->bpp;

	unsigned int unalignedStart = totalSize;

	while(totalSize - unalignedStart < 16)
	{
		unalignedStart -= dst->bpp;
	}

	unsigned int grayIndex = unalignedStart / src->bpp;
	unsigned short val = 0;

	for (unsigned int i = unalignedStart; i < totalSize; i += src->bpp)
	{
		val = 0;

		for (unsigned char b = 0; b < src->bpp && b < 3; b++)
		{
			val += src->data[i + b];
		}

		dst->data[grayIndex] = val / src->bpp;
		grayIndex++;
	}



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
