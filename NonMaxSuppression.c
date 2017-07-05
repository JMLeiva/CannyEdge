#include "include/NonMaxSuppression.h"
#include "include/Common.h"

char isMax(const Image* image, const unsigned int x, const unsigned int y, const unsigned char angle, const unsigned char bOffset);

void applyNonMaxSuppression(const Image* lum, const Image* angle, Image* dst)
{
	emptyImageWithFormat(lum->width, lum->height, lum->bpp, dst);

	for (unsigned short y = 0; y < lum->height; y++)
	{
		for (unsigned short x = 0; x < lum->width; x++)
		{
			unsigned int index = (y * lum->width + x) * lum->bpp;
			
			for (unsigned char b = 0; b < lum->bpp; b++)
			{
				if (isMax(lum, x, y, angle->data[index + b], b))
				{
					dst->data[index + b] = lum->data[index + b];
				}
				else
				{
					dst->data[index + b] = 0;
				}
			}
		}
	}
}

char isMax(const Image* image, const unsigned int x, const unsigned int y, const unsigned char angle, const unsigned char bOffset)
{

	unsigned int xA, yA, xC, yC;

	int iB = ((y * image->width + x) * image->bpp) + bOffset;


	switch (angle)
	{
	case 0:
	case 45:	
		if (y == 0)
		{
			yA = y;
		}
		else
		{
			yA = y-1;
		}

		if (y >= image->height-1)
		{
			yC = y;
		}
		else
		{
			yC = y + 1;
		}

		break;
	case 90:
		yA = y;
		yC = y;
		break;
	case 135:
		if (y == 0)
		{
			yC = y;
		}
		else
		{
			yC = y - 1;
		}

		if (y >= image->height-1)
		{
			yA = y;
		}
		else
		{
			yA = y + 1;
		}
		break;
	}

	switch (angle)
	{
	case 0:
		xA = x;
		xC = x;
		break;
	case 45:
	case 90:
	case 135:
		if (x == 0)
		{
			xA = x;
		}
		else
		{
			xA = x - 1;
		}

		if (x >= image->width-1)
		{
			xC = x;
		}
		else
		{
			xC = x + 1;
		}
		break;
	}

	int iA = ((yA * image->width + xA) * image->bpp) + bOffset;
	int iC = ((yC * image->width + xC) * image->bpp) + bOffset;

	unsigned char pA = image->data[iA];
	unsigned char pB = image->data[iB];
	unsigned char pC = image->data[iC];


	return pB >= pA && pB >= pC;
}
