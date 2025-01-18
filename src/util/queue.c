#include "queue.h"
#include <stdlib.h>

QNode* qnode_create(void* data) {
    QNode* node = malloc(sizeof(QNode));
    node->data = data;
    node->next = NULL;
    return node;
}

void qnode_destroy(QNode* node) {
    free(node);
}

Queue* queue_create(void) {
    Queue* queue = malloc(sizeof(Queue));
    queue->head = queue->tail = NULL;
    return queue;
}

void queue_enqueue(Queue* queue, void* data) {
    QNode* node = qnode_create(data);
    if (queue->head == NULL)
        queue->head = queue->tail = node;
    else
        queue->tail = queue->tail->next = node;
}

void* queue_dequeue(Queue* queue) {
    if (queue->tail == NULL)
        return NULL;
    void* data = queue->head->data;
    if (queue->head == queue->tail)
        queue->head = queue->tail = NULL;
    else
        queue->head = queue->head->next;
    return data;
}

int queue_empty(Queue* queue) {
    return queue->head == NULL;
}

void queue_destroy(Queue* queue) {
    QNode* cur = queue->head;
    while (cur != NULL) {
        QNode* next = cur->next;
        qnode_destroy(cur);
        cur = next;
    }
}