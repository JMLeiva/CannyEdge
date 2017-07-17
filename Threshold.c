#include "include/Threshold.h"
#include "include/Common.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

void flattenConfilctMap(unsigned int* map, unsigned int size);
int maxValueForConflictGroup(unsigned int* map, unsigned int size, unsigned int index);

void applyThreshold(const Image* src, const unsigned char threshold, Image* dst) {
	emptyImageWithFormat(src->width, src->height, src->bpp, dst);

	for (unsigned int i = 0; i < src->width * src->height; i += src->bpp) {
		unsigned short lum = 0;

		// Alpha se esta considerando como una canal de luz mas.
		for (unsigned char b = 0; b < src->bpp; b++) {
			lum += src->data[i + b];
		}

		lum /= src->bpp;

		if (lum < threshold) {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = 0;
			}
		} else {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = src->data[i + b];
			}
		}
	}
}

void applyThresholdLowHigh_c(const Image* src, const unsigned char thresholdlow,
		const unsigned char thresholdHigh, const unsigned char vlow,
		const unsigned char vHigh, Image* dst) {


	assert(src->bpp == 1);

	emptyImageWithFormat(src->width, src->height, src->bpp, dst);



	/*for (unsigned int i = 0; i < src->width * src->height; i += src->bpp) {
		unsigned short lum = 0;

		// Alpha se esta considerando como una canal de luz mas.
		for (unsigned char b = 0; b < src->bpp; b++) {
			lum += src->data[i + b];
		}

		lum /= src->bpp;

		if (lum < thresholdlow) {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = 0;
			}
		} else if (lum < thresholdHigh) {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = vlow;
			}
		} else {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = vHigh;
			}
		}
	}*/

	for (unsigned int i = 0; i < src->width * src->height; i += 1) {
		unsigned short lum = 0;

		// Alpha se esta considerando como una canal de luz mas.
		lum += src->data[i];

		lum /= src->bpp;

		if (lum < thresholdlow) {
			dst->data[i] = 0;

		} else if (lum < thresholdHigh) {
			dst->data[i] = vlow;

		} else {
			dst->data[i] = vHigh;
		}
	}
}

void applyThresholdLowHigh_asm(const Image* src,
		const unsigned char thresholdlow, const unsigned char thresholdHigh,
		const unsigned char vlow, const unsigned char vHigh, Image* dst) {

	assert(src->bpp == 1);

	applyThresholdLowHigh_asm_impl_1bpp(src, thresholdlow, thresholdHigh, vlow,
			vHigh, dst);

	// Last bytes if not 16 byte multiple
	unsigned int totalSize = src->width * src->height * src->bpp;

	unsigned int unalignedStart = totalSize;

	/*while (totalSize - unalignedStart < 16) {
		unalignedStart -= dst->bpp;
	}


	for (unsigned int i = unalignedStart; i < totalSize; i += src->bpp) {
		unsigned short lum = 0;

		// Alpha se esta considerando como una canal de luz mas.
		for (unsigned char b = 0; b < src->bpp; b++) {
			lum += src->data[i + b];
		}

		lum /= src->bpp;

		if (lum < thresholdlow) {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = 0;
			}
		} else if (lum < thresholdHigh) {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = vlow;
			}
		} else {
			for (unsigned char b = 0; b < src->bpp; b++) {
				dst->data[i + b] = vHigh;
			}
		}
	}*/

	while (totalSize - unalignedStart < 16) {
		unalignedStart -= 1;
	}

	for (unsigned int i = unalignedStart; i < totalSize; i += src->bpp) {
		unsigned short lum = 0;

		// Alpha se esta considerando como una canal de luz mas.
		lum += src->data[i];

		lum /= src->bpp;

		if (lum < thresholdlow) {
			dst->data[i] = 0;

		} else if (lum < thresholdHigh) {
			dst->data[i] = vlow;

		} else {
			dst->data[i] = vHigh;
		}
	}
}

