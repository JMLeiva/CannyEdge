#include "include/GaussBlur.h"
#include "include/Common.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

SquareMatrix getMatrix(const unsigned char mSize, const float sigma);
SquareMatrix getMatrixAligned(const unsigned char mSize, const float sigma);
float getGaussValue(const int x, const int y, const float sigma);

void applyGaussBlur_c(const Image* src, const unsigned char mSize, const float sigma, Image* dst)
{
	assert(mSize % 2 == 1);

	SquareMatrix mat = getMatrix(mSize, sigma);

	dst->width = src->width;
	dst->height = src->height;
	dst->bpp = src->bpp;
	dst->data = (unsigned char*)malloc(src->width * src->height * src->bpp);

	short* convolutionData = convolute_c(src, &mat);

	replaceData(convolutionData, dst);
	free(convolutionData);
	free(mat.data);
}

void applyGaussBlur_asm(const Image* src, const unsigned char mSize, const float sigma, Image* dst)
{
	assert(mSize % 2 == 1);

	SquareMatrix mat = getMatrixAligned(mSize, sigma);

	dst->width = src->width;
	dst->height = src->height;
	dst->bpp = src->bpp;
	dst->data = (unsigned char*)malloc(src->width * src->height * src->bpp);

	assert(src->bpp == 1);

	short* convolutionData = convolute_asm_1bpp(src, &mat);


	//////////////////////////////////////////////////////////////////////////////
	// Borders are calculated in C
	//////////////////////////////////////////////////////////////////////////////
	// VERTICAL BORDERS
	SquareMatrix matBorder = getMatrix(mSize, sigma);

	unsigned int x, y;

	int srcIndex = 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < matBorder.size / 2; x++)
		{
			srcIndex = (y * src->width + x) * src->bpp;
			performConvolutionStep(src, &matBorder, x, y, convolutionData + srcIndex);

			srcIndex += src->bpp;
		}

		for (x = src->width - mat.size / 2; x < src->width; x++)
		{
			srcIndex = (y * src->width + x) * src->bpp;
			performConvolutionStep(src, &matBorder, x, y, convolutionData + srcIndex);

			srcIndex += src->bpp;
		}
	}

	// HORIZONTAL BORDERS
	for (x = 0; x < src->width; x++)
	{
		for (y = 0; y < matBorder.size / 2; y++)
		{
			srcIndex = (y * src->width + x) * src->bpp;

			performConvolutionStep(src, &matBorder, x, y, convolutionData + srcIndex);

			srcIndex += src->bpp;
		}

		for (y = src->height - matBorder.size / 2; y < src->height; y++)
		{
			srcIndex = (y * src->width + x) * src->bpp;

			performConvolutionStep(src, &matBorder, x, y, convolutionData + srcIndex);

			srcIndex += src->bpp;
		}
	}


	replaceData(convolutionData, dst);
	free(convolutionData);
	free(mat.data);
	free(matBorder.data);
}

SquareMatrix getMatrix(const unsigned char mSize, const float sigma)
{
	SquareMatrix mat;
	mat.size = mSize;
	mat.data = (float*)malloc(mSize*mSize * sizeof(float));

	int i = 0;

	for (int y = -mSize / 2; y <= mSize / 2; y++)
	{
		for (int x = -mSize / 2; x <= mSize / 2; x++)
		{
			mat.data[i] = getGaussValue(x, y, sigma);
			i++;
		}
	}

	return mat;
}

SquareMatrix getMatrixAligned(const unsigned char mSize, const float sigma)
{
	unsigned int virtualSize = ( (mSize / 4) + 1) * 4;

	SquareMatrix mat;
	mat.size = mSize;
	mat.data = (float*)malloc(virtualSize*mSize * sizeof(float));

	unsigned int i = 0;

	for (int y = -mSize / 2; y <= mSize / 2; y++)
	{
		for (int x = -mSize / 2; x <= mSize / 2; x++)
		{
			mat.data[i] = getGaussValue(x, y, sigma);
			i++;
		}

		i += virtualSize - (i % virtualSize);
	}

	return mat;
}

float getGaussValue(const int x, const int y, const float sigma)
{
	float squaredSigma = sigma * sigma;

	float v1 = 1 / (2*M_PI * squaredSigma);
	double exp = (x*x + y*y) / (2*squaredSigma);
	float v2 = pow(M_E, -exp);

	float result = v1 * v2;

	return result;
}
