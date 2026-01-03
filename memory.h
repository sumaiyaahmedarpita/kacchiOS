#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

// Stack management (Xinu-style)
#define MINSTK      1024        /* Minimum stack size */
#define STKSIZE     8192        /* Default stack size */

// Memory block header for heap allocation
struct memblk {
    struct memblk *mnext;       /* Pointer to next block */
    uint32_t mlength;           /* Size of block in bytes */
};

// External declarations
extern struct memblk memlist;   /* Head of free memory list */

// Function prototypes

/* Stack allocation */
char *getstk(uint32_t nbytes);
syscall freestk(char *ptr, uint32_t nbytes);

/* Heap allocation */
char *getmem(uint32_t nbytes);
syscall freemem(char *ptr, uint32_t nbytes);

/* Memory initialization */
void meminit(void);

#endif // MEMORY_H
