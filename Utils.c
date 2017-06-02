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

int parseInt(char* str, bool* error)
{
	*error = FALSE;
	int result = 0;

	while (*str != 0)
	{
		if (*str < 48 || *str >= 58)
		{
			*error = TRUE;
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

float parseFloat(char* str, bool* error)
{
	*error = FALSE;
	float resultInt = 0;
	float resultDec = 0;
	float decLevel = 10;

	bool decMode = FALSE;

	while (*str != 0)
	{
		if(*str == '.' && !decMode)
		{
			decMode = TRUE;
			str++;
			continue;
		}

		if (*str < 48 || *str >= 58)
		{
			*error = TRUE;
			return 0;
		}

		if(!decMode)
		{
			resultInt += *str - 48;

			if (str[1] != 0 && str[1] != '.') // IF There are more numbers
			{
				resultInt *= 10;
			}
		}
		else
		{
			resultDec += *str - 48;
			resultDec /= decLevel;
			decLevel *= 10;
		}

		str++;
	}

	return resultInt + resultDec;
}
