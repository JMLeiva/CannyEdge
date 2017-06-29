
#define _MATH_DEFINES_DEFINED

#include <stdlib.h>
#include <stdio.h>
#include "include/Utils.h"
#include "include/Grayscale.h"
#include "include/GaussBlur.h"
#include "include/Threshold.h"
#include "include/SobelOperator.h"
#include "include/NonMaxSuppression.h"
#include "lodepng/lodepng.h"
#include "include/Structs.h"
#include "include/Log.h"
#include <string.h>

#define IMPL_C 0
#define IMPL_ASM 1
#define IMPL_ASM_YMM 2

bool benchamkEnabled;
bool outputImages;
unsigned char impl;

void applyCanny(const Image* src, unsigned char gaussRadius, float gaussSigma, unsigned char minThreshold, unsigned char maxThreshold);
Image* decodePng(const char* filename, unsigned char bpp);
void encodePng(const char* filename, Image* image);

int check_xcr0_ymm()
{
    uint32_t xcr0;
#if defined(_MSC_VER)
	xcr0 = (uint32_t)_xgetbv(0);  /* min VS2010 SP1 compiler is required */
#else
	__asm__("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx");
#endif
	return ((xcr0 & 6) == 6); /* checking if xmm and ymm state are enabled in XCR0 */
}

// Command Args
char* getCmdOption(char ** begin, char ** end, const char* option)
{
	size_t size = strlen(option);

	while(begin < end)
	{
		char* r = strstr(*begin, option);
		if(r != NULL && *(*begin+size) == NULL)
		{
			return *(++begin);
		}

		begin++;
	}

	return 0;
}


bool cmdOptionExists(char** begin, char** end, const char* option)
{
	size_t size = strlen(option);

	while(begin < end)
	{
		char* r = strstr(*begin, option);
		if(r != NULL && *(*begin+size) == NULL)
		{
			return TRUE;
		}

		begin++;
	}

	return FALSE;
}


uint64_t grayscale_t = 0, gauss_t = 0, sobel_t = 0, nonMax_t = 0, lowHigh_t = 0, hysteresis_t = 0;
uint64_t temp_t;

