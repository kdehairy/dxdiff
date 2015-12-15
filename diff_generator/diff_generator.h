//
// Created by kdehairy on 12/8/15.
//

#ifndef DEXPATCHER_DIFF_GENERATOR_H
#define DEXPATCHER_DIFF_GENERATOR_H

#include <stdio.h>
#include <DexFile.h>

void generateDiff( void *pBaseDex, size_t baseSize, void *pVariantDex,
        size_t varSize, FILE *pOut );

#endif //DEXPATCHER_DIFF_GENERATOR_H
