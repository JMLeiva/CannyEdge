#ifndef SOBEL_OPERATOR_H
#define SOBEL_OPERATOR_H

#include "Structs.h"

Image xSobel_c(const Image* image);
Image ySobel_c(const Image* image);

Image xSobel_asm(const Image* image);
Image ySobel_asm(const Image* image);

void applySobelOperator_c(const Image* src, Image* dstLum, Image* dstAngle);
void applySobelOperator_asm(const Image* src, Image* dstLum, Image* dstAngle);

#endif
