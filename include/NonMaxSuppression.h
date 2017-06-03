#ifndef NON_MAX_SUPPRESSION_H
#define NON_MAX_SUPPRESSION_H

#include "Structs.h"

void applyNonMaxSuppression(const Image* lum, const Image* angle, Image* dst);
#endif
