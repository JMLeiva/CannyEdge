#ifndef SOBEL_OPERATOR_H
#define SOBEL_OPERATOR_H

#include "Structs.h"

Image xSobel(Image* image);
Image ySobel(Image* image);

void applySobelOperator(Image* src, Image* dstLum, Image* dstAngle);

#endif
