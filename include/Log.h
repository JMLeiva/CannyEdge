#ifndef LOG_H
#define LOG_H

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_VERVOSE 2
#define LOG_LEVEL LOG_LEVEL_INFO

#include "Structs.h"

void log_info(const char* format, ...);
void log_verbose(const char* format, ...);

void log_sm(const SquareMatrix* mat);
void log_um(const UC_Matrix* mat);
void log_um_cap(const UC_Matrix* mat, const unsigned int capW, const unsigned int capH);
void log_im(const UI_Matrix* mat);
void log_im_cap(const UI_Matrix* mat, const unsigned int capW, const unsigned int capH);
void log_i(const Image* image);
void log_d(const short* data, const unsigned int width, const unsigned int height, const unsigned char bpp);

#endif
