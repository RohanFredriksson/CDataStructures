#include <stdlib.h>
#include "hashmap.h"

#define NUM_ELEMENTS 500

int main() {

    // Initialise the map
    int flag = 0;
    HashMap* h = malloc(sizeof(HashMap));
    HashMap_Init(h, sizeof(int), sizeof(int));
    
    // Put a lot of elements in the map to test it.
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        int key = i;
        int value = i * i;
        HashMap_Put(h, &key, &value);
        if (HashMap_Size(h) != i+1) {flag = 1;}

        int k = 0;
        for (int j = 0; j < h->n; j++) {

            KeyValue* left = h->left + j;
            KeyValue* right = h->right + j;

            if (left->key != NULL) {k++;}
            if (right->key != NULL) {k++;}

        }
        //printf("k: %d\tsize: %d\n", k, HashMap_Size(h));

    }
    
    // Retrieve every element from the map.
    for (int i = NUM_ELEMENTS - 1; i >= 0; i--) {
        int buffer;
        HashMap_Get(h, &i, &buffer);
        if (buffer != i*i) {flag = 1;}
    }

    // Remove every even element key from the map.
    int k = 0;
    int n = HashMap_Size(h);
    for (int i = 0; i < n; i = i + 2) {
        int key = i;
        k++;
        if (HashMap_Remove(h, &key)) {flag = 1;}
    }
    if (HashMap_Size(h) != n-k) {flag = 1;}

    // Free the map memory
    HashMap_Free(h);
    free(h);
    return flag;
}