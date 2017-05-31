#include "include/Utils.h"

//  Windows
#ifdef _WIN32

#include <intrin.h>
uint64_t rdtsc() {
	return __rdtsc();
}

//  Linux/GCC
#else

uint64_t rdtsc() {
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)hi << 32) | lo;
}

#endif

int parseInt(char* str, char* error)
{
	*error = 0;
	int result = 0;

	while (*str != 0)
	{
		if (*str < 48 || *str >= 58)
		{
			*error = 1;
			return 0;
		}

		result += *str - 48;
		
		if (str[1] != 0)
		{
			result *= 10;
		}

		str++;
	}

	return result;
}

float parseFloat(char* str, char* error)
{
	return 0;
}
