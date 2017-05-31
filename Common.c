#include "include/Common.h"
#include <stdlib.h>
#include <assert.h>

void performConvolutionStep(Image* image, SquareMatrix mat, unsigned int x, unsigned int y, short* dst);

short* convolute(Image* src, SquareMatrix mat)
{
	int dataSize = src->width * src->height * src->bpp;

	short* newData = (short*)malloc(dataSize * 2);

	unsigned int x, y;

	int srcIndex = 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			//srcIndex = (y * src->width + x) * src->bpp;

			performConvolutionStep(src, mat, x, y, newData + srcIndex);

			srcIndex += src->bpp;
		}
	}

	return newData;
}

void performConvolutionStep(Image* image, SquareMatrix mat, unsigned int x, unsigned int y, short* dst)
{
	int srcIndex;
	int matIndex = 0;

	int xOffset, yOffset;

	float* fDst = (float*)malloc(image->bpp * sizeof(float));

	for (char i = 0; i < image->bpp; i++)
	{
		fDst[i] = 0;
	}

	for (int my = -mat.size / 2; my <= mat.size / 2; my++)
	{
		for (int mx = -mat.size / 2; mx <= mat.size / 2; mx++)
		{
			if (y + my < 0 || y + my >= image->height)
			{
				yOffset = 0;
			}
			else
			{
				yOffset = my;
			}

			if (x + mx < 0 || x + mx >= image->width)
			{
				xOffset = 0;
			}
			else
			{
				xOffset = mx;
			}

			srcIndex = ((y + yOffset) * image->width + (x + xOffset)) * image->bpp;

			float matValue = mat.data[matIndex];

			for (char i = 0; i < image->bpp; i++)
			{
				float delta = image->data[srcIndex + i] * matValue;

				if (fDst[i] + delta > 255)
				{
					fDst[i] = 255;
				}
				else
				{
					fDst[i] += delta;
				}
			}

			matIndex++;
		}
	}

	for (char i = 0; i < image->bpp; i++)
	{
		dst[i] = (short)fDst[i];
	}

	free(fDst);
}

void emptyImageWithFormat(unsigned short width, unsigned short height, unsigned char bpp, Image* dst)
{
	dst->width = width;
	dst->height = height;
	dst->bpp = bpp;
	dst->data = (unsigned char*)malloc(width * height * bpp);
}

void replaceImage(Image* dst, Image* src)
{
	assert(dst->bpp == src->bpp);
	assert(dst->width == src->width);
	assert(dst->height == src->height);

	free(dst->data);
	dst->data = src->data;
}

void replaceData(Image* dst, short* src)
{
	for (int i = 0; i < dst->width * dst->height * dst->bpp; i++)
	{
		dst->data[i] = (char)abs(src[i]);
	}
}

UC_Matrix getUC_Matrix(unsigned int width, unsigned int height)
{
	UC_Matrix m;
	m.width = width;
	m.height = height;
	m.data = (unsigned char*)malloc(width * height);
	return m;
}

UC_Matrix getUC_Matrix_v(unsigned int width, unsigned int height, unsigned char defaultValue)
{
	UC_Matrix m;
	m.width = width;
	m.height = height;
	m.data = (unsigned char*)malloc(width * height);
	memset(m.data, defaultValue, width * height);
	return m;
}

UI_Matrix getUI_Matrix(unsigned int width, unsigned int height)
{
	UI_Matrix m;
	m.width = width;
	m.height = height;
	m.data = (unsigned int*)malloc(width * height * sizeof(int));
	return m;
}

UI_Matrix getUI_Matrix_v(unsigned int width, unsigned int height, unsigned char defaultValue)
{
	UI_Matrix m;
	m.width = width;
	m.height = height;
	m.data = (unsigned int*)malloc(width * height * sizeof(int));
	memset(m.data, defaultValue, width * height);
	return m;
}
