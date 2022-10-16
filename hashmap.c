#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "hashmap.h"

#include <stdio.h>

//-----------------------------------------------------------------------------
// SipHash reference C implementation
//
// Copyright (c) 2012-2016 Jean-Philippe Aumasson
// <jeanphilippe.aumasson@gmail.com>
// Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>
//
// To the extent possible under law, the author(s) have dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along
// with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
//
// default: SipHash-2-4
//-----------------------------------------------------------------------------
static uint64_t SIP64(const uint8_t *in, const size_t inlen, uint64_t seed0, uint64_t seed1) {
#define U8TO64_LE(p) \
    {  (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) | \
        ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) | \
        ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) | \
        ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56)) }
#define U64TO8_LE(p, v) \
    { U32TO8_LE((p), (uint32_t)((v))); \
      U32TO8_LE((p) + 4, (uint32_t)((v) >> 32)); }
#define U32TO8_LE(p, v) \
    { (p)[0] = (uint8_t)((v)); \
      (p)[1] = (uint8_t)((v) >> 8); \
      (p)[2] = (uint8_t)((v) >> 16); \
      (p)[3] = (uint8_t)((v) >> 24); }
#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))
#define SIPROUND \
    { v0 += v1; v1 = ROTL(v1, 13); \
      v1 ^= v0; v0 = ROTL(v0, 32); \
      v2 += v3; v3 = ROTL(v3, 16); \
      v3 ^= v2; \
      v0 += v3; v3 = ROTL(v3, 21); \
      v3 ^= v0; \
      v2 += v1; v1 = ROTL(v1, 17); \
      v1 ^= v2; v2 = ROTL(v2, 32); }
    uint64_t k0 = U8TO64_LE((uint8_t*)&seed0);
    uint64_t k1 = U8TO64_LE((uint8_t*)&seed1);
    uint64_t v3 = UINT64_C(0x7465646279746573) ^ k1;
    uint64_t v2 = UINT64_C(0x6c7967656e657261) ^ k0;
    uint64_t v1 = UINT64_C(0x646f72616e646f6d) ^ k1;
    uint64_t v0 = UINT64_C(0x736f6d6570736575) ^ k0;
    const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
    for (; in != end; in += 8) {
        uint64_t m = U8TO64_LE(in);
        v3 ^= m;
        SIPROUND; SIPROUND;
        v0 ^= m;
    }
    const int left = inlen & 7;
    uint64_t b = ((uint64_t)inlen) << 56;
    switch (left) {
    case 7: b |= ((uint64_t)in[6]) << 48;
    case 6: b |= ((uint64_t)in[5]) << 40;
    case 5: b |= ((uint64_t)in[4]) << 32;
    case 4: b |= ((uint64_t)in[3]) << 24;
    case 3: b |= ((uint64_t)in[2]) << 16;
    case 2: b |= ((uint64_t)in[1]) << 8;
    case 1: b |= ((uint64_t)in[0]); break;
    case 0: break;
    }
    v3 ^= b;
    SIPROUND; SIPROUND;
    v0 ^= b;
    v2 ^= 0xff;
    SIPROUND; SIPROUND; SIPROUND; SIPROUND;
    b = v0 ^ v1 ^ v2 ^ v3;
    uint64_t out = 0;
    U64TO8_LE((uint8_t*)&out, b);
    return out;
}

//-----------------------------------------------------------------------------

#define HASHMAP_INITIAL_N 3

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

    h->left = malloc(n * sizeof(KeyValue));
    h->left_seed_0 = rand_uint64();
    h->left_seed_1 = rand_uint64();

    h->right = malloc(n * sizeof(KeyValue));
    h->right_seed_0 = rand_uint64();
    h->right_seed_1 = rand_uint64();

    for (int i = 0; i < n; i++) {
        h->left[i].key = NULL;
        h->left[i].value = NULL;
        h->right[i].key = NULL;
        h->right[i].value = NULL;
    }

}

