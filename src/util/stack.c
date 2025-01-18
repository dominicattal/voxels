#include "stack.h"
#include <stdlib.h>

SNode* snode_create(void* data) {
    SNode* node = malloc(sizeof(SNode));
    node->data = data;
    node->next = NULL;
    return node;
}

void snode_destroy(SNode* node) {
    free(node);
}

Stack* stack_create(void) {
    Stack* stack = malloc(sizeof(Stack));
    stack->head = NULL;
    return stack;
}

void stack_push(Stack* stack, void* data) {
    SNode* node = snode_create(data);
    if (stack->head != NULL)
        node->next = stack->head;
    stack->head = node;
}

void* stack_pop(Stack* stack) {
    SNode* prev = stack->head;
    void* data = prev->data;
    stack->head = prev->next;
    snode_destroy(prev);
    return data;
}

void* stack_peek(Stack* stack) {
    return stack->head->data;
}

int stack_empty(Stack* stack) {
    return stack->head == NULL;
}

void stack_destroy(Stack* stack) {
    while (stack->head != NULL) {
        SNode* next = stack->head->next;
        free(stack->head);
        stack->head = next;
    }
    free(stack);
}