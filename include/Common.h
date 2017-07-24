#ifndef COMMON_H
#define COMMON_H

#include "Structs.h"

#define M_PI 3.14159265358979323846
#define M_E  2.71828182845904523536

void performConvolutionStep(const Image* image, const SquareMatrix* mat, const unsigned int x, const unsigned int y, short* dst);
short* convolute_c(const Image* src, const SquareMatrix* mat);
extern short* convolute_asm_1bpp(const Image* src, const SquareMatrix* mat);

void replaceImage(const Image* src, Image* dst);
void replaceData(const short* src, Image* dst);
void emptyImageWithFormat(const unsigned short width, const unsigned short height, const unsigned char bpp, Image* dst);

UI_Matrix getUI_Matrix(const unsigned int width, const unsigned int height);
UI_Matrix getUI_Matrix_v(const unsigned int width, const unsigned int height, const unsigned char defaultValue);

#endif
