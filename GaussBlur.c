#include "include/GaussBlur.h"
#include "include/Common.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

SquareMatrix getMatrix(const unsigned char mSize, const float sigma);
float getGaussValue(const int x, const int y, const float sigma);

void applyGaussBlur(const Image* src, const unsigned char mSize, const float sigma, Image* dst)
{
	assert(mSize % 2 == 1);

	SquareMatrix mat = getMatrix(mSize, sigma);

	dst->width = src->width;
	dst->height = src->height;
	dst->bpp = src->bpp;
	dst->data = (unsigned char*)malloc(src->width * src->height * src->bpp);

	short* convolutionData = convolute(src, mat);

	replaceData(convolutionData, dst);
	free(convolutionData);
	free(mat.data);
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

float getGaussValue(const int x, const int y, const float sigma)
{
	float squaredSigma = sigma * sigma;

	float v1 = 1 / (2*M_PI * squaredSigma);
	double exp = (x*x + y*y) / (2*squaredSigma);
	float v2 = pow(M_E, -exp);

	float result = v1 * v2;

	return result;
}
