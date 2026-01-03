#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

// System call return values
#define OK      1
#define SYSERR  (-1)
typedef int32_t syscall;

// Quantum (time slice) for round-robin scheduling
#define QUANTUM 10      /* Clock ticks per time slice */

// Ready list structure (queue)
#define NQENT   (NPROC + 2)  /* Queue entries: processes + head/tail */

struct qentry {
    int32_t qkey;           /* Key used for ordering */
    pid32 qnext;            /* Index of next process */
    pid32 qprev;            /* Index of previous process */
};

// External declarations
extern struct qentry readylist[];
extern uint32_t preempt;    /* Preemption counter */

// Function prototypes

/* Rescheduling */
void resched(void);
syscall yield(void);

/* Ready queue management */
pid32 dequeue(pid32 qid);
syscall enqueue(pid32 pid, pid32 qid);
void insert(pid32 pid, pid32 qid, int32_t key);
pid32 getfirst(pid32 qid);
pid32 getlast(pid32 qid);
pid32 getitem(pid32 pid);

/* Context switch (architecture-dependent) */
void ctxsw(void *old, void *new);

/* Queue initialization */
pid32 newqueue(void);

#endif // SCHEDULER_H
