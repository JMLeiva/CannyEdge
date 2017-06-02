#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "Structs.h"

uint64_t rdtsc();

int parseInt(char* str, bool* error);
float parseFloat(char* str, bool* error);

#endif // !UTILS_H
