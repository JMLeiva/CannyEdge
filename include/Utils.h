#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

uint64_t rdtsc();

int parseInt(char* str, char* error);
float parseFloat(char* str, char* error);

#endif // !UTILS_H
