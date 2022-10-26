#ifndef LIST_H
#define LIST_H

struct List {
    
    void* elements
    size_t element_size;
    size_t size;
    size_t n;

};
typedef struct List List;

void List_Init(List* l, size_t element_size);

int List_Get(List* l, size_t index, void* buffer);

int List_Pop(List* l, void* buffer);

int List_Shift(List* l, void* buffer);

int List_Remove(List* l, size_t index);

void List_Push(List* l, void* element);

void List_Unshift(List* l, void* element);

void List_Add(List* l, size_t index, void* element)

void List_Free(HashMap* h);

#endif