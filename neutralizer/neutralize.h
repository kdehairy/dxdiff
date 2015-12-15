//
// Created by kdehairy on 11/11/15.
//

#ifndef DEXPATCHER_NEUTRALIZE_H
#define DEXPATCHER_NEUTRALIZE_H

#include <DexFile.h>

/**
 It returns a copy of the base dex file with neutralized indices.

 We go through stringIds in both base and variant and find the unchanged string values, copy over their indices to
 the neutralized base dex. same with the rest of the dex sections.

 By the we end up with a base dex that has the same indices for unchanged values between it and the variant dex.
 */
void neutralizeBase( DexFile *pBaseDex, const DexFile *pVariantDex );

#endif //DEXPATCHER_NEUTRALIZE_H
