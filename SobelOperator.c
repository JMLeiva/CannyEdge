#include "include/SobelOperator.h"
#include "include/Common.h"
#include <math.h>
#include <stdlib.h>

#define M_PI_8 0.392699
#define M_3PI_8 1.17809
#define M_5PI_8 1.96349
#define M_7PI_8 2.74889

SquareMatrix getXMat();
SquareMatrix getYMat();
unsigned char normalizeAngle(const float angle);

void applySobelOperator(const Image* src, Image* dstLum, Image* dstAngle)
{
	SquareMatrix xMat = getXMat();
	SquareMatrix yMat = getYMat();

	short* xResult = convolute(src, xMat);
	short* yResult = convolute(src, yMat);

	//log(xResult, image->width, image->height, image->bpp);
	//log(yResult, image->width, image->height, image->bpp);
	emptyImageWithFormat(src->width, src->height, src->bpp, dstLum);
	emptyImageWithFormat(src->width, src->height, src->bpp, dstAngle); 

	for (unsigned int i = 0; i < src->width * src->height * src->bpp; i++)
	{
		for (int c = 0; c < src->bpp; c++)
		{
			float v = abs(xResult[i]) + abs(yResult[i]);//sqrt(xResult.data[i] * xResult.data[i] + yResult.data[i] * yResult.data[i]);
			dstLum->data[i] = (unsigned char)v;
			short angle = 0;

			angle = normalizeAngle(atan2f(xResult[i], yResult[i]));
					
			dstAngle->data[i] = angle;
		}
	}

	free(xResult);
	free(yResult);
	free(xMat.data);
	free(yMat.data);
}

Image xSobel(const Image* image)
{
	SquareMatrix xMat = getXMat();
	Image result;
	emptyImageWithFormat(image->width, image->height, image->bpp, &result);
	replaceData(convolute(image, xMat), &result);
	return result;
}

Image ySobel(const Image* image)
{
	SquareMatrix yMat = getYMat();

	Image result;
	emptyImageWithFormat(image->width, image->height, image->bpp, &result);
	replaceData(convolute(image, yMat), &result);
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

unsigned char normalizeAngle(const float angle)
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
