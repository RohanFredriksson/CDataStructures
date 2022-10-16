#include <stdint.h>
#include <stdbool.h>

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

// 
// 
// 
void HashMap_Init(HashMap* h, size_t key_size, size_t value_size);

//
// 
// 
int HashMap_Get(HashMap* h, void* key, void* buffer);

//
// 
// 
int HashMap_Remove(HashMap* h, void* key);

//
// 
// 
void HashMap_Put(HashMap* h, void* key, void* value);

//
// 
// 
void HashMap_Free(HashMap* h);

#endif