//
// Created by kdehairy on 12/8/15.
//

#include <unistd.h>
#include <bsdiff.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "bsdiff_helper.h"

typedef struct _DiffBuffer {
    uint8_t *buff;
    size_t size;
    off_t pos;
    int status;
    fWriteDiffCallback callback;
} DiffBuffer;

static void initStream( DiffStream *stream, size_t buffSize ) {
    DiffBuffer *buffer = (DiffBuffer *) stream->opaque;
    buffer->buff = (uint8_t *) malloc( buffSize );
    memset( buffer->buff, 0, buffSize );

    buffer->size = buffSize;
    buffer->pos = stream->headerSize;
}

static size_t writeStream( DiffStream *stream, const void *data, size_t size ) {
    DiffBuffer *buffer = (DiffBuffer *) stream->opaque;
    if ( buffer->pos + size > buffer->size ) {
        fprintf( stderr,
                "failed to write diff stream patch. buffer overflow\n" );
        buffer->status = -1;
        return 0;
    }
    memcpy( buffer->buff + buffer->pos, data, size );
    buffer->pos += size;
    return size;
}

static size_t writeHeaderStream( DiffStream *stream, const void *data ) {
    DiffBuffer *buffer = (DiffBuffer *) stream->opaque;
    if ( buffer->pos + stream->headerSize > buffer->size ) {
        fprintf( stderr, "failed to write diff stream. buffer overflow\n" );
        buffer->status = -1;
        return 0;
    }
    memcpy( buffer->buff, data, stream->headerSize );
    return stream->headerSize;
}

static void endStream( DiffStream *stream ) {
    DiffBuffer *buffer = (DiffBuffer *) stream->opaque;
    buffer->callback( buffer->buff, buffer->size, buffer->status );
}

size_t generateBsDiff( void *baseData, size_t baseSize, void *variantData,
        size_t variantSize, fWriteDiffCallback callback ) {
    DiffBuffer buffer;
    buffer.callback = callback;
    buffer.status = 0;
    DiffStream stream;
    stream.opaque = &buffer;
    stream.write = writeStream;
    stream.writeHeader = writeHeaderStream;
    stream.init = initStream;
    stream.end = endStream;

    size_t size = bsdiff((uint8_t *) baseData, baseSize,
            (uint8_t *) variantData, variantSize, &stream );
    if ( size == 0 ) {
        fprintf( stderr, "failed to bsdiff the stream\n" );
        if ( buffer.buff != NULL ) {
            free( buffer.buff );
        }
        return 0;
    }
    return size;
}
