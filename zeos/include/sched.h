/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <mm_address.h>
#include <types.h>

#define NR_TASKS 10
#define KERNEL_STACK_SIZE 1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };
struct task_struct {
    int PID; /* Process ID. This MUST be the first field of the struct. */
    page_table_entry *dir_pages_baseAddr;
    struct list_head list;
    unsigned long kernel_esp;

    /* Scheduling fields for round-robin */
    int quantum;

    /* Process hierarchy fields */
    struct task_struct *parent;
    struct list_head children;   /* List of child processes */
    struct list_head child_list; /* Entry in parent's children list */

    /* Blocking mechanism */
    int pending_unblocks;
};

union task_union {
    struct task_struct task;
    unsigned long stack[KERNEL_STACK_SIZE]; /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct *idle_task;
extern struct task_struct *init_task;

#define KERNEL_ESP(t) (DWord) & (t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP KERNEL_ESP(&task[1])

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

/* Global queues for process management */
extern struct list_head freequeue;
extern struct list_head readyqueue;
extern struct list_head blocked;

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
 * @brief Return from fork system call for child process.
 *
 * This function is used by child processes created by fork() to return
 * to user space with the appropriate return value (0 for child).
 */
extern void ret_from_fork(void);

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
 * @param t Pointer to the task structure.
 * @return 0 on success, negative error code on failure.
 */
int allocate_DIR(struct task_struct *t);

/**
 * @brief Get page table for a task.
 *
 * This function returns a pointer to the page table of the specified task.
 * @param t Pointer to the task structure.
 * @return Pointer to the task's page table.
 */
page_table_entry *get_PT(struct task_struct *t);

/**
 * @brief Get page directory for a task.
 *
 * This function returns a pointer to the page directory of the specified task.
 * @param t Pointer to the task structure.
 * @return Pointer to the task's page directory.
 */
page_table_entry *get_DIR(struct task_struct *t);

/* Headers for the scheduling policy */
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
 * @param t Pointer to the task structure to update.
 * @param dest Destination queue for the process.
 */
void update_process_state_rr(struct task_struct *t, struct list_head *dest);

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
void schedule();

/**
 * @brief Get quantum value for a task.
 *
 * This function returns the quantum value assigned to the specified task.
 * @param t Pointer to the task structure.
 * @return Quantum value of the task.
 */
int get_quantum(struct task_struct *t);

/**
 * @brief Set quantum value for a task.
 *
 * This function sets the quantum value for the specified task.
 * @param t Pointer to the task structure.
 * @param new_quantum New quantum value to set.
 */
void set_quantum(struct task_struct *t, int new_quantum);

/* PID management */
/**
 * @brief Get next available process ID.
 *
 * This function returns the next available process identifier and
 * increments the internal PID counter.
 * @return The next available PID.
 */
int get_next_pid(void);

#endif /* __SCHED_H__ */
