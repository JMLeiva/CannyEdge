#include "include/Log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void log_info(const char* fmt, ...)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	va_list argp;
	va_start(argp, fmt);

	vprintf(fmt, argp);

	va_end(argp);
#endif
}

void log_progress(float val)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO

	char* progressDecor;

	switch(((int)(val * 100) % 10))
	{
	case 0:
		progressDecor = "Working...";
		break;
	case 1:
		progressDecor = ".Working..";
		break;
	case 2:
		progressDecor = "..Working.";
		break;
	case 3:
		progressDecor = "...Working";
		break;
	case 4:
		progressDecor = "g...Workin";
		break;
	case 5:
		progressDecor = "ng...Worki";
		break;
	case 6:
		progressDecor = "ing...Work";
		break;
	case 7:
		progressDecor = "king...Wor";
		break;
	case 8:
		progressDecor = "rking...Wo";
		break;
	case 9:
		progressDecor = "orking...W";
		break;
	}

	printf(" %s%s [ %.02f % ]                   \r", KGRN, progressDecor, val * 100);
	printf("%s", KNRM);
	fflush(stdout);
#endif
}

void log_verbose(const char* fmt, ...)
{
#if LOG_LEVEL >= LOG_LEVEL_VERVOSE
	va_list argp;
	va_start(argp, fmt);

	vprintf(fmt, argp);

	va_end(argp);
#endif
}

void log_verbose_flush(const char* fmt, ...)
{
#if LOG_LEVEL >= LOG_LEVEL_VERVOSE
	va_list argp;
	va_start(argp, fmt);

	vprintf(fmt, argp);
	fflush(stdout);
	va_end(argp);
#endif
}

void log_sm(const SquareMatrix* mat)
{
	printf("--------------------------\n");

	for (int y = 0; y < mat->size; y++)
	{
		for (int x = 0; x < mat->size; x++)
		{
			printf("%f", mat->data[y * mat->size + x]);
		}

		printf("\n");
	}

	printf("\n--------------------------");
}

void log_um(const UC_Matrix* mat)
{
	printf("--------------------------\n");

	for (int y = 0; y < mat->height; y++)
	{
		for (int x = 0; x < mat->width; x++)
		{
			printf("[%3d]", mat->data[y * mat->width + x]);
		}

		printf("\n");
	}

	printf("--------------------------\n");
}

void log_um_cap(const UC_Matrix* mat, unsigned int capW, unsigned int capH)
{
	printf("--------------------------\n");

	for (unsigned int y = 0; y < capH; y++)
	{
		for (unsigned int x = 0; x < capW; x++)
		{
			printf("[%3d]", mat->data[y * mat->width + x]);
		}

		printf("\n");
	}

	printf("--------------------------\n");
}

void log_im(const UI_Matrix* mat)
{
	printf("--------------------------\n");

	for (unsigned int y = 0; y < mat->height; y++)
	{
		for (unsigned int x = 0; x < mat->width; x++)
		{
			printf("[%3d]", mat->data[y * mat->width + x]);
		}

		printf("\n");
	}

	printf("--------------------------\n");
}

void log_im_cap(const UI_Matrix* mat, unsigned int capW, unsigned int capH)
{
	printf("--------------------------\n");

	for (unsigned int y = 0; y < capH; y++)
	{
		for (unsigned int x = 0; x < capW; x++)
		{
			printf("[%3d]", mat->data[y * mat->width + x]);
		}

		printf("\n");
	}

	printf("--------------------------\n");
}

void log_i(const Image* image)
{
	printf("--------------------------\n");

	int index = 0;

	for (unsigned int y = 0; y < image->height; y++)
	{
		for (unsigned int x = 0; x < image->width; x++)
		{
			printf(" [ ");

			for (unsigned char b = 0; b < image->bpp; b++)
			{

				printf("%d", image->data[index]);

				index++;
			}

			printf(" ] ");
		}

		printf("\n");
	}

	printf("--------------------------\n");
}

void log_d(const short* data, const unsigned int width, const unsigned int height, const unsigned char bpp)
{
	printf("--------------------------\n");

	int index = 0;

	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			printf(" [ ");

			for (unsigned char b = 0; b < bpp; b++)
			{

				printf("%d", data[index]);

				index++;
			}

			printf(" ] ");
		}

		printf("\n");
	}

	printf("--------------------------\n");
}


