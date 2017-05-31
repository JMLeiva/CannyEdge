#ifndef GAUSS_BLUR_H
#define GAUSS_BLUR_H

#include "Structs.h"

void applyGaussBlur(Image* src, unsigned char mSize, float sigma, Image* dst);
#endif