void applyHysteresisThreshold(const Image* src, Image* dst) {
	// TODO
	assert(src->bpp == 1);

	emptyImageWithFormat(src->width, src->height, src->bpp, dst);

	// Groups cant exceed (width/2 + 1) * (height/2 + 1)
	unsigned int maxGroupSize = ((src->width / 2 + 1) * (src->height / 2) + 1);

	unsigned int* conflictMap = (unsigned int*) malloc(
			maxGroupSize * sizeof(int));
	unsigned char* valueMap = (unsigned char*) malloc(
			maxGroupSize * sizeof(char));

	memset(conflictMap, 0, maxGroupSize * sizeof(int));
	memset(valueMap, 0, maxGroupSize);

	UI_Matrix hystMatrix = getUI_Matrix(src->width, src->height);

	int uIndex, lIndex, ulIndex, urIndex;
	unsigned int lastNewGroup = 0;

	// ROW BY ROW PASS
	for (unsigned int index = 0; index < src->width * src->height; index++) {
		int a = 0;

		if (index == 527) {
			a++;
		}

		if (src->data[index] == 0) {
			hystMatrix.data[index] = 0;
			continue;
		}

		uIndex = index - src->width;
		lIndex = index - 1;
		ulIndex = index - (src->width + 1);
		urIndex = index - (src->width - 1);

		int parentVals[] = { -1, -1, -1, -1 };

		if (uIndex > 0) {
			parentVals[0] = hystMatrix.data[uIndex];
		}
		if (lIndex > 0) {
			parentVals[1] = hystMatrix.data[lIndex];
		}
		if (ulIndex > 0) {
			parentVals[2] = hystMatrix.data[ulIndex];
		}
		if (urIndex > 0) {
			parentVals[3] = hystMatrix.data[urIndex];
		}

		int firstValue = -1;
		int fisrtValueIndex = -1;
		bool conflicted = 0;

		for (int v = 0; v < 4; v++) {
			if (parentVals[v] > 0) {
				if (firstValue < 0) {
					firstValue = parentVals[v];
					fisrtValueIndex = v;
				} else {
					if (parentVals[v] > 0 && parentVals[v] != firstValue) {
						conflicted = 1;
						break;
					}
				}
			}
		}

		if (firstValue < 0) {
			// New Group (may have conflicts later on)
			lastNewGroup++;
			hystMatrix.data[index] = lastNewGroup;

			continue;
		} else {
			hystMatrix.data[index] = firstValue;

			if (!conflicted) {
				continue;
			} else {
				for (int v = fisrtValueIndex + 1; v < 4; v++) {
					if (parentVals[v] > 0) {
						// Check Not Equals
						if (parentVals[v] == firstValue) {
							continue;
						}

						// Avoid circular reference						
						if (conflictMap[firstValue] != 0
								&& conflictMap[firstValue] == parentVals[v]) {
							continue;
						}

						if (parentVals[v] == 1) {
							//printf("asdasd");
						}

						conflictMap[parentVals[v]] = firstValue;
					}
				}
			}
		}
	}


	//FlATTEN CONFLICT MAP
	flattenConfilctMap(conflictMap, maxGroupSize);

	// RELABEL
	for (unsigned int index = 0; index < hystMatrix.width * hystMatrix.height;
			index++) {
		unsigned int group = hystMatrix.data[index];

		if (conflictMap[group] == 0) {
			continue;
		}

		if (hystMatrix.data[index] != 0) {
			hystMatrix.data[index] = conflictMap[hystMatrix.data[index]];
		}
	}


	// SET MAX
	// Check all pixels with same group, and select the maximun value for that group
	for (unsigned int index = 0; index < hystMatrix.width * hystMatrix.height;
			index++) {
		unsigned int group = hystMatrix.data[index];

		if (valueMap[group] != 0) {
			valueMap[group] =
					valueMap[group] > src->data[index] ?
							valueMap[group] : src->data[index];
		} else {
			valueMap[group] = src->data[index];
		}
	}

	// PAINT
	for (unsigned int index = 0; index < hystMatrix.width * hystMatrix.height;
			index++) {
		unsigned int group = hystMatrix.data[index];

		if (group == 0) {
			dst->data[index] = 0;
		} else {
			dst->data[index] = valueMap[group] == 255 ? 255 : 0;
		}
	}

	free(hystMatrix.data);
	free(conflictMap);
	free(valueMap);
}

void flattenConfilctMap(unsigned int* map, unsigned int size)
{
	for(unsigned int index = 0; index < size; index++)
	{
		map[index] = maxValueForConflictGroup(map, size, index);
	}
}

int maxValueForConflictGroup(unsigned int* map, unsigned int size, unsigned int index)
{
	unsigned int maxValue = map[index];

	if(index > maxValue)
	{
		return index;
	}

	while(TRUE)
	{
		if(maxValue == 0) break;
		if(maxValue >= size) break;
		if(map[maxValue] == 0) break;

		return maxValueForConflictGroup(map, size, maxValue);
	}

	return maxValue;
}