void HashMap_Init(HashMap* h, size_t key_size, size_t value_size) {
    _HashMap_Init(h, HASHMAP_INITIAL_N, key_size, value_size);
}

int HashMap_Get(HashMap* h, void* key, void* buffer) {

    KeyValue* pair;
    int computed_hash;

    // Compute left hash
    computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    pair = h->left + computed_hash;
    
    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        // Store the value in the buffer, and return
        memcpy(buffer, pair->value, h->value_size);
        return 0;
    }

    // Compute right hash
    computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    pair = h->right + computed_hash;

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
    for (int i = 0; i < h->n; i++) {

        KeyValue* left = h->left + i;
        KeyValue* right = h->right + i;

        if (left->key != NULL) {HashMap_Put(&new_h, left->key, left->value);}
        if (right->key != NULL) {HashMap_Put(&new_h, right->key, right->value);}

    }

    // Destroy the old hashmap
    HashMap_Free(h);

    // Move the new hashmap into the location of the old one.
    memcpy(h, &new_h, sizeof(HashMap));

}

int HashMap_Remove(HashMap* h, void* key) {

    KeyValue* pair;
    int computed_hash;

    // Compute left hash
    computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    
    pair = h->left + computed_hash;
    
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
    pair = h->right + computed_hash;

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

void HashMap_Put(HashMap* h, void* key, void* value) {

    // If the load factor exceeds 0.5, rebuild the table to improve performance
    if (h->size > h->n / 2) {
        HashMap_Grow(h);
    }

    KeyValue* pair;
    int computed_hash;

    // Compute left hash
    computed_hash = hash(key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
    pair = h->left + computed_hash;

    // If spot in left table is empty
    if (pair->key == NULL) {
        
        // Allocate memory to hold the key pair
        pair->key = malloc(h->key_size);
        pair->value = malloc(h->value_size);
        
        // Copy the key, value data into the spot
        memcpy(pair->key, key, h->key_size);
        memcpy(pair->value, value, h->value_size);
        
        // Increment the size and return
        h->size++;
        return;

    }

    // If key is in the left table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) { 
        
        // If there is no memory allocated at this key, allocate some
        if (pair->value == NULL) {
            pair->value = malloc(h->value_size);
        }

        // Copy the value data into the allocated space, and return
        memcpy(pair->value, value, h->value_size);
        
        // Increment the size and return
        h->size++;
        return;

    }

    // Compute right hash
    computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
    pair = h->right + computed_hash;

    // If key is in the right table
    if (pair->key != NULL && memcmp(key, pair->key, h->key_size) == 0) {
        
        // If there is no memory allocated at this key, allocate some.
        if (pair->value == NULL) {
            pair->value = malloc(h->value_size);
        }

        // Copy the value data into the allocated space, and return
        memcpy(pair->value, value, h->value_size);
        
        // Increment the size and return
        h->size++;
        return;

    }

    // If spot in right table is empty
    if (pair->key == NULL) {
        
        // Allocate memory to hold the key pair
        pair->key = malloc(h->key_size);
        pair->value = malloc(h->value_size);
        
        // Copy the key, value data into the spot
        memcpy(pair->key, key, h->key_size);
        memcpy(pair->value, value, h->value_size);
        
        // Increment the size and return
        h->size++;
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
            KeyValue* left_pair = h->left + computed_hash;

            // Compute right hash of the left pair
            computed_hash = hash(pair->key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
            KeyValue* right_pair = h->right + computed_hash;
            
            bool exit_flag = 0;
            if (right_pair->key == NULL) {

                // Allocate some memory to copy the left pair into the right
                right_pair->key = malloc(h->key_size);
                right_pair->value = malloc(h->value_size);

                // After the copies, we can return.
                exit_flag = 1;

            }

            // Move the left pair into the right pair
            memcpy(right_pair->key, left_pair->key, h->key_size);
            memcpy(right_pair->value, left_pair->value, h->value_size);

            // Move the displaced pair into the left pair
            memcpy(left_pair->key, displaced_pair->key, h->key_size);
            memcpy(left_pair->value, displaced_pair->value, h->value_size);

            // We have found a successful home, for everybody :)
            if (exit_flag == 1) {
                
                // Increment the size and return
                h->size++;
                return;
            }

            // Left pair is now homeless :(
            displaced_pair = left_pair;

        }

        // Try and place it in the right table.
        else {

            // Compute right hash
            computed_hash = hash(key, h->key_size, h->right_seed_0, h->right_seed_1) % h->n;
            KeyValue* right_pair = h->right + computed_hash;

            // Compute left hash of the right pair
            computed_hash = hash(pair->key, h->key_size, h->left_seed_0, h->left_seed_1) % h->n;
            KeyValue* left_pair = h->left + computed_hash;
            
            bool exit_flag = 0;
            if (right_pair->key == NULL) {

                // Allocate some memory to copy the left pair into the right
                right_pair->key = malloc(h->key_size);
                right_pair->value = malloc(h->value_size);
                
                // After the copies, we can return.
                exit_flag = 1;

            }

            // Move the right pair into the left pair
            memcpy(left_pair->key, right_pair->key, h->key_size);
            memcpy(left_pair->value, right_pair->value, h->value_size);

            // Move the displaced pair into the right pair
            memcpy(right_pair->key, displaced_pair->key, h->key_size);
            memcpy(right_pair->value, displaced_pair->value, h->value_size);

            // We have found a successful home, for everybody :)
            if (exit_flag == 1) {

                // Increment the size and return
                h->size++;
                return;
            }

            // Right pair is now homeless :(
            displaced_pair = right_pair;

        }

    }   

    // If the eviction sequence is greater than 2n, then there is a cycle.
    // We must rebuild the entire hash table.
    HashMap_Grow(h);

    // Move the displaced pair into the new one.
    HashMap_Put(h, key, value);

}

void HashMap_Free(HashMap* h) {

    KeyValue* pair;
    for (int i = 0; i < h->n; i++) {
        
        pair = h->left + i;
        if (pair->key != NULL) {free(pair->key);}
        if (pair->value != NULL) {free(pair->value);}

        pair = h->right + i;
        if (pair->key != NULL) {free(pair->key);}
        if (pair->value != NULL) {free(pair->value);}

    }

    free(h->left);
    free(h->right);
}

int main() {

    HashMap* h = malloc(sizeof(HashMap));
    HashMap_Init(h, sizeof(int), sizeof(int));
    
    int k1 = 1;
    int v1 = 100;

    int k2 = 2;
    int v2 = 200;

    int k3 = 3;
    int v3 = 300;

    int k4 = 4;
    int v4 = 400;

    int k5 = 5;
    int v5 = 500;

    int k6 = 6;
    int v6 = 600;

    int k7 = 7;
    int v7 = 700;

    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k1, &v1);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k2, &v2);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k3, &v3);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k4, &v4);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k5, &v5);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k6, &v6);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Put(h, &k7, &v7);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    
    int buffer;
    HashMap_Get(h, &k1, &buffer);
    printf("%d\n", buffer);
    HashMap_Get(h, &k2, &buffer);
    printf("%d\n", buffer);
    HashMap_Get(h, &k3, &buffer);
    printf("%d\n", buffer);
    HashMap_Get(h, &k4, &buffer);
    printf("%d\n", buffer);
    HashMap_Get(h, &k5, &buffer);
    printf("%d\n", buffer);
    HashMap_Get(h, &k6, &buffer);
    printf("%d\n", buffer);
    HashMap_Get(h, &k7, &buffer);
    printf("%d\n", buffer);

    HashMap_Remove(h, &k4);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Remove(h, &k7);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Remove(h, &k6);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Remove(h, &k1);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Remove(h, &k2);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Remove(h, &k5);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);
    HashMap_Remove(h, &k3);
    printf("SIZE: %ld N: %ld\n", h->size, h->n);

    HashMap_Free(h);
    free(h);

    return 0;
}