int main(int argc, char * argv[])
{
	unsigned int times = 1;

	unsigned char gaussRadius = 2;
	float gaussSigma = 1.4f;
	unsigned char minThreshold = 30;
	unsigned char maxThreshold = 120;


	benchamkEnabled = FALSE;
	outputImages = FALSE;
	impl = IMPL_C;
	char* src_path = "SRC-64.png";

	unsigned char load_bpp = 4;

	bool error = FALSE;

	if (cmdOptionExists(argv, argv + argc, "-t"))
	{
		char* c_times = getCmdOption(argv, argv + argc, "-t");

		times = parseInt(c_times, &error);

		if (error)
		{
			printf("-t MUST BE A POSITIVE INT\n");
			return -1;
		}
	}

	if (cmdOptionExists(argv, argv + argc, "-gr"))
	{
		char* c_gaussRadius = getCmdOption(argv, argv + argc, "-gr");

		gaussRadius = parseInt(c_gaussRadius, &error);

		if (error)
		{
			printf("-gr MUST BE A POSITIVE INT\n");
			return -1;
		}

		if (gaussRadius > 7)
		{
			printf("-gr MUST BE less than 7\n");
			return -1;
		}

		printf("Gauss Radius = %d\n", gaussRadius);
	}

	if (cmdOptionExists(argv, argv + argc, "-gs"))
	{
		char* c_gaussSigma = getCmdOption(argv, argv + argc, "-gs");

		gaussSigma = parseFloat(c_gaussSigma, &error);

		if (error)
		{
			printf("-gs MUST BE A FLOAT\n");
			return -1;
		}

		printf("Gauss Sigma = %f\n", gaussSigma);
	}

	if (cmdOptionExists(argv, argv + argc, "-mint"))
	{
		char* c_minThreshold = getCmdOption(argv, argv + argc, "-mint");

		unsigned int i_minThreshold = parseInt(c_minThreshold, &error);

		if (error)
		{
			printf("-mint MUST BE A POSITIVE CHAR\n");
			return -1;
		}

		if (i_minThreshold > 255)
		{
			printf("-mint MUST BE A CHAR (0 - 255)\n");
			return -1;
		}

		minThreshold = i_minThreshold;
	}

	if (cmdOptionExists(argv, argv + argc, "-maxt"))
	{
		char* c_maxThreshold = getCmdOption(argv, argv + argc, "-maxt");

		unsigned int i_maxThreshold = parseInt(c_maxThreshold, &error);

		if (error)
		{
			printf("-maxt MUST BE A INT\n");
			return -1;
		}

		if (i_maxThreshold > 255)
		{
			printf("-maxt MUST BE A CHAR (0 - 255)\n");
			return -1;
		}

		maxThreshold = i_maxThreshold;
	}

	if (cmdOptionExists(argv, argv + argc, "-bpp"))
	{
		char* c_bpp = getCmdOption(argv, argv + argc, "-bpp");

		unsigned int i_bpp = parseInt(c_bpp, &error);

		if (error)
		{
			printf("-maxt MUST BE A INT\n");
			return -1;
		}

		if (i_bpp > 4 || i_bpp < 3)
		{
			printf("-bpp MUST BE A CHAR (3 - 4)\n");
			return -1;
		}

		load_bpp = i_bpp;
	}

	outputImages = cmdOptionExists(argv, argv + argc, "-oe");
	benchamkEnabled = cmdOptionExists(argv, argv + argc, "-be");

	if (cmdOptionExists(argv, argv + argc, "-impl"))
	{
		const char* c_impl = getCmdOption(argv, argv + argc, "-impl");

		if(strcmp(c_impl, "c") == 0 || strcmp(c_impl, "C") == 0)
		{
			impl = IMPL_C;
		}
		else if(strcmp(c_impl, "asm") == 0 || strcmp(c_impl, "ASM") == 0)
		{
			impl = IMPL_ASM;
		}
		else if(strcmp(c_impl, "asm_y") == 0 || strcmp(c_impl, "ASM_Y") == 0)
		{
			printf("-impl ASM_Y is not supported yet\n");
			return -1;
			//impl = IMPL_ASM_YMM;
		}
		else
		{
			printf("-impl MUST BE [C | ASM | ASM_Y]\n");
			return -1;
		}
	}

	if(cmdOptionExists(argv, argv + argc, "-i"))
	{
		src_path = getCmdOption(argv, argv + argc, "-i");
	}

	// IMAGE LOAD
	log_info("Starting...\n");

	Image* src = decodePng(src_path, load_bpp);

	if (src == NULL)
	{
		return 1;
	}

	log_info("Image loaded successfully\n");
	log_info("Starting Processing %d times\n", times);

	for (unsigned int i = 0; i < times; i++)
	{
		applyCanny(src, gaussRadius, gaussSigma, minThreshold, maxThreshold);
		log_info_flush(".");
	}

	log_info("\n");
	free(src->data);
	free(src);

	if (benchamkEnabled)
	{
		log_info("Grayscale	Time %lld \n", grayscale_t / times);
		log_info("Gauss Blur	Time %lld \n", gauss_t / times);
		log_info("Sobel		Time %lld \n", sobel_t / times);
		log_info("NonMax		Time %lld \n", nonMax_t / times);
		log_info("LowHigh		Time %lld \n", lowHigh_t / times);
		log_info("Hysteresis	Time %lld \n", hysteresis_t / times);
		log_info("\nTotal: %lld \n", (grayscale_t + gauss_t + sobel_t + nonMax_t + lowHigh_t + hysteresis_t) / times);
	}

	log_info("COMPLTED\n\n");
}

