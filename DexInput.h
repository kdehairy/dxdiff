
#ifndef _DEXINPUT_H_
#define _DEXINPUT_H_


#include <DexFile.h>

typedef struct _DexInput {
    DexFile* dexFile;
    MemMapping map;
    bool isMapped = false;
} DexInput;

void freeDexInput( DexInput* input ) {
    if ( input->isMapped ) {
        sysReleaseShmem( &input->map );
    }
    if ( input->dexFile != NULL ) {
        dexFileFree( input->dexFile );
    }
}

#endif // _DEXINPUT_H_