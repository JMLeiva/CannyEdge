#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Structs.h"

void applyThreshold(Image* src, unsigned char threshold, Image* dst);
void applyThresholdLowHigh(Image* src, unsigned char thresholdlow, unsigned char thresholdHigh, unsigned char vlow, unsigned char vHigh, Image* dst);
void applyHysteresisThreshold(Image* src, Image* dst);
void applyHysteresisThreshold2(Image* src, Image* dst);
#endif
