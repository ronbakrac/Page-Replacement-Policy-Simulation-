// C program for array implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
// C program for array implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "PageTableEntry.h"

// Modified source code from:
// https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/

struct QNode
{
    int key;
    struct PTE *entry;
    struct QNode *next;
};

// The queue, front stores the front node of LL and rear stores ths
// last node of LL
struct Queue
{
    struct QNode *front, *rear;
    int size, capacity;
};

// A utility function to create a new linked list node.
struct QNode *newNode(struct PTE *entry)
{
    struct QNode *temp = (struct QNode *)malloc(sizeof(struct QNode));
    temp->key = entry->pageNumber;
    temp->entry = entry;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue *createQueue(int capacity)
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

int isFull(struct Queue *q)
{
    return q->capacity == q->size;
}

void outputQueue(struct Queue *q)
{
    if (q->front == NULL)
    {
        printf("[] 0\n");
        return;
    }
    struct QNode *copy = q->front;
    printf("[");
    while (copy != NULL)
    {
        printf("%d ", copy->key);
        copy = copy->next;
    }
    printf("] %d\n", q->size);
}

// The function to add a PTE entry to q
void enqueue(struct Queue *q, struct PTE *entry)
{
    // Create a new LL node
    struct QNode *temp = newNode(entry);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        q->size++;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    q->size++;
    // printf("Enqueued %d\n", entry->pageNumber);
}

// Function to remove a PTE from given queue
struct PTE *dequeue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;

    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    q->front = q->front->next;
    q->size--;

    // printf("Dequeued %d\n", temp->entry->pageNumber);
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    return temp->entry;
}

// Finds the specified pageNumber in the queue and removes it
// while still maintaining queue integrity
void updateQueue(struct Queue *queue, int pageNumber, bool dirty)
{
    struct QNode *copy = queue->front;
    struct QNode *prev;

    // If it is the first entry we can just dequeue
    if (copy->key == pageNumber)
    {
        copy->entry->inDirty = false;
        copy->entry->inClean = false;
        dequeue(queue);
        return;
    }

    while (copy != NULL)
    {
        if (copy->key == pageNumber)
            break;
        prev = copy;
        copy = copy->next;
    }

    // Make sure to remove from dirty/clean
    copy->entry->inDirty = false;
    copy->entry->inClean = false;

    // If we remove the end, be sure to update rear
    if (copy->next == NULL) {
        queue->rear = prev;
    }

    // Link prev to next
    prev->next = copy->next;

    queue->size--;
}