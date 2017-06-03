#ifndef SOBEL_OPERATOR_H
#define SOBEL_OPERATOR_H

#include "Structs.h"

Image xSobel(const Image* image);
Image ySobel(const Image* image);

void applySobelOperator(const Image* src, Image* dstLum, Image* dstAngle);

#endif