void applyCanny(const Image* src, unsigned char gaussRadius, float gaussSigma, unsigned char minThreshold, unsigned char maxThreshold)
{
	Image grayScale, gauss, sobelLum, sobelAngle, nonMax, lowHigh, hysteresis;


	// GRAYSCALE

	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	switch(impl)
	{
	case IMPL_C:
		applyGrayscale_c(src, &grayScale);
		break;
	case IMPL_ASM:
		//applyGrayscale_c(src, &grayScale);
		applyGrayscale_asm(src, &grayScale);
		break;
	case IMPL_ASM_YMM:
		// TODO
		//applyGrayscale_asm(src, &grayScale);
		break;
	}

	if (benchamkEnabled)
	{
		grayscale_t += rdtsc() - temp_t;
	}

	log_verbose("Grayscale completed\n");

	// GAUSS BLUR
	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	switch(impl)
	{
	case IMPL_C:
		applyGaussBlur_c(&grayScale, gaussRadius * 2 + 1, gaussSigma, &gauss);
		break;
	case IMPL_ASM:
		applyGaussBlur_asm(&grayScale, gaussRadius * 2 + 1, gaussSigma, &gauss);
		break;
	case IMPL_ASM_YMM:
		// TODO
		break;
	}


	if (benchamkEnabled)
	{
		gauss_t += rdtsc() - temp_t;
	}

	log_verbose("Gauss Blur completed\n");

	// SOBEL
	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	switch(impl)
	{
	case IMPL_C:
		applySobelOperator_c(&gauss, &sobelLum, &sobelAngle);
		break;
	case IMPL_ASM:
		applySobelOperator_asm(&gauss, &sobelLum, &sobelAngle);
		break;
	case IMPL_ASM_YMM:
		// TODO
		break;
	}


	if (benchamkEnabled)
	{
		sobel_t += rdtsc() - temp_t;
	}

	log_verbose("Sobel Operator completed\n");

	// NONMAX SUPRESSION
	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	applyNonMaxSuppression(&sobelLum, &sobelAngle, &nonMax);

	if (benchamkEnabled)
	{
		nonMax_t += rdtsc() - temp_t;
	}

	log_verbose("NonMax Supression completed\n");

	// LOWHIGH THRESSHOLD
	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	switch(impl)
	{
	case IMPL_C:
		applyThresholdLowHigh_c(&nonMax, minThreshold, maxThreshold, 128, 255, &lowHigh);
		break;
	case IMPL_ASM:
		applyThresholdLowHigh_asm(&nonMax, minThreshold, maxThreshold, 128, 255, &lowHigh);
		break;
	case IMPL_ASM_YMM:
		// TODO
		break;
	}



	if (benchamkEnabled)
	{
		lowHigh_t += rdtsc() - temp_t;
	}

	log_verbose("LowHigh Thresshold completed\n");

	// HYSTERESIS
	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	applyHysteresisThreshold(&lowHigh, &hysteresis);

	if (benchamkEnabled)
	{
		hysteresis_t += rdtsc() - temp_t;
	}

	log_verbose("Hysteresis completed\n");

	if (outputImages)
	{
		Image xS, yS;

		switch(impl)
		{
		case IMPL_C:
			yS = ySobel_c(&gauss);
			xS = xSobel_c(&gauss);
			break;
		case IMPL_ASM:
			yS = ySobel_asm(&gauss);
			xS = xSobel_asm(&gauss);
			break;
		case IMPL_ASM_YMM:
			// TODO
			break;
		}

		log_verbose("Outputing Steps\n");
		encodePng("01-Grayscale.png", &grayScale);
		log_verbose(".");
		encodePng("02-Gauss.png", &gauss);
		log_verbose(".");
		log_verbose(".");
		encodePng("03A-SobelX.png", &xS);
		log_verbose(".");
		log_verbose(".");
		encodePng("03B-SobelY.png", &yS);
		log_verbose(".");
		encodePng("03C-Sobel.png", &sobelLum);
		log_verbose(".");
		encodePng("04-NonMax.png", &nonMax);
		log_verbose(".");
		encodePng("05-LowHigh.png", &lowHigh);
		log_verbose(".");
		encodePng("06-Result.png", &hysteresis);
		log_verbose(".");

		free(xS.data);
		free(yS.data);
	}

	free(grayScale.data);
	free(gauss.data);
	free(sobelLum.data);
	free(sobelAngle.data);
	free(nonMax.data);
	free(lowHigh.data);
	free(hysteresis.data);
}


Image* decodePng(const char* filename, unsigned char bpp)
{
	Image* image = (Image*)malloc(sizeof(Image));
	unsigned error;

	if(bpp == 3)
	{
		error = lodepng_decode24_file(&image->data, &image->width, &image->height, filename);
	}
	else if(bpp == 4)
	{
		error = lodepng_decode32_file(&image->data, &image->width, &image->height, filename);
	}
	else
	{
		printf("error: BPP=%d not supported", bpp);
		return NULL;
	}

	image->bpp = bpp;

	if (error)
	{
		printf("error %u: %s\n", error, lodepng_error_text(error));
		free(image);
		image = NULL;
	}
	

	return image;
}

void encodePng(const char* filename, Image* image)
{
	/*Encode the image*/
	unsigned error = 0;

	if (image->bpp == 4)
	{
		lodepng_encode32_file(filename, image->data, image->width, image->height);
	}
	if (image->bpp == 3)
	{
		lodepng_encode24_file(filename, image->data, image->width, image->height);
	}
	else if (image->bpp == 1)
	{
		error = lodepng_encode_file(filename, image->data, image->width, image->height, LCT_GREY, 8);
	}

	/*if there's an error, display it*/
	if (error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

