
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
#include <math.h>

#define IMPL_C 0
#define IMPL_ASM 1
#define IMPL_ASM_YMM 2

bool benchmarkEnabled;
bool outputImages;
unsigned char impl;

uint64_t getBenchmarkTotal(Benchmark* benchmark);
uint64_t getMean(Benchmark* benchmarkList, unsigned int size);
double getStdDeviation(Benchmark* benchmarkList, unsigned int size, uint64_t mean);

void applyCanny(const Image* src, unsigned char gaussRadius, float gaussSigma, unsigned char minThreshold, unsigned char maxThreshold, Benchmark* benchmark);
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



Benchmark* benchmarkList;
uint64_t temp_t;

int main(int argc, char * argv[])
{
	unsigned int times = 1;

	unsigned char gaussRadius = 2;
	float gaussSigma = 1.4f;
	unsigned char minThreshold = 30;
	unsigned char maxThreshold = 120;


	benchmarkEnabled = TRUE;
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
	benchmarkEnabled = cmdOptionExists(argv, argv + argc, "-be");

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


	if(benchmarkEnabled)
	{
		benchmarkList = (Benchmark*)malloc(times * sizeof(Benchmark));
	}

	for (unsigned int i = 0; i < times; i++)
	{
		applyCanny(src, gaussRadius, gaussSigma, minThreshold, maxThreshold, &benchmarkList[i]);
		log_info_flush(".");
	}

	log_info("\n");
	free(src->data);
	free(src);



	if (benchmarkEnabled)
	{
		for(unsigned int i = 0; i < times; i++)
		{
			benchmarkList[i].valid = TRUE;
		}

		uint64_t mean = getMean(benchmarkList, times);
		double stdDeviation = getStdDeviation(benchmarkList, times, mean);

		// Remove ouliers
		for(unsigned int i = 0; i < times; i++)
		{
			uint64_t total = getBenchmarkTotal(&benchmarkList[i]);

			if(abs(total - mean) > 2*stdDeviation)
			{
				benchmarkList[i].valid = FALSE;
			}
		}

		//RECALCULATE ALL
		mean = getMean(benchmarkList, times);
		stdDeviation = getStdDeviation(benchmarkList, times, mean);

		// Get sub values
		Benchmark mixedBenchmark;
		mixedBenchmark.gauss_t = 0;
		mixedBenchmark.grayscale_t = 0;
		mixedBenchmark.hysteresis_t = 0;
		mixedBenchmark.lowHigh_t = 0;
		mixedBenchmark.nonMax_t = 0;
		mixedBenchmark.sobel_t = 0;
		mixedBenchmark.valid = TRUE;


		unsigned int outliers = 0;
		unsigned int realTimes = times;

		for(unsigned int i = 0; i < times; i++)
		{
			if(!benchmarkList[i].valid)
			{
				outliers++;
				realTimes--;
				continue;
			}

			mixedBenchmark.gauss_t += benchmarkList[i].gauss_t;
			mixedBenchmark.grayscale_t += benchmarkList[i].grayscale_t;
			mixedBenchmark.hysteresis_t += benchmarkList[i].hysteresis_t;
			mixedBenchmark.lowHigh_t += benchmarkList[i].lowHigh_t;
			mixedBenchmark.nonMax_t += benchmarkList[i].nonMax_t;
			mixedBenchmark.sobel_t += benchmarkList[i].sobel_t;
		}

		mixedBenchmark.gauss_t /= realTimes;
		mixedBenchmark.grayscale_t /= realTimes;
		mixedBenchmark.hysteresis_t /= realTimes;
		mixedBenchmark.lowHigh_t /= realTimes;
		mixedBenchmark.nonMax_t /= realTimes;
		mixedBenchmark.sobel_t /= realTimes;

		log_info("Grayscale	Time %lld \n", 		mixedBenchmark.grayscale_t);
		log_info("Gauss Blur	Time %lld \n", 	mixedBenchmark.gauss_t);
		log_info("Sobel		Time %lld \n", 		mixedBenchmark.sobel_t);
		log_info("NonMax		Time %lld \n", 	mixedBenchmark.nonMax_t);
		log_info("LowHigh		Time %lld \n", 	mixedBenchmark.lowHigh_t);
		log_info("Hysteresis	Time %lld \n", 	mixedBenchmark.hysteresis_t);
		log_info("\nTotal: 		%lld \n", mean);
		log_info("\nDeviation:	%f (%.02f%)\n", stdDeviation, (stdDeviation / mean) * 100);
		log_info("\Outliers:	%d (%.02f%)\n", outliers, ((float)outliers / times) * 100);
	}

	log_info("COMPLTED\n\n");
}

