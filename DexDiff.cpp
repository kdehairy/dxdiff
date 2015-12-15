#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SysUtil.h>
#include <getopt.h>
#include "CmdUtils.h"
#include "DexInput.h"
#include "neutralizer/neutralize.h"
#include "diff_generator/diff_generator.h"

static const char *progName = "dxDiff";

typedef struct _Options {
    const char *baseDex;
    const char *newDex;
    const char *diffOut;
    uint8_t neutralize;
} Options;

Options options;
DexInput baseDexInput;
DexInput newDexInput;
FILE *pOut;

static void cleanup() {
    freeDexInput( &baseDexInput );
    freeDexInput( &newDexInput );
    if ( pOut != NULL ) {
        fclose( pOut );
    }
}

static int loadDexFile( const char *fileName, DexInput *input ) {

    // Mapping file to memory
    if ( dexOpenAndMap( fileName, NULL, &input->map, false ) != 0 ) {
        fprintf( stderr, "Failed to map the dex file '%s' in memory",
                fileName );
        return 1;
    }
    input->isMapped = true;

    //parsing dex file
    input->dexFile = dexFileParse((u1 *) input->map.addr, input->map.length,
            kDexParseVerifyChecksum );
    if ( input->dexFile == NULL ) {
        fprintf( stderr, "Failed to parse dex file '%s'", fileName );
        sysReleaseShmem( &input->map );
        input->isMapped = false;
        return 1;
    }

    return 0;
}

int parseArgs( int argc, char *const argv[] ) {
    int arg;
    while ( ( arg = getopt( argc, argv, "n" ) ) != -1 ) {
        switch ( arg ) {
            case 'n':
                options.neutralize = 1;
                break;
            default:
                fprintf( stderr, "UnRecognized parameter 'n'\n" );
                return 1;
        }
    }

    if ( optind + 2 > argc ) {
        fprintf( stderr, "Missing base, variant and patch file\n" );
        return 1;
    }

    options.baseDex = argv[optind];
    options.newDex = argv[optind + 1];
    options.diffOut = argv[optind + 2];

    return 0;
}

int main( int argc, char *const argv[] ) {
    int result = 0;
    memset( &options, 0, sizeof( Options ));

    if ( parseArgs( argc, argv ) ) {
        printf( "Usage: %s <base file> <new file> <patch file>\n", progName );
        return 1;
    }

    printf( "base dex: '%s'\n", options.baseDex );
    printf( "variant dex: '%s'\n", options.newDex );

    // load the base dex file
    result = loadDexFile( options.baseDex, &baseDexInput );
    if ( result != 0 ) {
        cleanup();
        return result;
    }

    // load the variant dex file
    result = loadDexFile( options.newDex, &newDexInput );
    if ( result != 0 ) {
        cleanup();
        return result;
    }

    if ( options.neutralize ) {
        sysChangeMapAccess( baseDexInput.map.addr, baseDexInput.map.length, true,
                &baseDexInput.map );

        neutralizeBase( baseDexInput.dexFile, newDexInput.dexFile );

        sysChangeMapAccess( baseDexInput.map.addr, baseDexInput.map.length, false,
                &baseDexInput.map );
    }

    pOut = fopen( options.diffOut, "w" );
    if ( pOut == NULL ) {
        cleanup();
        fprintf( stderr, "Cannot open file '%s'\n", options.diffOut );
        return 1;
    }

    printf( "== generating diff ==\n" );
    printf( "baseSize: %zu\n", baseDexInput.map.length );
    printf( "varSize: %zu\n", newDexInput.map.length );
    generateDiff( baseDexInput.map.addr, baseDexInput.map.length,
            newDexInput.map.addr, newDexInput.map.length, pOut );

    cleanup();
    return result;
}