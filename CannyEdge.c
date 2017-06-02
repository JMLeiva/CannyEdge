
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

char benchamkEnabled;
char outputImages;

void applyCanny(Image* src, unsigned char gaussRadius, float gaussSigma, unsigned char minThreshold, unsigned char maxThreshold);
Image* decodePng(const char* filename);
void encodePng(const char* filename, Image* image);

// Command Args
char* getCmdOption(char ** begin, char ** end, const char* option)
{
	while(begin < end)
	{
		if(strstr(*begin, option) != NULL)
		{
			return *(++begin);
		}

		begin++;
	}

	return 0;
}


bool cmdOptionExists(char** begin, char** end, const char* option)
{
	while(begin < end)
	{
		if(strstr(*begin, option) != NULL)
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
	benchamkEnabled = 0;
	outputImages = 0;

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

		if (gaussRadius > 20)
		{
			printf("-gr MUST BE less than 20\n");
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

	outputImages = cmdOptionExists(argv, argv + argc, "-oe");
	benchamkEnabled = cmdOptionExists(argv, argv + argc, "-be");

	// IMAGE LOAD
	log_info("Starting...\n");

	Image* src = decodePng("SRC.png");

	if (src == NULL)
	{
		return 1;
	}

	log_info("Image loaded successfully\n");
	log_info("Starting Processing %d times\n", times);

	for (int i = 0; i < times; i++)
	{
		applyCanny(src, gaussRadius, gaussSigma, minThreshold, maxThreshold);
		log_info("%d\n", i);
	}

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

void applyCanny(Image* src, unsigned char gaussRadius, float gaussSigma, unsigned char minThreshold, unsigned char maxThreshold)
{
	Image grayScale, gauss, sobelLum, sobelAngle, nonMax, lowHigh, hysteresis;


	// GRAYSCALE

	if (benchamkEnabled)
	{
		temp_t = rdtsc();
	}

	applyGrayscale(src, &grayScale);

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

	applyGaussBlur(&grayScale, gaussRadius * 2 + 1, gaussSigma, &gauss);

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

	applySobelOperator(&gauss, &sobelLum, &sobelAngle);

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

	applyThresholdLowHigh(&nonMax, minThreshold, maxThreshold, 128, 255, &lowHigh);

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
		log_verbose("Outputing Steps\n");
		encodePng("01-Grayscale.png", &grayScale);
		log_verbose(".");
		encodePng("02-Gauss.png", &gauss);
		log_verbose(".");
		Image xS = xSobel(&gauss);
		log_verbose(".");
		encodePng("03A-SobelX.png", &xS);
		log_verbose(".");
		Image yS = ySobel(&gauss);
		log_verbose(".");
		encodePng("03B-SobelY.png", &yS);
		log_verbose(".");
		encodePng("03C-.png", &sobelLum);
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


Image* decodePng(const char* filename)
{
	Image* image = (Image*)malloc(sizeof(Image));
	unsigned error;

	error = lodepng_decode24_file(&image->data, &image->width, &image->height, filename);
	image->bpp = 3;

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

