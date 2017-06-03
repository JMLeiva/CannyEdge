#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Structs.h"

void applyThreshold(const Image* src, const unsigned char threshold, Image* dst);
void applyThresholdLowHigh(const Image* src, const unsigned char thresholdlow, const unsigned char thresholdHigh, const unsigned char vlow, const unsigned char vHigh, Image* dst);
void applyHysteresisThreshold(const Image* src, Image* dst);
void applyHysteresisThreshold2(const Image* src, Image* dst);
#endif
