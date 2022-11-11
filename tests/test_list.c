#include <stdlib.h>
#include "list.h"

#define NUM_ELEMENTS 500

int main() {

    // Initialise the map
    int flag = 0;
    List l;
    List_Init(&l, sizeof(int));
    int buffer;

    // Test pop and shift on an empty list
    if (List_Pop(&l, NULL) != 1) {flag = 1;}
    if (List_Shift(&l, NULL) != 1) {flag = 1;}
    
    // Put a lot of elements in the map to test it.
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        int element = i*i;
        List_Push(&l,&element);
        if (List_Length(&l) != i+1) {flag = 1;}
    }
    if (List_Length(&l) != NUM_ELEMENTS) {flag = 1;}

    // Get all fibonacci indices
    int last_k = 0;
    int k = 1;
    while (k < NUM_ELEMENTS) {
        
        List_Get(&l, k, &buffer);
        if (buffer != k*k) {flag = 1;}
        
        int tmp = k;
        k = k + last_k;
        last_k = tmp;
    }

    // Test get with a null buffer
    if (List_Get(&l, 5, NULL) != 2) {flag = 1;}

    // Test get with invalid bounds and buffers
    if (List_Get(&l, -4, NULL) != 1) {flag = 1;}
    if (List_Get(&l, 10000, NULL) != 1) {flag = 1;}
    if (List_Get(&l, -4, &buffer) != 1) {flag = 1;}
    if (List_Get(&l, 10000, &buffer) != 1) {flag = 1;}

    // Pop all numbers off the list, should get all elements in reverse order
    for (int i = NUM_ELEMENTS-1; i >= 0; i--) {
        if (List_Pop(&l, &buffer) != 0) {flag = 1;}
        if (buffer != i*i) {flag = 1;}
        if (List_Length(&l) != i) {flag = 1;}
    }

    // Test if list is empty
    if (List_Length(&l) != 0) {flag = 1;}
    if (List_Pop(&l, NULL) != 1) {flag = 1;}
    if (List_Shift(&l, NULL) != 1) {flag = 1;}

    // Put a lot of elements in the map to test it.
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        int element = i*i;
        List_Push(&l,&element);
        if (List_Length(&l) != i+1) {flag = 1;}
    }
    if (List_Length(&l) != NUM_ELEMENTS) {flag = 1;}

    // Unshift half of the list
    for (int i = 0; i < NUM_ELEMENTS / 2; i++) {
        List_Shift(&l, &buffer);
        if (buffer != i*i) {flag = 1;}
    }
    if (List_Length(&l) != NUM_ELEMENTS - (NUM_ELEMENTS / 2)) {flag = 1;}
    
    // Clear the rest of the list
    List_Clear(&l);

    // Test if list is empty
    if (List_Length(&l) != 0) {flag = 1;}
    if (List_Pop(&l, NULL) != 1) {flag = 1;}
    if (List_Shift(&l, NULL) != 1) {flag = 1;}

    // Test the add method
    int element = 9;
    if (List_Add(&l, -1, &element) != 1) {flag = 1;}
    if (List_Add(&l, 1, &element) != 1) {flag = 1;}
    if (List_Add(&l, 0, &element) != 0) {flag = 1;}
    // [9]

    element = 78;
    if (List_Add(&l, -1, &element) != 1) {flag = 1;}
    if (List_Add(&l, 0, &element) != 0) {flag = 1;}
    if (List_Add(&l, 5, &element) != 1) {flag = 1;}
    // [78, 9]

    element = 111;
    if (List_Add(&l, 2, &element) != 0) {flag = 1;}
    // [78, 9, 111]

    element = 67;
    if (List_Add(&l, 1, &element) != 0) {flag = 1;}
    // [78, 67, 9, 111]

    // Test the contents of the list
    List_Get(&l, 0, &buffer);
    if (buffer != 78) {flag = 1;}
    List_Get(&l, 1, &buffer);
    if (buffer != 67) {flag = 1;}
    List_Get(&l, 2, &buffer);
    if (buffer != 9) {flag = 1;}
    List_Get(&l, 3, &buffer);
    if (buffer != 111) {flag = 1;}

    // Test the remove method
    if (List_Remove(&l, 1) != 0) {flag = 1;}
    // [78, 9, 111]

    if (List_Remove(&l, 2) != 0) {flag = 1;}
    if (List_Remove(&l, 265) != 1) {flag = 1;}
    if (List_Remove(&l, -1) != 1) {flag = 1;}
    // [78, 9]

    if (List_Remove(&l, 0) != 0) {flag = 1;}
    // [9]

    if (List_Remove(&l, 0) != 0) {flag = 1;}
    // [0]

    if (List_Remove(&l, 0) != 1) {flag = 1;}
    if (List_Remove(&l, 2) != 1) {flag = 1;}
    if (List_Remove(&l, -100) != 1) {flag = 1;}

    List_Free(&l);
    return flag;
}