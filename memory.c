#include "memory.h"
#include "process.h"

// Memory pool definitions
#define MEMSIZE (1024 * 1024)   /* 1 MB memory pool */

// Static memory pool
static char mempool[MEMSIZE] __attribute__((aligned(8)));

// Free memory list head
struct memblk memlist;

/*------------------------------------------------------------------------
 * meminit  -  Initialize the free memory list
 *------------------------------------------------------------------------
 */
void meminit(void) {
    struct memblk *memptr;
    
    // Initialize free list with entire memory pool
    memptr = (struct memblk *)mempool;
    memlist.mnext = memptr;
    memptr->mnext = (struct memblk *)NULL;
    memptr->mlength = MEMSIZE;
}

/*------------------------------------------------------------------------
 * getstk  -  Allocate stack memory for a process
 *------------------------------------------------------------------------
 */
char *getstk(uint32_t nbytes) {
    char *saddr;
    
    // Round up to multiple of 8 bytes
    nbytes = (uint32_t)roundmb(nbytes);
    
    if (nbytes < MINSTK) {
        nbytes = MINSTK;
    }
    
    // Allocate memory
    saddr = getmem(nbytes);
    
    return saddr;
}

/*------------------------------------------------------------------------
 * freestk  -  Free stack memory allocated for a process
 *------------------------------------------------------------------------
 */
syscall freestk(char *ptr, uint32_t nbytes) {
    // Round up to multiple of 8 bytes
    nbytes = (uint32_t)roundmb(nbytes);
    
    return freemem(ptr, nbytes);
}

/*------------------------------------------------------------------------
 * getmem  -  Allocate heap storage, returning lowest address
 *------------------------------------------------------------------------
 */
char *getmem(uint32_t nbytes) {
    struct memblk *prev, *curr, *leftover;
    
    if (nbytes == 0) {
        return (char *)SYSERR;
    }
    
    // Round to multiple of memblk size
    nbytes = (uint32_t)roundmb(nbytes);
    
    // Walk free list to find block that is large enough
    prev = &memlist;
    curr = memlist.mnext;
    
    while (curr != NULL) {
        if (curr->mlength >= nbytes) {
            // Found a block that fits
            
            if (curr->mlength == nbytes) {
                // Exact fit - remove from free list
                prev->mnext = curr->mnext;
            } else {
                // Split block
                leftover = (struct memblk *)((char *)curr + nbytes);
                leftover->mnext = curr->mnext;
                leftover->mlength = curr->mlength - nbytes;
                prev->mnext = leftover;
            }
            
            return (char *)curr;
        }
        
        prev = curr;
        curr = curr->mnext;
    }
    
    return (char *)SYSERR;  // No block large enough
}

/*------------------------------------------------------------------------
 * freemem  -  Free a memory block, returning it to free list
 *------------------------------------------------------------------------
 */
syscall freemem(char *ptr, uint32_t nbytes) {
    struct memblk *block, *prev, *curr;
    
    if (nbytes == 0 || ptr == NULL) {
        return SYSERR;
    }
    
    // Round to multiple of memblk size
    nbytes = (uint32_t)roundmb(nbytes);
    
    block = (struct memblk *)ptr;
    block->mlength = nbytes;
    
    // Walk free list to find insertion point
    prev = &memlist;
    curr = memlist.mnext;
    
    // Find spot in list (ordered by address)
    while (curr != NULL && curr < block) {
        prev = curr;
        curr = curr->mnext;
    }
    
    // Insert block
    prev->mnext = block;
    block->mnext = curr;
    
    // Try to coalesce with next block
    if (((char *)block + block->mlength) == (char *)curr) {
        block->mlength += curr->mlength;
        block->mnext = curr->mnext;
    }
    
    // Try to coalesce with previous block
    if (prev != &memlist) {
        if (((char *)prev + prev->mlength) == (char *)block) {
            prev->mlength += block->mlength;
            prev->mnext = block->mnext;
        }
    }
    
    return OK;
}

/*------------------------------------------------------------------------
 * roundmb  -  Round address up to multiple of memblk size
 *------------------------------------------------------------------------
 */
static uint32_t roundmb(uint32_t nbytes) {
    uint32_t remainder;
    
    remainder = nbytes % sizeof(struct memblk);
    if (remainder != 0) {
        nbytes += sizeof(struct memblk) - remainder;
    }
    
    return nbytes;
}
