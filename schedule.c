#include "scheduler.h"
#include "process.h"

// Ready list (queue) implementation
struct qentry readylist[NQENT];

// Preemption counter
uint32_t preempt;

// Head and tail of ready list
static pid32 queuehead;
static pid32 queuetail;

/*------------------------------------------------------------------------
 * resched  -  Reschedule processor to highest priority ready process
 *------------------------------------------------------------------------
 */
void resched(void) {
    struct procent *oldptr;     /* Ptr to old process entry */
    struct procent *newptr;     /* Ptr to new process entry */
    
    oldptr = &proctab[currpid];
    
    // Check if rescheduling is needed
    if (oldptr->prstate == PR_CURR) {
        if (oldptr->prprio > getfirst(queuetail)) {
            // Current process has higher priority
            return;
        }
        // Move current to ready
        oldptr->prstate = PR_READY;
        insert(currpid, queuetail, oldptr->prprio);
    }
    
    // Get highest priority process from ready list
    currpid = dequeue(queuetail);
    newptr = &proctab[currpid];
    newptr->prstate = PR_CURR;
    preempt = QUANTUM;      /* Reset time slice */
    
    // Context switch
    ctxsw(&oldptr->prstkptr, &newptr->prstkptr);
}

/*------------------------------------------------------------------------
 * yield  -  Voluntarily relinquish the CPU (end of quantum)
 *------------------------------------------------------------------------
 */
syscall yield(void) {
    resched();
    return OK;
}

/*------------------------------------------------------------------------
 * Ready list (queue) management functions
 *------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------
 * insert  -  Insert a process into a queue in descending key order
 *------------------------------------------------------------------------
 */
void insert(pid32 pid, pid32 qid, int32_t key) {
    pid32 curr;     /* Runs through items in queue */
    pid32 prev;
    
    curr = readylist[qid].qnext;
    
    // Find insertion point
    while (readylist[curr].qkey >= key) {
        curr = readylist[curr].qnext;
    }
    
    // Insert process before curr
    prev = readylist[curr].qprev;
    
    readylist[pid].qnext = curr;
    readylist[pid].qprev = prev;
    readylist[pid].qkey = key;
    readylist[prev].qnext = pid;
    readylist[curr].qprev = pid;
}

/*------------------------------------------------------------------------
 * dequeue  -  Remove and return first process from queue
 *------------------------------------------------------------------------
 */
pid32 dequeue(pid32 qid) {
    pid32 pid;
    
    if (isempty(qid)) {
        return (pid32)SYSERR;
    }
    
    pid = getfirst(qid);
    getitem(pid);  // Remove from queue
    return pid;
}

/*------------------------------------------------------------------------
 * getfirst  -  Return ID of first process on queue (not removed)
 *------------------------------------------------------------------------
 */
pid32 getfirst(pid32 qid) {
    return readylist[qid].qnext;
}

/*------------------------------------------------------------------------
 * getlast  -  Return ID of last process on queue (not removed)
 *------------------------------------------------------------------------
 */
pid32 getlast(pid32 qid) {
    return readylist[readylist[qid].qprev].qkey;
}

/*------------------------------------------------------------------------
 * getitem  -  Remove a process from an arbitrary point in a queue
 *------------------------------------------------------------------------
 */
pid32 getitem(pid32 pid) {
    pid32 prev, next;
    
    next = readylist[pid].qnext;
    prev = readylist[pid].qprev;
    
    readylist[prev].qnext = next;
    readylist[next].qprev = prev;
    
    return pid;
}

/*------------------------------------------------------------------------
 * isempty  -  Check if queue is empty
 *------------------------------------------------------------------------
 */
int isempty(pid32 qid) {
    return (readylist[qid].qnext >= NPROC);
}

/*------------------------------------------------------------------------
 * newqueue  -  Allocate and initialize a queue in the global queue table
 *------------------------------------------------------------------------
 */
pid32 newqueue(void) {
    static pid32 nextqid = NPROC;
    pid32 qid;
    
    if (nextqid >= NQENT) {
        return (pid32)SYSERR;
    }
    
    qid = nextqid++;
    
    // Initialize queue to empty
    readylist[qid].qnext = qid + 1;
    readylist[qid + 1].qprev = qid;
    readylist[qid + 1].qnext = qid + 1;
    
    return qid;
}
