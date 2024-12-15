#ifndef LIST_H
#define LIST_H

#include "type.h"

typedef struct {
    i32 length;
    i32 max_length;
    void* buffer;
} List;

void list_push(List* list, void* element);
void list_pop(List* list, i32 idx);
void list_destroy(List* list);

#endif