//
// Created by kdehairy on 12/8/15.
//

#ifndef DEXPATCHER_BSDIFF_HELPER_H
#define DEXPATCHER_BSDIFF_HELPER_H

#include <stddef.h>

typedef void (*fWriteDiffCallback)( void *data, size_t size, int status );

size_t generateBsDiff( void *baseData, size_t baseSize, void *variantData,
        size_t variantSize, fWriteDiffCallback callback );

#endif //DEXPATCHER_BSDIFF_HELPER_H
