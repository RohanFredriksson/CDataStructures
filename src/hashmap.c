#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "hash.h"
#include "hashmap.h"

#define HASHMAP_INITIAL_N 16

void _HashMap_Put(HashMap* h, void* key, void* value, bool allocate);

uint64_t _HashMap_Random(void) {
    uint64_t r = 0;
    for (int i = 0; i < 64; i += 15) {
        r = r * ((uint64_t) RAND_MAX + 1) + rand();
    }
    return r;
}

uint64_t _HashMap_Hash(const void *key, size_t key_size, uint64_t seed0, uint64_t seed1) {
    if (key_size == 0) {return OAAT(key);}
    return SIP64((uint8_t*)key, key_size, seed0, seed1);
}

void _HashMap_KeyCopy(HashMap* h, void* destination, void* key) {
    if (h->key_size == 0) {memcpy(destination, key, strlen(key) + 1);}
    else {memcpy(destination, key, h->key_size);}
}

void _HashMap_ValueCopy(HashMap* h, void* destination, void* value) {
    if (h->value_size == 0) {memcpy(destination, value, strlen(value) + 1);}
    else {memcpy(destination, value, h->value_size);}
}

void* _HashMap_KeyMalloc(HashMap* h, void* key) {
    if (h->key_size == 0) {return malloc(strlen(key) + 1);}
    return malloc(h->key_size);
}

void* _HashMap_ValueMalloc(HashMap* h, void* value) {
    if (h->value_size == 0) {return malloc(strlen(value) + 1);}
    return malloc(h->value_size);
}

void _HashMap_Init(HashMap* h, size_t n, size_t key_size, size_t value_size) {

    h->key_size = key_size;
    h->value_size = value_size;
    h->size = 0;
    h->n = n;

    h->array = malloc(2 * n * sizeof(KeyValue));
    h->left_seed_0 = _HashMap_Random();
    h->left_seed_1 = _HashMap_Random();
    h->right_seed_0 = _HashMap_Random();
    h->right_seed_1 = _HashMap_Random();

    for (int i = 0; i < 2 * n; i++) {
        h->array[i].key = NULL;
        h->array[i].value = NULL;
    }

}

void HashMap_Init(HashMap* h, size_t key_size, size_t value_size) {
    _HashMap_Init(h, HASHMAP_INITIAL_N, key_size, value_size);
}

int HashMap_Size(HashMap* h) {
    return h->size;
}

int HashMap_Get(HashMap* h, void* key, void* buffer) {

    // If the buffer is null, we cannot write to it, return 1.
    if (buffer == NULL) {return 2;}

    KeyValue* pair;
    int computed_hash;

    // Compute left hash
    computed_hash = _HashMap_Hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    pair = h->array + computed_hash;
    
    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        _HashMap_ValueCopy(h, buffer, pair->value);
        return 0;
    }

    // Compute right hash
    computed_hash = _HashMap_Hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    pair = h->array + h->n + computed_hash;

    // If key is in the right table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        _HashMap_ValueCopy(h, buffer, pair->value);
        return 0;
    }

    // Key was not found
    return 1;

}

void HashMap_Grow(HashMap* h) {

    HashMap new_h;
    _HashMap_Init(&new_h, 2 * h->n, h->key_size, h->value_size);

    // Move all key value pairs from the old map to the new one.
    for (int i = 0; i < 2 * h->n; i++) {
        KeyValue* current = h->array + i;
        if (current->key != NULL) {_HashMap_Put(&new_h, current->key, current->value, 0);}
    }

    // Free all required memory from the old hashmap
    free(h->array);

    // Move the new hashmap into the location of the old one.
    memcpy(h, &new_h, sizeof(HashMap));

}

int HashMap_Remove(HashMap* h, void* key) {

    KeyValue* pair;
    int computed_hash;

    // Compute left hash
    computed_hash = _HashMap_Hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    pair = h->array + computed_hash;
    
    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        
        // Free the memory allocated to store the key and value
        free(pair->key);
        free(pair->value);

        // Set the pointers to null
        pair->key = NULL;
        pair->value = NULL;

        // Reduce size, and return
        h->size--;
        return 0;

    }

    // Compute right hash
    computed_hash = _HashMap_Hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    pair = h->array + h->n + computed_hash;

    // If key is in the right table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        
        // Free the memory allocated to store the key and value
        free(pair->key);
        free(pair->value);

        // Set the pointers to null
        pair->key = NULL;
        pair->value = NULL;

        // Reduce size, and return
        h->size--;
        return 0;

    }

    // Key was not found
    return 1;

}

