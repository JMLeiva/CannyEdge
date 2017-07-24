#include "include/SobelOperator.h"
#include "include/Common.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#define M_PI_8 0.392699
#define M_3PI_8 1.17809
#define M_5PI_8 1.96349
#define M_7PI_8 2.74889

SquareMatrix getXMat();
SquareMatrix getYMat();

SquareMatrix getXMatAligned();
SquareMatrix getYMatAligned();


void applySobelOperator_c(const Image* src, Image* dstLum, Image* dstAngle)
{
	assert(src->bpp == 1);

	SquareMatrix xMat = getXMat();
	SquareMatrix yMat = getYMat();

	short* xResult = convolute_c(src, &xMat);
	short* yResult = convolute_c(src, &yMat);

	emptyImageWithFormat(src->width, src->height, src->bpp, dstLum);
	emptyImageWithFormat(src->width, src->height, src->bpp, dstAngle); 

	for (unsigned int i = 0; i < src->width * src->height * src->bpp; i++)
	{

		float v = sqrt(xResult[i] * xResult[i] + yResult[i] * yResult[i]); //abs(xResult[i]) + abs(yResult[i]);//

		if(v > 255)
		{
			dstLum->data[i] = 255;
		}
		else
		{
			dstLum->data[i] = (unsigned char)v;
		}

		short angle = 0;

		angle = normalizeAngle(atan2f(xResult[i], yResult[i]));

		dstAngle->data[i] = angle;
	}

	free(xResult);
	free(yResult);
	free(xMat.data);
	free(yMat.data);
}

void applySobelOperator_asm(const Image* src, Image* dstLum, Image* dstAngle)
{
	assert(src->bpp == 1);

	SquareMatrix xMat = getXMatAligned();
	SquareMatrix yMat = getYMatAligned();

	short* xResult = convolute_asm_1bpp(src, &xMat);
	short* yResult = convolute_asm_1bpp(src, &yMat);


	//////////////////////////////////////////////////////////////////////////////
	// Borders are calculated in C
	//////////////////////////////////////////////////////////////////////////////
	SquareMatrix xMatBorder = getXMat();
	SquareMatrix yMatBorder = getYMat();

	unsigned int x, y;

	int srcIndex = 0;

	// VERTICAL BORDERS
	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < xMatBorder.size / 2; x++)
		{
			srcIndex = (y * src->width + x);

			performConvolutionStep(src, &xMatBorder, x, y, xResult + srcIndex);
			performConvolutionStep(src, &yMatBorder, x, y, yResult + srcIndex);

			srcIndex++;
		}

		for (x = src->width - xMatBorder.size / 2; x < src->width; x++)
		{
			srcIndex = (y * src->width + x);

			performConvolutionStep(src, &xMatBorder, x, y, xResult + srcIndex);
			performConvolutionStep(src, &yMatBorder, x, y, yResult + srcIndex);

			srcIndex++;
		}
	}

	// HORIZONTAL BORDERS
	for (x = 0; x < src->width; x++)
	{
		for (y = 0; y < xMatBorder.size / 2; y++)
		{
			srcIndex = (y * src->width + x);

			performConvolutionStep(src, &xMatBorder, x, y, xResult + srcIndex);
			performConvolutionStep(src, &yMatBorder, x, y, yResult + srcIndex);

			srcIndex++;
		}

		for (y = src->height - xMatBorder.size / 2; y < src->height; y++)
		{
			srcIndex = (y * src->width + x);

			performConvolutionStep(src, &xMatBorder, x, y, xResult + srcIndex);
			performConvolutionStep(src, &yMatBorder, x, y, yResult + srcIndex);

			srcIndex++;
		}
	}

	emptyImageWithFormat(src->width, src->height, src->bpp, dstLum);
	emptyImageWithFormat(src->width, src->height, src->bpp, dstAngle);

	//apply_sobel_gradient_calculation_1bpp(xResult, yResult, dstLum, dstAngle);

	for (unsigned int i = 0; i < src->width * src->height * src->bpp; i++)
	{

		float v = sqrt(xResult[i] * xResult[i] + yResult[i] * yResult[i]); //abs(xResult[i]) + abs(yResult[i]);//
		if(v > 255)
		{
			dstLum->data[i] = 255;
		}
		else
		{
			dstLum->data[i] = (unsigned char)v;
		}

		short angle = 0;

		angle = normalizeAngle(atan2f(xResult[i], yResult[i]));

		dstAngle->data[i] = angle;
	}

	free(xResult);
	free(yResult);
	free(xMat.data);
	free(yMat.data);
	free(xMatBorder.data);
	free(yMatBorder.data);
}

