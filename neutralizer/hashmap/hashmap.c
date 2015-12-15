//
// Created by kdehairy on 11/15/15.
//

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "hashmap.h"

#define INITIAL_SIZE 1024

typedef struct _hashmap_entry {
    uint32_t key;
    uint32_t value;
    int inUse;
} hashmap_entry;

typedef struct _hashmap_map {
    uint32_t table_size;
    size_t size;
    hashmap_entry *data;
} hashmap_map;

map_t hashmap_new() {
    hashmap_map *map = (hashmap_map *) malloc( sizeof( hashmap_map ));
    if ( !map ) {
        goto fail;
    }

    map->data = (hashmap_entry *) calloc( INITIAL_SIZE, sizeof( hashmap_entry ));
    if ( !map->data ) {
        goto fail;
    }

    map->table_size = INITIAL_SIZE;
    map->size = 0;

    return map;

    fail:
    if ( map ) {
        hashmap_free( map );
    }
    return NULL;
}


static uint32_t hashmap_hash_int( hashmap_map *map, uint32_t key ) {
    /* Robert Jenkins' 32 bit Mix Function */
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    /* Knuth's Multiplicative Method */
    key = (key >> 3) * 2654435761u;

    return key % map->table_size;
}

static int hashmap_hash( hashmap_map *map, uint32_t key ) {
    if ( map->size == map->table_size ) {
        return MAP_FULL;
    }

    uint32_t hash = hashmap_hash_int( map, key );

    for ( int i = 0; i < map->table_size; ++i ) {
        if ( map->data[hash].inUse == 0 ) {
            return hash;
        }
        if ( map->data[hash].key == key ) {
            return hash;
        }

        hash = (hash + 1) % map->table_size;
    }

    return MAP_FULL;
}

static int hashmap_rehash( hashmap_map *map ) {
    hashmap_entry *tmp = (hashmap_entry *) calloc( 2 * map->table_size, sizeof( hashmap_entry ) );
    if ( ! tmp ) {
        return MAP_OMEM;
    }

    hashmap_entry *current = map->data;
    map->data = tmp;

    uint32_t oldSize = map->table_size;
    map->table_size = 2 * map->table_size;
    map->size = 0;

    for ( int i = 0; i < oldSize; ++i ) {
        int status = hashmap_put( map, current[i].key, current[i].value );
        if ( status != MAP_OK ) {
            return status;
        }
    }
    free( current );

    return MAP_OK;
}

int hashmap_put( map_t map, uint32_t key, uint32_t value ) {
    hashmap_map *_map = (hashmap_map *) map;

    int hash = hashmap_hash( _map, key );

    while ( hash == MAP_FULL ) {
        if ( hashmap_rehash( _map ) == MAP_OMEM ) {
            return MAP_OMEM;
        }
        hash = hashmap_hash( _map, key );
    }

    _map->data[hash].value = value;
    _map->data[hash].key = key;
    _map->data[hash].inUse = 1;
    _map->size++;

    return MAP_OK;
}

int hashmap_get( map_t map, uint32_t key, uint32_t *value) {
    hashmap_map *_map = (hashmap_map *) map;

    int hash = hashmap_hash_int( _map, key );

    for ( int i = 0; i < _map->table_size; ++i ) {
        if ( _map->data[hash].key == key && _map->data[hash].inUse == 1 ) {
            *value = _map->data[hash].value;
            return MAP_OK;
        }
        hash = (hash + 1) % _map->table_size;
    }
    return MAP_NOT_FOUND;
}

void hashmap_free( map_t map ) {
    hashmap_map *_map = (hashmap_map *) map;
    free( _map->data );
    free( _map );
}

size_t hashmap_size( map_t map ) {
    hashmap_map *_map = (hashmap_map *) map;
    return _map->size;
}
