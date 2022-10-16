#include <stdint.h>

#ifndef HASHMAP_H
#define HASHMAP_H

struct KeyValue {
    void* key;
    void* value;
};
typedef struct KeyValue KeyValue;

struct HashMap {
    
    size_t key_size;
    size_t value_size;
    size_t size;
    size_t n;

    KeyValue* left;
    uint64_t left_seed_0;
    uint64_t left_seed_1;

    KeyValue* right;
    uint64_t right_seed_0;
    uint64_t right_seed_1;

};
typedef struct HashMap HashMap;

/*
Initialises the memory of a HashMap structure.

Inputs:
 - HashMap* h: the memory address of the HashMap structure.
 - size_t key_size: the size in bytes of the key datatype.
 - size_t value_size: the size in bytes of the value datatype.

Example:
 - This creates a map with float key and integer values.

    HashMap* h = malloc(sizeof(HashMap));
    HashMap_Init(h, sizeof(float), sizeof(int));

*/
void HashMap_Init(HashMap* h, size_t key_size, size_t value_size);

/*
Given a key, gets the associated value of the key in the HashMap.

Inputs:
 - HashMap* h: the memory address of the HashMap structure.
 - void* key: a memory address which contains data about the key.
 - void* buffer: a memory address where the value will be placed if found.

Outputs:
 - 0: if the object was successfully retrieved from the map.
 - 1: if the object could not be found in the map.

Example:
 - This gets the value stored at 1.0f in the map

    float key = 1.0f;
    int buffer;

    HashMap_Get(h, &key, &buffer);

*/
int HashMap_Get(HashMap* h, void* key, void* buffer);

/*
Given a key, removes the associated key/value pair in the HashMap.

Inputs:
 - HashMap* h: the memory address of the HashMap structure.
 - void* key: a memory address which contains data about the key.

Outputs:
 - 0: if the key/value pair was successfully removed from the map.
 - 1: if the key could not be found in the map.

Example:
 - This removes the key/value pair associated with 1.0f in the map

    float key = 1.0f;

    HashMap_Remove(h, &key);

*/
int HashMap_Remove(HashMap* h, void* key);

/*
Given a key/value pair, adds/updates the key/value pair in the HashMap.

Inputs:
 - HashMap* h: the memory address of the HashMap structure.
 - void* key: a memory address which contains data about the key.
 - void* value: a memory address which contains data about the value.

Example:
 - This adds a key/value pair to the map.

    float key = 1.0f;
    int value = 100;

    HashMap_Put(h, &key, &value);

*/
void HashMap_Put(HashMap* h, void* key, void* value);

/*
Frees all memory associated with an initialised HashMap structure.

Inputs:
 - HashMap* h: the memory address of the HashMap structure.

Example:
 - This frees all dynamically allocated memory.

    HashMap_Free(h);

*/
void HashMap_Free(HashMap* h);

#endif