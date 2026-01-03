#include "process.h"
#include "memory.h"

// Process table
static pcb_t* process_table[MAX_PROCESSES] = {NULL};
static pid_t next_pid = 1;
static pcb_t* current_process = NULL;

// Helper function to find free slot in process table
static int find_free_slot(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == NULL || 
            process_table[i]->state == TERMINATED) {
            return i;
        }
    }
    return -1;
}

void process_init(void) {
    // Initialize all process table entries to NULL
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = NULL;
    }
    next_pid = 1;
    current_process = NULL;
}

pid_t process_create(void (*entry_point)(void)) {
    // Find free slot
    int slot = find_free_slot();
    if (slot == -1) {
        return 0; // No free slots
    }
    
    // Allocate PCB
    pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
    if (pcb == NULL) {
        return 0; // Memory allocation failed
    }
    
    // Allocate stack
    void* stack = allocate_stack();
    if (stack == NULL) {
        free(pcb);
        return 0; // Stack allocation failed
    }
    
    // Initialize PCB
    pcb->pid = next_pid++;
    pcb->state = READY;
    pcb->stack_base = stack;
    pcb->stack_ptr = stack;
    pcb->next = NULL;
    
    // Initialize context
    // Clear all registers
    pcb->context.eax = 0;
    pcb->context.ebx = 0;
    pcb->context.ecx = 0;
    pcb->context.edx = 0;
    pcb->context.esi = 0;
    pcb->context.edi = 0;
    pcb->context.ebp = (uint32_t)stack;
    pcb->context.esp = (uint32_t)stack;
    pcb->context.eip = (uint32_t)entry_point;
    pcb->context.eflags = 0x200; // Enable interrupts
    
    // Add to process table
    process_table[slot] = pcb;
    
    return pcb->pid;
}

void process_run(pid_t pid) {
    pcb_t* pcb = get_process(pid);
    if (pcb != NULL && pcb->state != TERMINATED) {
        // Set previous current process to READY if it exists
        if (current_process != NULL && current_process->state == CURRENT) {
            current_process->state = READY;
        }
        
        pcb->state = CURRENT;
        current_process = pcb;
    }
}

void process_ready(pid_t pid) {
    pcb_t* pcb = get_process(pid);
    if (pcb != NULL && pcb->state != TERMINATED) {
        pcb->state = READY;
        
        // Update current_process if this was the current process
        if (current_process == pcb) {
            current_process = NULL;
        }
    }
}

void process_terminate(pid_t pid) {
    pcb_t* pcb = get_process(pid);
    if (pcb != NULL) {
        pcb->state = TERMINATED;
        
        // Free stack
        if (pcb->stack_base != NULL) {
            free_stack(pcb->stack_base);
            pcb->stack_base = NULL;
        }
        
        // Update current_process if this was the current process
        if (current_process == pcb) {
            current_process = NULL;
        }
        
        // Free PCB
        // Note: We keep the pointer in process_table for now
        // It will be reused when a new process is created
    }
}

pcb_t* get_process(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] != NULL && process_table[i]->pid == pid) {
            return process_table[i];
        }
    }
    return NULL;
}

pcb_t* get_current_process(void) {
    return current_process;
}

int get_process_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] != NULL && 
            process_table[i]->state != TERMINATED) {
            count++;
        }
    }
    return count;
}

void print_process_table(void) {
    // Use your io.c functions to print process information
    // Example format:
    // PID | State | Stack Base
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] != NULL && 
            process_table[i]->state != TERMINATED) {
            pcb_t* pcb = process_table[i];
            // printf("PID: %u, State: %d, Stack: %p\n", 
            //        pcb->pid, pcb->state, pcb->stack_base);
        }
    }
}
