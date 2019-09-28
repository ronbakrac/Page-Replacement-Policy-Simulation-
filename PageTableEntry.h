// C program for array implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
// C program for array implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct PTE
{
    bool dirty;     // true when written to
    bool present;   // true when in RAM
    int pageNumber; // page number of trace and how it is indexed in map
    int lastUsed;   // used for LRU to specify when in time last used
    bool inClean; // Used for VMS to specify if its in the clean queue
    bool inDirty; // Used for VMS to specify if its in the dirty queue
} PTE;