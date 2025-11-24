/**
 * @file sched.h
 * @brief Process scheduling and task management definitions for ZeOS.
 *
 * This header defines task structures, process states, scheduling
 * functions, and process management interfaces for the ZeOS kernel.
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <mm_address.h>
#include <types.h>

/* Maximum number of tasks in the system */
#define NR_TASKS 10

/* Size of the kernel stack for each process in words */
#define KERNEL_STACK_SIZE 1024

/* Default quantum assigned to new processes (10 ticks default)*/
#define DEFAULT_QUANTUM 1000

/* Process states for scheduling */
enum state_t {
    ST_RUN,    /* Currently running */
    ST_READY,  /* Ready to run, in ready queue */
    ST_BLOCKED /* Blocked waiting for an event */
};

/* Process control block structure */
struct task_struct {
    int PID;                              /* Process identifier*/
    page_table_entry *dir_pages_baseAddr; /* Page directory base address */
    struct list_head list;                /* Entry in process queues */
    unsigned long kernel_esp;             /* Kernel stack pointer */
    int quantum;                          /* Current process state */
    enum state_t status;                  /* Current process state */
    struct task_struct *parent;           /* Pointer to parent process */
    struct list_head children;            /* List of child processes */
    struct list_head child_list;          /* Entry in parent's children list */
    int pending_unblocks;                 /* Number of pending unblock operations */

    /* Thread support fields */
    int TID;                              /* Thread identifier (1 for main thread) */
    int thread_count;                     /* Number of threads in process */
    struct task_struct *master_thread;    /* Pointer to master thread */
    struct list_head threads;             /* List of threads in this process */
    struct list_head thread_list;         /* Entry in master thread's threads list */
    int *user_stack_ptr;                  /* Pointer to user stack for this thread */
    int user_stack_frames;                /* Number of pages allocated for user stack */
    unsigned int user_stack_region_start; /* First logical page reserved for the stack region */
    unsigned int user_stack_region_pages; /* Total reserved pages (mapped+gap) */
    unsigned long user_initial_esp;       /* User ESP used when the thread starts */
    unsigned long user_entry;             /* User entry point used on first dispatch */
};

/* Union for process data and stack */
union task_union {
    struct task_struct task;                /* Process control block */
    unsigned long stack[KERNEL_STACK_SIZE]; /* Kernel stack for the process */
};

extern union task_union tasks[NR_TASKS]; /* Global array of all possible tasks in the system */

extern struct task_struct *idle_task; /* Pointer to the idle task (PID 0) */
extern struct task_struct *init_task; /* Pointer to the initial user task (PID 1) */

extern struct task_struct *current_task; /* Pointer to current running task (optimization) */

extern struct list_head freequeue;    /* Queue for free (unused) task structures */
extern struct list_head readyqueue;   /* Queue for ready-to-run processes */
extern struct list_head blockedqueue; /* Queue for blocked processes */

/* Calculate kernel stack pointer for a task */
#define KERNEL_ESP(task) (DWord) & (task)->stack[KERNEL_STACK_SIZE]

/* Initial ESP for the first user process */
#define INITIAL_ESP KERNEL_ESP(&tasks[1])

/**
 * @brief Initialize the initial process task.
 *
 * This function initializes the data structures for the initial process (task1).
 * Sets up the initial task structure and prepares it for execution.
 */
void init_task1(void);

/**
 * @brief Initialize the idle task.
 *
 * This function creates and initializes the idle task which runs when no other
 * processes are ready to execute. The idle task runs with the lowest priority.
 */
void init_idle(void);

/**
 * @brief Initialize the scheduler.
 *
 * This function initializes the scheduler subsystem, setting up queues
 * and data structures needed for process scheduling.
 */
void init_sched(void);

/**
 * @brief Initialize process queues.
 *
 * This function initializes the free queue and ready queue used for
 * process management and scheduling.
 */
void init_queues(void);

/**
 * @brief Get the current running task.
 *
 * This function returns a pointer to the task_struct of the currently
 * executing process.
 * @return Pointer to the current task's task_struct.
 */
struct task_struct *current();

/**
 * @brief CPU idle function executed by the idle task.
 *
 * This function implements the idle process behavior - enables interrupts
 * and enters an infinite loop, consuming minimal CPU cycles.
 */
void cpu_idle(void);

