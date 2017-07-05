#ifndef SOBEL_OPERATOR_H
#define SOBEL_OPERATOR_H

#include "Structs.h"

Image xSobel_c(const Image* image);
Image ySobel_c(const Image* image);

Image xSobel_asm(const Image* image);
Image ySobel_asm(const Image* image);

void applySobelOperator_c(const Image* src, Image* dstLum, Image* dstAngle);
void applySobelOperator_asm(const Image* src, Image* dstLum, Image* dstAngle);
extern void apply_sobel_gradient_calculation_1bpp(short* xResult, short * yResult, Image* dstLum, Image* dstAngle);
unsigned char normalizeAngle(const double angle);
#endif
