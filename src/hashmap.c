#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "siphash.h"
#include "hashmap.h"

#define HASHMAP_INITIAL_N 16

uint64_t rand_uint64(void) {
    uint64_t r = 0;
    for (int i = 0; i < 64; i += 15) {
        r = r * ((uint64_t) RAND_MAX + 1) + rand();
    }
    return r;
}

uint64_t hash(const void *data, size_t len, uint64_t seed0, uint64_t seed1) {
    return SIP64((uint8_t*)data, len, seed0, seed1);
}

void _HashMap_Init(HashMap* h, size_t n, size_t key_size, size_t value_size) {

    h->key_size = key_size;
    h->value_size = value_size;
    h->size = 0;
    h->n = n;

    h->array = malloc(2 * n * sizeof(KeyValue));
    h->left_seed_0 = rand_uint64();
    h->left_seed_1 = rand_uint64();
    h->right_seed_0 = rand_uint64();
    h->right_seed_1 = rand_uint64();

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
    computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    pair = h->array + computed_hash;
    
    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        // Store the value in the buffer, and return
        memcpy(buffer, pair->value, h->value_size);
        return 0;
    }

    // Compute right hash
    computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    pair = h->array + h->n + computed_hash;

    // If key is in the right table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        // Store the value in the buffer, and return
        memcpy(buffer, pair->value, h->value_size);
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
        if (current->key != NULL) {_HashMap_Put(&new_h, current->key, current->value);}
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
    computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
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
    computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
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

void _HashMap_Put(HashMap* h, void* key, void* value) {

    // If the load factor exceeds 0.5, rebuild the table to improve performance
    if (h->size > h->n / 2) {
        HashMap_Grow(h);
    }

    KeyValue* pair;
    int computed_hash;

    // Compute left hash
    computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    pair = h->array + computed_hash;

    // If spot in left table is empty
    if (pair->key == NULL) {
        
        // Set the key, value data in the spot
        pair->key = key;
        pair->value = value;
        
        // Increment the size and return
        h->size++;
        return;

    }

    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) { 
        
        // Set the value data into the space, and return
        pair->value = value;
        return;

    }

    // Compute right hash
    computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    pair = h->array + h->n + computed_hash;

    // If spot in right table is empty
    if (pair->key == NULL) {
        
        // Set the key, value data in the spot
        pair->key = key;
        pair->value = value;
        
        // Increment the size and return
        h->size++;
        return;

    }

    // If key is in the right table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        
        // Set the value data into the space, and return
        pair->value = value;
        return;

    }

    KeyValue new_pair = {key, value};
    KeyValue* displaced_pair = &new_pair;

    // Start an eviction sequence
    for (int i = 0; i < 2*h->n; i++) {

        // If i is even, try and place it in the left table.
        if (i % 2 == 0) {

            // Compute left hash
            computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
            KeyValue* left_pair = h->array + computed_hash;

            // Compute right hash of the left pair
            computed_hash = hash(left_pair->key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
            KeyValue* right_pair = h->array + h->n + computed_hash;
            
            bool exit_flag = 0;
            if (right_pair->key == NULL) {
                exit_flag = 1;
            }

            // If we cant exit, store the right pair in a temporary buffer
            void* tmp_key; 
            void* tmp_value; 
            if (exit_flag == 0) {
                tmp_key = right_pair->key;
                tmp_value = right_pair->value;
            }

            // Move the left pair into the right pair
            right_pair->key = left_pair->key;
            right_pair->value = left_pair->value;

            // Move the displaced pair into the left pair
            left_pair->key = displaced_pair->key;
            left_pair->value = displaced_pair->value;

            // We have found a successful home, for everybody :)
            if (exit_flag == 1) {

                // Increment the size, free memory and return
                h->size++;
                return;

            }

            // Right pair is now homeless :(
            displaced_pair->key = tmp_key;
            displaced_pair->value = tmp_value;

        }

        // Try and place it in the right table.
        else {

            // Compute right hash
            computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
            KeyValue* right_pair = h->array + h->n + computed_hash;

            // Compute left hash of the right pair
            computed_hash = hash(right_pair->key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
            KeyValue* left_pair = h->array + computed_hash;
            
            bool exit_flag = 0;
            if (right_pair->key == NULL) {
                exit_flag = 1;
            }

            // If we cant exit, store the left pair in a temporary buffer
            void* tmp_key; 
            void* tmp_value; 
            if (exit_flag == 0) {
                tmp_key = left_pair->key;
                tmp_value = left_pair->value;
            }

            // Move the right pair into the left pair
            left_pair->key = right_pair->key;
            left_pair->value = right_pair->value;

            // Move the displaced pair into the right pair
            right_pair->key = displaced_pair->key;
            right_pair->value = displaced_pair->value;

            // We have found a successful home, for everybody :)
            if (exit_flag == 1) {

                // Increment the size and return
                h->size++;
                return;

            }

            // Left pair is now homeless :(
            displaced_pair->key = tmp_key;
            displaced_pair->value = tmp_value;

        }

    }   

    // If the eviction sequence is greater than 2n, then there is a cycle.
    // We must rebuild the entire hash table.
    HashMap_Grow(h);

    // Move the displaced pair into the new one.
    _HashMap_Put(h, key, value);

}

bool _HashMap_TryPut(HashMap* h, KeyValue* pair, void* key, void* value) {

    // If pair is empty.
    if (pair->key == NULL) {
        
        // Allocate memory to hold the key, value pair
        pair->key = malloc(h->key_size);
        pair->value = malloc(h->value_size);
        
        // Copy the key, value pair into the spot
        memcpy(pair->key, key, h->key_size);
        memcpy(pair->value, value, h->value_size);
        
        // Increment the size and return
        h->size++;
        return 1;

    }

    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) { 
        
        // If there is no memory allocated at this key, allocate some
        if (pair->value == NULL) {
            pair->value = malloc(h->value_size);
        }

        // Copy the value data into the allocated space, and return
        memcpy(pair->value, value, h->value_size);
        return 1;

    }

    return 0;
}

