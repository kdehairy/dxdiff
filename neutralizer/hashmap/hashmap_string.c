//
// Created by kdehairy on 11/16/15.
//

#include <malloc.h>
#include <string.h>
#include "hashmap_string.h"

#define INITIAL_SIZE 1024

typedef struct _hashmap_string_entry {
    const char *key;
    void *value;
    int inUse;
} hashmap_string_entry;

typedef struct _hashmap_string_map {
    uint32_t table_size;
    size_t size;
    hashmap_string_entry *data;
} hashmap_string_map;

map_string_t hashmap_string_new() {
    hashmap_string_map *map = (hashmap_string_map *) malloc( sizeof( hashmap_string_map ));
    if ( !map ) {
        goto fail;
    }

    map->data = (hashmap_string_entry *) calloc( INITIAL_SIZE, sizeof( hashmap_string_entry ));
    if ( !map->data ) {
        goto fail;
    }

    map->table_size = INITIAL_SIZE;
    map->size = 0;

    return map;

    fail:
    if ( map ) {
        hashmap_string_free( map );
    }
    return NULL;
}

static uint32_t hashmap_string_hash_string( const char *key ) {
    /* java implementation of string hash function
     * http://docs.oracle.com/javase/1.5.0/docs/api/java/lang/String.html#hashCode()
     */
    uint32_t result = 0;
    size_t size = strlen( key );

    for ( int i = 0; i < size; ++i ) {
        result += key[i] * ( 31^( size - ( i + 1 ) ) );
    }

    return result;
}

static int hashmap_string_hash( hashmap_string_map *map, const char *key ) {
    if ( map->size == map->table_size ) {
        return MAP_FULL;
    }

    uint32_t hash = hashmap_string_hash_string( key );

    if ( map->data[hash].inUse == 0 ) {
        return hash;
    }
    if ( strcmp( map->data[hash].key, key ) == 0 ) {
        return hash;
    }

    return MAP_COLLISION;
}

static int hashmap_string_rehash( hashmap_string_map *map ) {
    hashmap_string_entry *tmp = (hashmap_string_entry *) calloc( 2 * map->table_size, sizeof( hashmap_string_entry ) );
    if ( ! tmp ) {
        return MAP_OMEM;
    }

    hashmap_string_entry *current = map->data;
    map->data = tmp;

    uint32_t oldSize = map->table_size;
    map->table_size = 2 * map->table_size;
    map->size = 0;

    for ( int i = 0; i < oldSize; ++i ) {
        int status = hashmap_string_put( map, current[i].key, current[i].value );
        if ( status != MAP_OK ) {
            return status;
        }
    }
    free( current );

    return MAP_OK;
}

int hashmap_string_put( map_string_t map, const char *key, void *value ) {
    hashmap_string_map *_map = (hashmap_string_map *) map;

    int hash = hashmap_string_hash( _map, key );

    while ( hash == MAP_FULL ) {
        if ( hashmap_string_rehash( _map ) == MAP_OMEM ) {
            return MAP_OMEM;
        }
        hash = hashmap_string_hash( _map, key );
    }

    _map->data[hash].value = value;
    _map->data[hash].key = key;
    _map->data[hash].inUse = 1;
    _map->size++;

    return MAP_OK;
}

int hashmap_string_get( map_string_t map, const char *key, void **value) {
    hashmap_string_map *_map = (hashmap_string_map *) map;

    int hash = hashmap_string_hash_string( key );

    if ( strcmp( _map->data[hash].key, key ) == 0 && _map->data[hash].inUse == 1 ) {
        *value = _map->data[hash].value;
        return MAP_OK;
    }
    return MAP_NOT_FOUND;
}

void hashmap_string_free( map_string_t map ) {
    hashmap_string_map *_map = (hashmap_string_map *) map;
    free( _map->data );
    free( _map );
}

size_t hashmap_string_size( map_string_t map ) {
    hashmap_string_map *_map = (hashmap_string_map *) map;
    return _map->size;
}