Image xSobel_c(const Image* image)
{
	SquareMatrix xMat = getXMat();
	Image result;
	emptyImageWithFormat(image->width, image->height, image->bpp, &result);
	replaceData(convolute_c(image, &xMat), &result);
	return result;
}

Image ySobel_c(const Image* image)
{
	SquareMatrix yMat = getYMat();

	Image result;
	emptyImageWithFormat(image->width, image->height, image->bpp, &result);
	replaceData(convolute_c(image, &yMat), &result);
	return result;
}

Image xSobel_asm(const Image* image)
{
	SquareMatrix xMat = getXMatAligned();
	Image result;
	emptyImageWithFormat(image->width, image->height, image->bpp, &result);
	replaceData(convolute_asm_1bpp(image, &xMat), &result);
	return result;
}

Image ySobel_asm(const Image* image)
{
	SquareMatrix yMat = getYMatAligned();

	Image result;
	emptyImageWithFormat(image->width, image->height, image->bpp, &result);
	replaceData(convolute_asm_1bpp(image, &yMat), &result);
	return result;
}


SquareMatrix getXMat()
{
	SquareMatrix mat;

	mat.size = 3;

	mat.data = (float*)malloc(9 * sizeof(float));

	mat.data[0] = -1;
	mat.data[1] = 0;
	mat.data[2] = 1;

	mat.data[3] = -2;
	mat.data[4] = 0;
	mat.data[5] = 2;

	mat.data[6] = -1;
	mat.data[7] = 0;
	mat.data[8] = 1;

	return mat;
}

SquareMatrix getYMat()
{
	SquareMatrix mat;

	mat.size = 3;

	mat.data = (float*)malloc(9 * sizeof(float));

	mat.data[0] = -1;
	mat.data[1] = -2;
	mat.data[2] = -1;

	mat.data[3] = 0;
	mat.data[4] = 0;
	mat.data[5] = 0;

	mat.data[6] = 1;
	mat.data[7] = 2;
	mat.data[8] = 1;

	return mat;
}

SquareMatrix getXMatAligned()
{
	SquareMatrix mat;

	mat.size = 3;

	mat.data = (float*)malloc(16 * sizeof(float));

	mat.data[0] = -1;
	mat.data[1] = 0;
	mat.data[2] = 1;

	mat.data[4] = -2;
	mat.data[5] = 0;
	mat.data[6] = 2;

	mat.data[8] = -1;
	mat.data[9] = 0;
	mat.data[10] = 1;

	return mat;
}

SquareMatrix getYMatAligned()
{
	SquareMatrix mat;

	mat.size = 3;

	mat.data = (float*)malloc(16 * sizeof(float));

	mat.data[0] = -1;
	mat.data[1] = -2;
	mat.data[2] = -1;

	mat.data[4] = 0;
	mat.data[5] = 0;
	mat.data[6] = 0;

	mat.data[8] = 1;
	mat.data[9] = 2;
	mat.data[10] = 1;

	return mat;
}

unsigned char normalizeAngle(const double angle)
{
	if (angle > 0)
	{
		if (angle <= M_PI_8)
		{
			return 0;
		}
		else if (angle <= M_3PI_8)
		{
			return 45;
		}
		else if (angle <= M_5PI_8)
		{
			return 90;
		}
		else if (angle <= M_7PI_8)
		{
			return 135;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (angle >= -M_PI_8)
		{
			return 0;
		}
		else if (angle >= -M_3PI_8)
		{
			return 135;
		}
		else if (angle >= M_5PI_8)
		{
			return 90;
		}
		else if (angle >= M_7PI_8)
		{
			return 45;
		}
		else
		{
			return 0;
		}
	}
}
