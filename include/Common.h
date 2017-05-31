#ifndef COMMON_H
#define COMMON_H

#include "Structs.h"

#define M_PI 3.14159265358979323846
#define M_E  2.71828182845904523536

short* convolute(Image* src, SquareMatrix mat);
void replaceImage(Image* dst, Image* src);
void replaceData(Image* dst, short* src);
void emptyImageWithFormat(unsigned short width, unsigned short height, unsigned char bpp, Image* dst);

UI_Matrix getUI_Matrix(unsigned int width, unsigned int height);
UI_Matrix getUI_Matrix_v(unsigned int width, unsigned int height, unsigned char defaultValue);

UC_Matrix getUC_Matrix(unsigned int width, unsigned int height);
UC_Matrix getUC_Matrix_v(unsigned int width, unsigned int height, unsigned char defaultValue);

#endif
