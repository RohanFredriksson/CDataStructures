#include <stdbool.h>

#ifndef LIST_H
#define LIST_H

struct List {

    void** elements;
    size_t element_size;
    int size;
    int length;

};
typedef struct List List;

/*
Initialises the memory of a List structure.

Inputs:
 - List* l: the memory address of the List structure.
 - size_t element_size: the size in bytes of the element datatype being stored.

Time Complexity: O(1)

Example:
 - This creates a list which stores floats

    List* l = malloc(sizeof(List));
    List_Init(l, sizeof(float));

*/
void List_Init(List* l, size_t element_size);

/*
Returns the number of elements that are stored in the List.

Inputs:
 - List* l: the memory address of the List structure.

Outputs:
 - int: the number of elements that are current stored in the list

Time Complexity: O(1)

Example:
 - This gets the length of the list
 
    int length = List_Length(l);

*/
int List_Length(List* l);

/*
Returns the address of the array of pointers where each element is stored.

Inputs:
 - List* l: the memory address of the List structure.

Outputs:
 - void**: the address of the array of pointers where each element is stored.

Time Complexity: O(1)

Example:
 - This gets all elements from the list.
 
    (int**) elements = List_Elements(l);

*/
void** List_Elements(List* l);

/*
Given an index, gets the element at the index in the List.

Inputs:
 - List* l: the memory address of the List structure.
 - int index: the index of the item in the list.
 - void* buffer: a memory address where the element will be stored if possible.

Outputs:
 - 0: if the index supplied is out of bounds.
 - 1: if the index supplied is in the bounds.

Time Complexity: O(1)

Example:
 - This gets the value stored at index 3 in the list

    float buffer;
    List_Get(l, 3, &buffer);

*/
bool List_Get(List* l, int index, void* buffer);

/*
Stores the last element in the list in the buffer, then removes the last element from the list.

Inputs:
 - List* l: the memory address of the List structure.
 - void* buffer: a memory address where the element will be stored if possible.

Outputs:
 - 0: if the value could not be returned.
 - 1: if the value was returned.

Time Complexity: O(1)

Example:
 - This pops the last value off the list

    float buffer;
    List_Pop(l, &buffer);

*/
bool List_Pop(List* l, void* buffer);

/*
Stores the first element in the list in the buffer, then removes the first element from the list.

Inputs:
 - List* l: the memory address of the List structure.
 - void* buffer: a memory address where the element will be stored if possible.

Outputs:
 - 0: if the value could not be returned.
 - 1: if the value was returned.

Time Complexity: O(n)

Example:
 - This shifts the first value off the list

    float buffer;
    List_Shift(l, &buffer);

*/
bool List_Shift(List* l, void* buffer);

/*
Given an index, removes the element stored at the index from the list.

Inputs:
 - List* l: the memory address of the List structure.
 - int index: the index of the element to remove.

Outputs:
 - 0: if the index could not be removed.
 - 1: if the index was successfully removed.

Time Complexity: O(n)

Example:
 - This removes the element at index 3 from the list

    List_Remove(l, 3);

*/
bool List_Remove(List* l, int index);

/*
Pushes an element to the end of the list.

Inputs:
 - List* l: the memory address of the List structure.
 - void* element: the memory address where the element is stored

Time Complexity: Amortised O(1)

Example:
 - This pushes the element to the end of the list

    float element = 3.0f
    List_Push(l, &element);

*/
void List_Push(List* l, void* element);

/*
Unshifts an element to the start of the list.

Inputs:
 - List* l: the memory address of the List structure.
 - void* element: the memory address where the element is stored

Time Complexity: O(n)

Example:
 - This unshifts the element to the end of the list

    float element = 3.0f
    List_Unshift(l, &element);

*/
void List_Unshift(List* l, void* element);

/*
Given an index, adds an element to the index in the list.
All proceeding elements are shifted up.

Inputs:
 - List* l: the memory address of the List structure.
 - int index: the index of the element to remove.
 - void* element: the memory address where the element is stored

Outputs:
 - 0: if the index could not be added.
 - 1: if the index was successfully added.

Time Complexity: O(n)

Example:
 - This adds the element to index 3 from the list

    float element = 4.0f;
    List_Remove(l, 3, &element);

*/
bool List_Add(List* l, int index, void* element);

/*
Clears all elements from a given List structure.

Inputs:
 - List* l: the memory address of the List structure.

Time Complexity: O(n)

Example:
 - This clears a list.

    List_Clear(l);

*/
void List_Clear(List* l);

/*
Frees all associated memory with a List structure.

Inputs:
 - List* l: the memory address of the List structure.

Time Complexity: O(n)

Example:
 - This frees the list.

    List_Free(l);

*/
void List_Free(List* l);

#endif