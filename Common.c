#include "include/Common.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

short* convolute_c(const Image* src, const SquareMatrix* mat)
{
	assert(src->bpp == 1);

	int dataSize = src->width * src->height;

	short* newData = (short*)malloc(dataSize * 2);

	unsigned int x, y;

	int srcIndex = 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			if(x == 12 && y == 2)
			{
				x++;
				x--;
			}

			performConvolutionStep(src, mat, x, y, newData + srcIndex);

			srcIndex += src->bpp;
		}
	}

	return newData;
}


void performConvolutionStep(const Image* image, const SquareMatrix* mat, const unsigned int x, const unsigned int y, short* dst)
{
	int srcIndex;
	int matIndex = 0;

	int xOffset, yOffset;


	float fDst = 0;


	for (int my = -mat->size / 2; my <= mat->size / 2; my++)
	{
		for (int mx = -mat->size / 2; mx <= mat->size / 2; mx++)
		{
			if ((int)y + my < 0 || y + my >= image->height)
			{
				yOffset = 0;
			}
			else
			{
				yOffset = my;
			}

			if ((int)x + mx < 0 || x + mx >= image->width)
			{
				xOffset = 0;
			}
			else
			{
				xOffset = mx;
			}



			srcIndex = ((y + yOffset) * image->width + (x + xOffset));

			float matValue = mat->data[matIndex];

			float delta = image->data[srcIndex] * matValue;
			fDst += delta;

			matIndex++;
		}
	}


	dst[0] = (short)fDst;
}

void emptyImageWithFormat(const unsigned short width, const unsigned short height, const unsigned char bpp, Image* dst)
{
	dst->width = width;
	dst->height = height;
	dst->bpp = bpp;
	dst->data = (unsigned char*)malloc(width * height * bpp);
}

void replaceImage(const Image* src, Image* dst)
{
	assert(dst->bpp == src->bpp);
	assert(dst->width == src->width);
	assert(dst->height == src->height);

	free(dst->data);
	dst->data = src->data;
}

void replaceData(const short* src, Image* dst)
{
	for (unsigned int i = 0; i < dst->width * dst->height * dst->bpp; i++)
	{
		dst->data[i] = (char)abs(src[i]);
	}
}


UI_Matrix getUI_Matrix(const unsigned int width, const unsigned int height)
{
	UI_Matrix m;
	m.width = width;
	m.height = height;
	m.data = (unsigned int*)malloc(width * height * sizeof(int));
	return m;
}

UI_Matrix getUI_Matrix_v(const unsigned int width, const unsigned int height, const unsigned char defaultValue)
{
	UI_Matrix m;
	m.width = width;
	m.height = height;
	m.data = (unsigned int*)malloc(width * height * sizeof(int));
	memset(m.data, defaultValue, width * height);
	return m;
}
