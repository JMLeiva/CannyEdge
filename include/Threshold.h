#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Structs.h"

void applyThreshold(const Image* src, const unsigned char threshold, Image* dst);
void applyThresholdLowHigh_c(const Image* src, const unsigned char thresholdlow, const unsigned char thresholdHigh, const unsigned char vlow, const unsigned char vHigh, Image* dst);
void applyThresholdLowHigh_asm(const Image* src, const unsigned char thresholdlow, const unsigned char thresholdHigh, const unsigned char vlow, const unsigned char vHigh, Image* dst);
extern void applyThresholdLowHigh_asm_impl_1bpp(const Image* src, const unsigned char thresholdlow, const unsigned char thresholdHigh, const unsigned char vlow, const unsigned char vHigh, Image* dst);
void applyHysteresisThreshold(const Image* src, Image* dst);
void applyHysteresisThreshold2(const Image* src, Image* dst);
#endif
