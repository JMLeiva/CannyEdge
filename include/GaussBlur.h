#ifndef GAUSS_BLUR_H
#define GAUSS_BLUR_H

#include "Structs.h"

void applyGaussBlur_c(const Image* src, const unsigned char mSize, const float sigma, Image* dst);
void applyGaussBlur_asm(const Image* src, const unsigned char mSize, const float sigma, Image* dst);
#endif
