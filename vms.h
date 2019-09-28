#include "linkedQueue.h"

void enqueueByBit(struct Queue *clean, struct Queue *dirty, struct PTE *entry, bool procABFull);
void debugOutput(int timeInTrace, int miss, int writes, struct Queue *processA, struct Queue *processB, struct Queue *cleanQueue, struct Queue *dirtQueue);

int writes = 0;

void vms(FILE *file, struct PTE *pageTable[], struct Queue *processA, struct Queue *processB, struct Queue *cleanQueue, struct Queue *dirtQueue, int maxFrames, bool debug)
{
    unsigned pgNumber;
    unsigned offset;
    char rw;

    int timeInTrace = 0;

    int miss = 0;

    // We only need first 5 bits for page #
    while (fscanf(file, "%05x %3x %c", &pgNumber, &offset, &rw) == 3)
    {
        bool isRamFull = (maxFrames == (processA->size + processB->size + dirtQueue->size + cleanQueue->size));
        bool isProcABFull = (maxFrames == (processA->size + processB->size));

        // Bit shift 16 bits to get upper 4 bits to check if its a 3, if so we're in processB
        unsigned processNumber = pgNumber >> 16;
        bool isProcessB = (processNumber == 3);
        struct Queue *processX = isProcessB ? processB : processA;

        // Defualt dirty to false and update to true if rw is W or entry was already dirty
        bool dirty = false;
        if (rw == 'W' || rw == 'w' || (pageTable[pgNumber] != NULL && pageTable[pgNumber]->dirty))
        {
            dirty = true;
        }

        // Update page table if this entry doesnt exist
        if (pageTable[pgNumber] == NULL)
        {
            PTE *entry = malloc(sizeof(struct PTE));
            *entry = (PTE){.dirty = false, .present = false, .pageNumber = pgNumber};
            pageTable[pgNumber] = entry;
        }

        // Update if we dirtied
        struct PTE *entry = pageTable[pgNumber];
        entry->dirty = dirty;

        if (debug)
        {
            printf("Before Step Number: %d\n", timeInTrace + 1);
            printf("page entry: %d\npage dirty: %d\n", entry->pageNumber, entry->dirty);
            printf("Present: %d %d %d\n", entry->present, entry->inClean, entry->inDirty);
            printf("AB Size max: %d\n\n", isProcABFull);
        }

        // If our entry is present
        if (entry->present)
        {
            // first check if its in the process list, we do nothing then
            if (!entry->inClean && !entry->inDirty)
            {
                timeInTrace++;
                if (debug)
                {
                    debugOutput(timeInTrace, miss, writes, processA, processB, cleanQueue, dirtQueue);
                }
                continue;
            }

            // If it is not in the process list but is present, the process list MUST be full
            struct PTE *front = dequeue(processX);
            enqueueByBit(cleanQueue, dirtQueue, front, isProcABFull);

            // We then update the entry from clean/dirty back into the process list
            if (entry->inClean)
            {
                // Remove present from clean and enq to processX
                updateQueue(cleanQueue, entry->pageNumber, false);
            }
            else
            {
                // Remove present from dirty and enq to processX
                updateQueue(dirtQueue, entry->pageNumber, true);
            }
            // And finally enqueue our entry from dirty/clean back to process list
            enqueue(processX, entry);
        }
        else
        {
            // If its not present, we need a disk read
            miss++;

            // It is not present so we must check if RAM is full
            if (isRamFull)
            {
                // First we check if we can take something from clean
                if (cleanQueue->size > 0)
                {
                    struct PTE *front = dequeue(cleanQueue);
                    front->inClean = false;
                    front->present = false;
                    if (isFull(processX))
                    {
                        front = dequeue(processX);
                        enqueueByBit(cleanQueue, dirtQueue, front, isProcABFull);
                    }
                    entry->present = true;
                    enqueue(processX, entry);
                }
                // Otherwise check if we can take something from dirty
                else if (dirtQueue->size > 0)
                {
                    struct PTE *front = dequeue(dirtQueue);
                    front->inDirty = false;
                    front->present = false;
                    front->dirty = false;
                    writes++;
                    if (isFull(processX))
                    {
                        front = dequeue(processX);
                        enqueueByBit(cleanQueue, dirtQueue, front, isProcABFull);
                    }
                    entry->present = true;
                    enqueue(processX, entry);
                }
                // Otherwise both process full and we must take something from a process
                else
                {
                    struct PTE *front = dequeue(processX);
                    front->present = false;
                    if (front->dirty)
                    {
                        writes++;
                        front->dirty = false;
                    }
                    entry->present = true;
                    enqueue(processX, entry);
                }
            }
            // If RAM is not full
            else
            {
                if (isFull(processX))
                {
                    struct PTE *front = dequeue(processX);
                    enqueueByBit(cleanQueue, dirtQueue, front, isProcABFull);
                }
                entry->present = true;
                enqueue(processX, entry);
            }
        }

        timeInTrace++;
        if (debug)
        {
            debugOutput(timeInTrace, miss, writes, processA, processB, cleanQueue, dirtQueue);
        }
    }
    printf("Total memory frames: %d\n", maxFrames);
    printf("Events in trace: %d\n", timeInTrace);
    printf("Total disk reads: %d\n", miss);
    printf("Total disk writes: %d\n", writes);
}

// Helper function for outputting the queues to debug
void debugOutput(int timeInTrace, int miss, int writes, struct Queue *processA, struct Queue *processB, struct Queue *cleanQueue, struct Queue *dirtQueue)
{
    printf("After step number %d\n", timeInTrace);
    printf("ProcessA: ");
    outputQueue(processA);
    printf("ProcessB: ");
    outputQueue(processB);
    printf("Clean: ");
    outputQueue(cleanQueue);
    printf("Dirty: ");
    outputQueue(dirtQueue);
    printf("Sizes: %d, %d, %d, %d\n", processA->size, processB->size, cleanQueue->size, dirtQueue->size);
    printf("Disk writes: %d\nMiss: %d\n************\n\n", writes, miss);
}

// Enqueues entry to dirty or clean depending if the entry is dirty or clean
void enqueueByBit(struct Queue *clean, struct Queue *dirty, struct PTE *entry, bool procABFull)
{
    if (procABFull)
    {
        if (entry->dirty)
        {
            writes++;
            entry->dirty = false;
        }
        entry->present = false;
        return;
    }

    if (entry->dirty)
    {
        // If dirty is full, we must deq it, causing a write
        if (isFull(dirty))
        {
            struct PTE *front = dequeue(dirty);
            front->present = false;
            front->inDirty = false;
            front->dirty = false;
            writes++;
        }
        entry->inDirty = true;
        enqueue(dirty, entry);
    }
    else
    {
        if (isFull(clean))
        {
            struct PTE *front = dequeue(clean);
            front->present = false;
            front->inClean = false;
        }
        entry->inClean = true;
        enqueue(clean, entry);
    }
}
