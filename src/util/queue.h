#ifndef QUEUE_H
#define QUEUE_H

typedef struct QNode QNode;

typedef struct QNode {
    void*  data;
    QNode* next;
} QNode;

typedef struct Queue {
    QNode* head;
    QNode* tail;
} Queue;

Queue* queue_create(void);
void   queue_enqueue(Queue* queue, void* data);
void*  queue_dequeue(Queue* queue);
int    queue_empty(Queue* queue);
void   queue_destroy(Queue* queue);

#endif