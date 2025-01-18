#ifndef STACK_H
#define STACK_H

typedef struct SNode SNode;

typedef struct SNode {
    void* data;
    SNode* next;
} SNode;

typedef struct Stack {
    SNode* head;
} Stack;

Stack* stack_create(void);
void   stack_push(Stack* stack, void* data);
void*  stack_pop(Stack* stack);
void*  stack_peek(Stack* stack);
int    stack_empty(Stack* stack);
void   stack_destroy(Stack* stack);

#endif