bool _HashMap_Evict(HashMap* h, KeyValue* displaced, KeyValue* target, KeyValue* refuge) {
    
    bool exit_flag = 0;
    if (refuge->key == NULL) {

        // Allocate some memory to copy the target into refuge
        refuge->key = malloc(h->key_size);
        refuge->value = malloc(h->value_size);

        // After the copies, we can return.
        exit_flag = 1;

    }

    // If we cant exit, store the refuge pair in a temporary buffer
    void* tmp_key; 
    void* tmp_value; 
    if (exit_flag == 0) {
        tmp_key = refuge->key;
        tmp_value = refuge->value;
    }

    // Move the target into the refuge
    memcpy(refuge->key, target->key, h->key_size);
    memcpy(refuge->value, target->value, h->value_size);

    // Move the displaced into the target
    memcpy(target->key, displaced->key, h->key_size);
    memcpy(target->value, displaced->value, h->value_size);

    // We have found a successful home, for everybody :)
    if (exit_flag == 1) {
        h->size++;
        return 1;
    }

    // Refuge pair is now homeless :(
    memcpy(displaced->key, tmp_key, h->key_size);
    memcpy(displaced->value, tmp_value, h->value_size);
    return 0;
}

void HashMap_Put(HashMap* h, void* key, void* value) {

    KeyValue* left_pair;
    KeyValue* right_pair;
    int left_hash;
    int right_hash;

    // If the load factor exceeds 0.5, rebuild the table to improve performance
    if (h->size > h->n / 2) {HashMap_Grow(h);}

    // Try and put the key, value pair in the left spot.
    left_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    left_pair = h->array + left_hash;
    if (_HashMap_TryPut(h, left_pair, key, value)) {return;}

    // Try and put the key, value pair in the right spot.
    right_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    right_pair = h->array + h->n + right_hash;
    if (_HashMap_TryPut(h, right_pair, key, value)) {return;}

    // Start an eviction sequence
    KeyValue new_pair = {key, value};
    KeyValue* displaced_pair = &new_pair;

    for (int i = 0; i < 2*h->n; i++) {

        // If i is even, try and place it in the left table.
        if (i % 2 == 0) {

            // Compute right hash of the left pair
            right_hash = hash(left_pair->key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
            right_pair = h->array + h->n + right_hash;
            
            // Evict the left pair. If it is resolved, then return.
            if (_HashMap_Evict(h, displaced_pair, left_pair, right_pair)) {return;}

        }

        // Try and place it in the right table.
        else {

            // Compute left hash of the right pair
            left_hash = hash(right_pair->key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
            left_pair = h->array + left_hash;
            
            // Evict the right pair. If it is resolved, then return.
            if (_HashMap_Evict(h, displaced_pair, right_pair, left_pair)) {return;}

        }

    }   

    // If the eviction sequence is greater than 2n, then there is a cycle.
    // We must rebuild the entire hash table.
    HashMap_Grow(h);

    // Move the displaced pair into the new one.
    HashMap_Put(h, key, value);

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