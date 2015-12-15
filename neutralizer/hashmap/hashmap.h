//
// Created by kdehairy on 11/15/15.
//


#ifndef DEXPATCHER_HASHMAP_H
#define DEXPATCHER_HASHMAP_H

#define MAP_FULL      -1
#define MAP_OMEM      -2
#define MAP_NOT_FOUND -3
#define MAP_OK         1

typedef void* map_t;

#ifdef __cplusplus
extern "C" {
#endif

map_t hashmap_new();

int hashmap_put( map_t map, uint32_t key, uint32_t value );

int hashmap_get( map_t map, uint32_t key, uint32_t *value );

void hashmap_free( map_t map );

size_t hashmap_size( map_t map );

#ifdef __cplusplus
}
#endif

#endif //DEXPATCHER_HASHMAP_H
