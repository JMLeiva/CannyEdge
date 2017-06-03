#ifndef GAUSS_BLUR_H
#define GAUSS_BLUR_H

#include "Structs.h"

void applyGaussBlur(const Image* src, const unsigned char mSize, const float sigma, Image* dst);
#endif
