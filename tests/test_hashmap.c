#include <stdlib.h>
#include "hashmap.h"

#define NUM_ELEMENTS 500

int main() {

    // Initialise the map
    int flag = 0;
    HashMap h;
    HashMap_Init(&h, sizeof(int), sizeof(int));
    
    // Put a lot of elements in the map to test it.
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        int key = i;
        int value = i * i;
        HashMap_Put(&h, &key, &value);
        if (HashMap_Size(&h) != i+1) {flag = 1;}
    }
    
    // Retrieve every element from the map.
    for (int i = NUM_ELEMENTS - 1; i >= 0; i--) {
        int buffer;
        HashMap_Get(&h, &i, &buffer);
        if (buffer != i*i) {flag = 1;}
    }

    // Remove every even key from the map.
    int k = 0;
    int n = HashMap_Size(&h);
    for (int i = 0; i < n; i = i + 2) {
        int key = i;
        k++;
        if (HashMap_Remove(&h, &key) != 1) {flag = 1;}
    }
    if (HashMap_Size(&h) != n-k) {flag = 1;}

    // Attempt to get every even key from the map.
    for (int i = 0; i < n; i = i + 2) {
        int key = i;
        int buffer;
        if (HashMap_Get(&h, &key, &buffer) != 0) {flag = 1;}
    }

    // Attempt to get every odd key from the map.
    for (int i = 1; i < n; i = i + 2) {
        int key = i;
        int buffer;
        if (HashMap_Get(&h, &key, &buffer) != 1) {flag = 1;}
        if (buffer != i*i) {flag = 1;}
    }

    // Get a key not in the map
    int key = 1010101;
    int buffer;
    if (HashMap_Get(&h, &key, &buffer) != 0) {flag = 1;}

    key = 1;
    if (HashMap_Get(&h, &key, NULL) != 0) {flag = 1;}

    // Free the map memory
    HashMap_Free(&h);
    return flag;
}