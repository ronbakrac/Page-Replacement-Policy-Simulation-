#include <stdbool.h>
#include <string.h>
#include "vms.h"

// 2^20: the number of possible pages needed for direct map
#define MB 1048576

// Number of times a disk write occurs
int diskWrites = 0;

// Direct map of page number to index. So we need 2^20 entries
// This must be declared in global to prevent seg fault, see:
// https://stackoverflow.com/questions/3815232/seg-fault-when-initializing-array
struct PTE *pageTable[MB];

void fifo(struct PTE *RAM[], struct PTE *entry, int sizeOfRam, int iterator);
void lru(struct PTE *RAM[], struct PTE *entry, int sizeOfRam);

int append(struct PTE *RAM[], struct PTE *entry, int nFrameIndex);
bool isNotFull(struct PTE *RAM[], int maxFrames);

void enqueueDirtyOrClean(struct Queue *clean, struct Queue *dirty, struct PTE *entry, bool isRamFull);

int main(int argc, char *argv[])
{
    // Informatin from the mem trace parse
    unsigned pgNumber;
    unsigned offset;
    char rw;

    // Initialize the command line arguments
    char *traceFile = argv[1];              // File to get Trace from
    int maxFrames = atoi(argv[2]);          // the number of frames allocated
    char *policy = argv[3];                 // the page replacement policy
    bool debug = !strcmp(argv[4], "debug"); // debug or quiet

    // Index that keeps track of last element in RAM
    int nFrameIndex = 0;

    // Iterator to help keep track of time for least recently used algo
    int timeInTrace = 0;
    // Iterator to help keep track of whats in RAM first for FIFO
    int fifoIterator = 0;

    // Keeps track of how many misses (reads) there are
    int miss = 0;

    // Ram with size specified from command line arg
    struct PTE *RAM[maxFrames];

    // Open the .trace file specified
    FILE *file = fopen(traceFile, "r");
    if (file == NULL)
    {
        printf("Error! Could not open file\n");
        exit(-1); // must include stdlib.h
    }

    // Initialize our arrays
    for (int i = 0; i < MB; i++)
    {
        pageTable[i] = malloc(sizeof(struct PTE));
        pageTable[i] = NULL;
    }
    for (int i = 0; i < maxFrames; i++)
    {
        RAM[i] = malloc(sizeof(struct PTE));
        RAM[i] = NULL;
    }

    // If the policy is vms, handle separately from fifo and lru due to complexity
    if (strcmp(policy, "vms") == 0)
    {
        int RSS = maxFrames / 2;
        int dirtyCleanSize = RSS + 1;
        struct Queue *processA = createQueue(RSS);
        struct Queue *processB = createQueue(RSS);
        struct Queue *dirtQueue = createQueue(dirtyCleanSize);
        struct Queue *cleanQueue = createQueue(dirtyCleanSize);

        vms(file, pageTable, processA, processB, cleanQueue, dirtQueue, maxFrames, debug);
        return 0;
    }

    // **** FIFO and LRU ****

    // We only need first 5 bits for page #
    while (fscanf(file, "%05x %3x %c", &pgNumber, &offset, &rw) == 3)
    {
        // Defualt dirty to false and update to true if rw is W or entry was already dirty
        bool dirty = false;
        if (rw == 'W' || rw == 'w' || (pageTable[pgNumber] != NULL && pageTable[pgNumber]->dirty))
        {
            dirty = true;
        }

        // Update page table if this entry doesnt exist
        if (pageTable[pgNumber] == NULL)
        {
            // The entry should be present if the RAM is not full
            bool present = isNotFull(RAM, maxFrames);

            PTE *entry = malloc(sizeof(struct PTE));
            *entry = (PTE){.dirty = false, .present = present, .pageNumber = pgNumber, .lastUsed = timeInTrace};
            pageTable[pgNumber] = entry;

            // If we can add it to RAM, append and miss++
            if (present)
            {
                nFrameIndex = append(RAM, entry, nFrameIndex);
                miss++;
            }
        }

        // We must evict if RAM is full and the page isnt present
        if (!isNotFull(RAM, maxFrames) && !pageTable[pgNumber]->present)
        {
            // Check which policy we want to use from command line
            if (strcmp(policy, "fifo") == 0)
            {
                fifo(RAM, pageTable[pgNumber], maxFrames, fifoIterator);
                fifoIterator++;
            }
            else if (strcmp(policy, "lru") == 0)
            {
                lru(RAM, pageTable[pgNumber], maxFrames);
            }
            miss++;
        }

        // Update the entry's dirty, last used, then increase time
        pageTable[pgNumber]->dirty = dirty;
        pageTable[pgNumber]->lastUsed = timeInTrace;
        timeInTrace++;

        if (debug)
        {
            printf("Step Number: %d\n", timeInTrace);
            printf("Total disk reads: %d\n", miss);
            printf("Total disk writes: %d\n", diskWrites);
        }
    }

    // **** Quiet output ****
    printf("Total memory frames: %d\n", maxFrames);
    printf("Events in trace: %d\n", timeInTrace);
    printf("Total disk reads: %d\n", miss);
    printf("Total disk writes: %d\n", diskWrites);

    return 0;
}

// FIFO page replacement policy
// Accessing elements of an array using[iterator % sizeOfRam]
// is a simple implementation of a queue and fulfills what we need
// where RAM is our queue in this scenario
void fifo(struct PTE *RAM[], struct PTE *entry, int sizeOfRam, int iterator)
{
    // If what we are going to replace is dirty, we must write
    if (RAM[iterator % sizeOfRam]->dirty)
    {
        diskWrites++;
    }

    // Set the new entry to present and the old entry to not present, then update RAM
    RAM[iterator % sizeOfRam]->present = false;
    RAM[iterator % sizeOfRam]->dirty = false;

    entry->present = true;
    RAM[iterator % sizeOfRam] = entry;
}

// lru page replacement policy
void lru(struct PTE *RAM[], struct PTE *entry, int sizeOfRam)
{
    // Set min initially to INT_MAX to ensure we get true min
    int minimum = INT_MAX;
    int minIndex = 0;
    // Find the min last used. This corresponds to the last used in RAM
    for (int i = 0; i < sizeOfRam; i++)
    {
        if (RAM[i]->lastUsed < minimum)
        {
            minimum = RAM[i]->lastUsed;
            minIndex = i;
        }
    }

    // If what we are going to replace is dirty, we must write
    if (RAM[minIndex]->dirty)
    {
        diskWrites++;
    }

    // Set the new entry to present and the old entry to not present, then update RAM
    RAM[minIndex]->present = false;
    RAM[minIndex]->dirty = false;

    entry->present = true;
    RAM[minIndex] = entry;
}

// Convenience function used for first building RAM before swapping
// Keep track of last index to give us O(1) complexity
int append(struct PTE *RAM[], struct PTE *entry, int nFrameIndex)
{
    RAM[nFrameIndex] = entry;
    nFrameIndex++;
    return nFrameIndex;
}

// Convenience function that returns true if RAM is not full
bool isNotFull(struct PTE *RAM[], int maxFrames)
{
    return RAM[maxFrames - 1] == NULL;
}
