#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include "Structs.h"

extern void applyGrayscale_asm(const Image* src, Image* dst, bool ymmEnabled);
void applyGrayscale_c(const Image* src, Image* dst);
void applyGrayscaleMax(const Image* src, Image* dst);

#endif
