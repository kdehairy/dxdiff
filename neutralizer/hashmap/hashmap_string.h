//
// Created by kdehairy on 11/16/15.
//

#ifndef DEXPATCHER_HASHMAP_STRING_H
#define DEXPATCHER_HASHMAP_STRING_H

#include <stddef.h>
#include <stdint.h>

#define MAP_FULL      -1
#define MAP_OMEM      -2
#define MAP_NOT_FOUND -3
#define MAP_COLLISION -4
#define MAP_OK         0

typedef void* map_string_t;

#ifdef __cplusplus
extern "C" {
#endif

map_string_t hashmap_string_new();

int hashmap_string_put( map_string_t map, const char *key, void *value );

int hashmap_string_get( map_string_t map, const char *key, void **value );

void hashmap_string_free( map_string_t map );

size_t hashmap_string_size( map_string_t map );

#ifdef __cplusplus
}
#endif


#endif //DEXPATCHER_HASHMAP_STRING_H
