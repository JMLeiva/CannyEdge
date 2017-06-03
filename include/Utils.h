#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "Structs.h"

uint64_t rdtsc();

int parseInt(const char* str, bool* error);
float parseFloat(const char* str, bool* error);

#endif // !UTILS_H
