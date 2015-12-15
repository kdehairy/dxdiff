//
// Created by kdehairy on 12/8/15.
//

#include <malloc.h>
#include <string.h>
#include "diff_generator.h"
#include "../bsdiff_helper.h"

static void *pData = NULL;

void callback( void *data, size_t size, int status ) {
    if ( status < 0 ) {
        return;
    }
    pData = malloc( size );
    memcpy( pData, data, size );
    pData = data;
}

void generateDiff( void *pBaseDex, size_t baseSize, void *pVariantDex,
        size_t varSize, FILE *pOut ) {

    size_t size = generateBsDiff( pBaseDex, baseSize, pVariantDex, varSize, &callback );
    if ( pData == NULL ) {
        return;
    }
    if ( fseeko( pOut, 0, SEEK_SET ) ) {
        fprintf( stderr, "failed to seek to start of file\n" );
        free( pData );
        return;
    }
    printf( "diff size to write: %zu\n", size );
    if ( fwrite( pData, size, 1, pOut ) != 1 ) {
        fprintf( stderr, "failed to write to file\n" );
        free( pData );
        return;
    }
    free( pData );
}