void applyCanny(const Image* src, unsigned char gaussRadius, float gaussSigma, unsigned char minThreshold, unsigned char maxThreshold, Benchmark* benchmark)
{
	Image grayScale, gauss, sobelLum, sobelAngle, nonMax, lowHigh, hysteresis;


	// GRAYSCALE

	if (benchmarkEnabled)
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

	if (benchmarkEnabled)
	{
		benchmark->grayscale_t = rdtsc() - temp_t;
	}

	log_verbose("Grayscale completed\n");

	// GAUSS BLUR
	if (benchmarkEnabled)
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


	if (benchmarkEnabled)
	{
		benchmark->gauss_t = rdtsc() - temp_t;
	}

	log_verbose("Gauss Blur completed\n");

	// SOBEL
	if (benchmarkEnabled)
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


	if (benchmarkEnabled)
	{
		benchmark->sobel_t = rdtsc() - temp_t;
	}

	log_verbose("Sobel Operator completed\n");

	// NONMAX SUPRESSION
	if (benchmarkEnabled)
	{
		temp_t = rdtsc();
	}

	applyNonMaxSuppression(&sobelLum, &sobelAngle, &nonMax);

	if (benchmarkEnabled)
	{
		benchmark->nonMax_t = rdtsc() - temp_t;
	}

	log_verbose("NonMax Supression completed\n");

	// LOWHIGH THRESSHOLD
	if (benchmarkEnabled)
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



	if (benchmarkEnabled)
	{
		benchmark->lowHigh_t = rdtsc() - temp_t;
	}

	log_verbose("LowHigh Thresshold completed\n");

	// HYSTERESIS
	if (benchmarkEnabled)
	{
		temp_t = rdtsc();
	}

	applyHysteresisThreshold(&lowHigh, &hysteresis);

	if (benchmarkEnabled)
	{
		benchmark->hysteresis_t = rdtsc() - temp_t;
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

uint64_t getBenchmarkTotal(Benchmark* benchmark)
{
	return benchmark->gauss_t + benchmark->grayscale_t + benchmark->hysteresis_t + benchmark->lowHigh_t + benchmark->nonMax_t + benchmark->sobel_t;
}

uint64_t getMean(Benchmark* benchmarkList, unsigned int size)
{
	uint64_t result = 0;
	unsigned int validSize = size;
	// Calculate Mean
	for(unsigned int i = 0; i < size; i++)
	{
		if(!benchmarkList[i].valid)
		{
			validSize--;
			continue;
		}
		result += getBenchmarkTotal(&benchmarkList[i]);
	}

	result /= validSize;

	return result;
}


double getStdDeviation(Benchmark* benchmarkList, unsigned int size, uint64_t mean)
{
	uint64_t stdDeviation_acc = 0;
	double result = 0;
	unsigned int validSize = size;
	// Calculate Std deviaton
	for(unsigned int i = 0; i < size; i++)
	{
		if(!benchmarkList[i].valid)
		{
			validSize--;
			continue;
		}

		uint64_t total = getBenchmarkTotal(&benchmarkList[i]);

		uint64_t squaredDiff = total - mean;
		squaredDiff *= squaredDiff;

		stdDeviation_acc += squaredDiff;
	}

	result = (double)stdDeviation_acc / validSize;
	result = sqrt(result);

	return result;
}