bool _HashMap_TryPut(HashMap* h, KeyValue* pair, void* key, void* value, bool allocate) {

    // If pair is empty.
    if (pair->key == NULL) {
        
        if (allocate) {
            pair->key = _HashMap_KeyMalloc(h, key);
            pair->value = _HashMap_ValueMalloc(h, value);
            _HashMap_KeyCopy(h, pair->key, key);
            _HashMap_ValueCopy(h, pair->value, value);
        } else {
            pair->key = key;
            pair->value = value;
        }
        
        // Increment the size and return
        h->size++;
        return 1;
    }

    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) { 
        
        if (allocate) {
            pair->value = _HashMap_ValueMalloc(h, value);
            _HashMap_ValueCopy(h, pair->value, value);
        } else {
            pair->value = value;
        }
        
        return 1;
    }

    return 0;
}

bool _HashMap_Evict(HashMap* h, KeyValue* displaced, KeyValue* target, KeyValue* refuge, bool allocate) {
    
    void* tmp_key; 
    void* tmp_value; 
    bool exit_flag = 0;
    
    if (refuge->key == NULL) {
        
        if (allocate) {
            tmp_key = _HashMap_KeyMalloc(h, displaced->key);
            tmp_value = _HashMap_ValueMalloc(h, displaced->value);
            _HashMap_KeyCopy(h, tmp_key, displaced->key);
            _HashMap_ValueCopy(h, tmp_value, displaced->value);
            displaced->key = tmp_key;
            displaced->value = tmp_value;
        }

        exit_flag = 1;
    }

    // If we cant exit, store the refuge pair in a temporary buffer
    if (exit_flag == 0) {
        tmp_key = refuge->key;
        tmp_value = refuge->value;
    }

    // Move the target into the refuge
    refuge->key = target->key;
    refuge->value = target->value;

    // Move the displaced into the target
    target->key = displaced->key;
    target->value = displaced->value;

    // We have found a successful home, for everybody :)
    if (exit_flag == 1) {
        h->size++;
        return 1;
    }

    // Refuge pair is now homeless :(
    displaced->key = tmp_key;
    displaced->value = tmp_value;
    return 0;
}

void _HashMap_Put(HashMap* h, void* key, void* value, bool allocate) {

    KeyValue* left_pair;
    KeyValue* right_pair;
    int left_hash;
    int right_hash;

    // If the load factor exceeds 0.5, rebuild the table to improve performance
    if (h->size > h->n / 2) {HashMap_Grow(h);}

    // Try and put the key, value pair in the left spot.
    left_hash = _HashMap_Hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    left_pair = h->array + left_hash;
    if (_HashMap_TryPut(h, left_pair, key, value, allocate)) {return;}

    // Try and put the key, value pair in the right spot.
    right_hash = _HashMap_Hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    right_pair = h->array + h->n + right_hash;
    if (_HashMap_TryPut(h, right_pair, key, value, allocate)) {return;}

    // Start an eviction sequence
    KeyValue new_pair = {key, value};
    KeyValue* displaced_pair = &new_pair;

    for (int i = 0; i < 2*h->n; i++) {

        // If i is even, try and place it in the left table.
        if (i % 2 == 0) {

            // Compute right hash of the left pair
            right_hash = _HashMap_Hash(left_pair->key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
            right_pair = h->array + h->n + right_hash;
            
            // Evict the left pair. If it is resolved, then return.
            if (_HashMap_Evict(h, displaced_pair, left_pair, right_pair, allocate)) {return;}

        }

        // Try and place it in the right table.
        else {

            // Compute left hash of the right pair
            left_hash = _HashMap_Hash(right_pair->key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
            left_pair = h->array + left_hash;
            
            // Evict the right pair. If it is resolved, then return.
            if (_HashMap_Evict(h, displaced_pair, right_pair, left_pair, allocate)) {return;}

        }

    }   

    // If the eviction sequence is greater than 2n, then there is a cycle.
    // We must rebuild the entire hash table.
    HashMap_Grow(h);

    // Move the displaced pair into the new one.
    _HashMap_Put(h, key, value, allocate);

}

void HashMap_Put(HashMap* h, void* key, void* value) {
    _HashMap_Put(h, key, value, 1);
}

void HashMap_Clear(HashMap* h) {
    size_t key_size = h->key_size;
    size_t value_size = h->value_size;
    HashMap_Free(h);
    HashMap_Init(h, key_size, value_size);
}

void HashMap_Free(HashMap* h) {

    KeyValue* pair;
    for (int i = 0; i < 2 * h->n; i++) {
        
        pair = h->array + i;
        if (pair->key != NULL) {free(pair->key);}
        if (pair->value != NULL) {free(pair->value);}

    }

    free(h->array);
}