/**
 * @brief Low-level task context switch implementation.
 *
 * This function performs the low-level context switching operations including
 * updating the TSS ESP0, setting MSR for SYSENTER, switching page directory,
 * and calling the assembly context switch routine.
 *
 * @param new Pointer to the task union of the new task to switch to.
 */
void inner_task_switch(union task_union *new);

/**
 * @brief Switch to a new task.
 *
 * This function performs a context switch to the specified task.
 * Saves the current context and loads the new task's context.
 * @param new Pointer to the task union of the new task to switch to.
 */
extern void task_switch(union task_union *new);

/**
 * @brief Low-level context switch function.
 *
 * This assembly function performs the actual context switching by saving
 * the current ESP and loading the new ESP.
 * @param current_esp Pointer to store the current ESP.
 * @param new_esp New ESP value to load.
 */
extern void switch_context(unsigned long *current_esp, unsigned long new_esp);

/**
 * @brief Convert list_head to task_struct.
 *
 * This function converts a list_head pointer to its containing task_struct.
 * Used to navigate from queue entries to task structures.
 * @param l Pointer to the list_head within a task_struct.
 * @return Pointer to the containing task_struct.
 */
struct task_struct *list_head_to_task_struct(struct list_head *l);

/**
 * @brief Allocate page directory for a task.
 *
 * This function allocates and initializes a page directory for the specified task.
 * Sets up the memory management structures for the process.
 * @param task Pointer to the task structure.
 * @return 0 on success, negative error code on failure.
 */
int allocate_DIR(struct task_struct *taskask);

/**
 * @brief Get page table for a task.
 *
 * This function returns a pointer to the page table of the specified task.
 * @param task Pointer to the task structure.
 * @return Pointer to the task's page table.
 */
page_table_entry *get_PT(struct task_struct *task);

/**
 * @brief Get page directory for a task.
 *
 * This function returns a pointer to the page directory of the specified task.
 * @param task Pointer to the task structure.
 * @return Pointer to the task's page directory.
 */
page_table_entry *get_DIR(struct task_struct *task);

/**
 * @brief Schedule next process using round-robin policy.
 *
 * This function implements the round-robin scheduling algorithm to select
 * the next process to run.
 */
void sched_next_rr();

/**
 * @brief Update process state in round-robin scheduler.
 *
 * This function updates the state of a process and moves it to the
 * appropriate queue in the round-robin scheduler.
 * @param task Pointer to the task structure to update.
 * @param dest_queue Destination queue for the process.
 */
void update_process_state_rr(struct task_struct *taskask, struct list_head *dest_queue);

/**
 * @brief Check if scheduling is needed.
 *
 * This function determines whether a context switch should occur based
 * on the round-robin scheduling policy.
 * @return 1 if scheduling is needed, 0 otherwise.
 */
int needs_sched_rr();

/**
 * @brief Update scheduling data for round-robin.
 *
 * This function updates scheduling-related data structures for the
 * round-robin scheduling algorithm.
 */
void update_sched_data_rr();

/**
 * @brief Main scheduler function.
 *
 * This function implements the main scheduling logic, calling the
 * appropriate scheduling policy functions to determine if a context
 * switch is needed and performing it if necessary.
 */
void scheduler();

/**
 * @brief Get quantum value for a task.
 *
 * This function returns the quantum value assigned to the specified task.
 * @param task Pointer to the task structure.
 * @return Quantum value of the task.
 */
int get_quantum(struct task_struct *task);

/**
 * @brief Set quantum value for a task.
 *
 * This function sets the quantum value for the specified task.
 * @param task Pointer to the task structure.
 * @param new_quantum New quantum value to set.
 */
void set_quantum(struct task_struct *taskask, int new_quantum);

/* PID management */
/**
 * @brief Get next available process ID.
 *
 * This function returns the next available process identifier and
 * increments the internal PID counter.
 * @return The next available PID.
 */
int get_next_pid(void);

/* test functions */
/**
 * @brief Print debug information before context switch.
 *
 * This function prints scheduler debug information showing the context
 * switch from current process to target process and ready queue status.
 * @param from_pid PID of the current process being switched from.
 * @param to_pid PID of the target process being switched to.
 */
void printDebugInfoSched(int from_pid, int to_pid);

#endif /* __SCHED_H__ */
