/**
 * @file sched.c
 * @brief Process scheduler and task management for ZeOS.
 *
 * This file implements the process scheduler, task initialization,
 * context switching, and process state management using round-robin
 * scheduling with process queues and memory management integration.
 */

#include <debug.h>
#include <interrupt.h>
#include <io.h>
#include <libc.h>
#include <mm.h>
#include <sched.h>
#include <segment.h>
#include <utils.h>

union task_union tasks[NR_TASKS] __attribute__((__section__(".data.task")));

/* Global PID counter for assigning unique process identifiers */
static int next_pid = 1;

/* Current quantum ticks remaining for the running process */
static int current_quantum = 0;

struct task_struct *current_task = NULL;

struct task_struct *idle_task;
struct task_struct *init_task;

struct list_head freequeue;
struct list_head readyqueue;
struct list_head blockedqueue;

struct task_struct *list_head_to_task_struct(struct list_head *l) {
    return list_entry(l, struct task_struct, list);
}

page_table_entry *get_DIR(struct task_struct *task) {
    return task->dir_pages_baseAddr;
}

page_table_entry *get_PT(struct task_struct *task) {
    return (page_table_entry *)(((unsigned int)(task->dir_pages_baseAddr->bits.pbase_addr)) << 12);
}

int allocate_DIR(struct task_struct *task) {
    unsigned long base = (unsigned long)&tasks[0].task;
    unsigned long current_task = (unsigned long)task;
    int pos = (current_task - base) / sizeof(union task_union); // Index in tasks[]
    task->dir_pages_baseAddr = (page_table_entry *)&dir_pages[pos];
    return 1;
}

void cpu_idle(void) {
    __asm__ __volatile__("sti" : : : "memory");

    while (1) {
        ;
    }
}

void init_idle(void) {
    struct list_head *free_task = freequeue.next;
    list_del(free_task);

    idle_task = list_head_to_task_struct(free_task);
    idle_task->PID = 0;

    /* Initialize quantum */
    idle_task->quantum = DEFAULT_QUANTUM;

    /* Initialize process hierarchy */
    idle_task->parent = NULL;
    INIT_LIST_HEAD(&idle_task->children);
    INIT_LIST_HEAD(&idle_task->child_list);

    /* Initialize blocking mechanism */
    idle_task->pending_unblocks = 0;

    /* Initialize thread support */
    idle_task->TID = 1;
    idle_task->thread_count = 1;
    idle_task->master_thread = idle_task;
    INIT_LIST_HEAD(&idle_task->threads);
    INIT_LIST_HEAD(&idle_task->thread_list);
    idle_task->user_stack_ptr = NULL;
    idle_task->user_stack_frames = 0;

    allocate_DIR(idle_task);

    union task_union *idle_union = (union task_union *)idle_task;

    idle_union->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)&cpu_idle;
    idle_union->stack[KERNEL_STACK_SIZE - 2] = 0;
    idle_task->kernel_esp = (unsigned long)&idle_union->stack[KERNEL_STACK_SIZE - 2];
}

void init_task1(void) {
    struct list_head *free_task = freequeue.next;
    list_del(free_task);

    init_task = list_head_to_task_struct(free_task);

    init_task->PID = 1;

    /* Initialize scheduling fields */
    init_task->quantum = DEFAULT_QUANTUM;
    current_quantum = DEFAULT_QUANTUM;

    /* Set as current running task */
    current_task = init_task;

    /* Initialize status */
    init_task->status = ST_RUN;

    /* Initialize process hierarchy */
    init_task->parent = NULL;
    INIT_LIST_HEAD(&init_task->children);
    INIT_LIST_HEAD(&init_task->child_list);

    /* Initialize blocking mechanism */
    init_task->pending_unblocks = 0;

    /* Initialize thread support */
    init_task->TID = 1;
    init_task->thread_count = 1;
    init_task->master_thread = init_task;
    INIT_LIST_HEAD(&init_task->threads);
    INIT_LIST_HEAD(&init_task->thread_list);
    init_task->user_stack_ptr = NULL;
    init_task->user_stack_frames = 0;

    allocate_DIR(init_task);
    set_user_pages(init_task);

    union task_union *init_union = (union task_union *)init_task;

    tss.esp0 = KERNEL_ESP(init_union);
    writeMSR(0x175, (unsigned long)tss.esp0);
    set_cr3(init_task->dir_pages_baseAddr);
}

void init_queues() {
    INIT_LIST_HEAD(&freequeue);
    INIT_LIST_HEAD(&readyqueue);
    INIT_LIST_HEAD(&blockedqueue);

    /* Initialize free queue with all available task structures */
    for (int i = 0; i < NR_TASKS; ++i) {
        list_add_tail(&tasks[i].task.list, &freequeue);
    }
}

void init_sched() {
    init_queues();
}

struct task_struct *current() {
    int ret_value;

    __asm__ __volatile__("movl %%esp, %0" : "=g"(ret_value));
    return (struct task_struct *)(ret_value & 0xfffff000);
}

void inner_task_switch(union task_union *new) {
    struct task_struct *old_task = current();
    current_task = &new->task; /* (Optimization) Update global current_task pointer */

#if DEBUG_INFO_TASK_SWITCH
    printDebugInfoSched(old_task->PID, new->task.PID);
#endif

    /* Update TSS stack pointer for the new task */
    tss.esp0 = KERNEL_ESP(new);
    writeMSR(0x175, (DWord)tss.esp0);

    /* Switch to the new task's page directory */
    set_cr3(get_DIR(&new->task));

    /* Perform context switch, execution continues in the new process */
    switch_context(&old_task->kernel_esp, new->task.kernel_esp);
}

int get_next_pid(void) {
    return ++next_pid;
}

int get_quantum(struct task_struct *task) {
    return task->quantum;
}

void set_quantum(struct task_struct *task, int new_quantum) {
    task->quantum = new_quantum;
}

void update_sched_data_rr(void) {
    current_quantum--;
}

int needs_sched_rr(void) {
    if (current_quantum <= 0) {
        current_quantum = get_quantum(current_task);
        // Need to switch if there are ready processes and quantum expired
        if (!list_empty(&readyqueue)) {
            return 1;
        }
    }
    // Need to switch if the current process is blocked
    return (current_task->status == ST_BLOCKED);
}

void update_process_state_rr(struct task_struct *task, struct list_head *dest_queue) {
    struct list_head *task_list = &task->list;

    // Remove from current queue only if not running
    if (task->status != ST_RUN) {
        list_del(task_list);
    }

    // Update state and queue based on destination
    if (dest_queue != NULL) {
        list_add_tail(task_list, dest_queue);
        task->status = (dest_queue == &readyqueue) ? ST_READY : ST_BLOCKED;
    } else {
        // No destination queue means the task is now running
        task->status = ST_RUN;
    }
}

void sched_next_rr(void) {
    struct list_head *next;
    struct task_struct *next_task;

    /* Select next process: from ready queue if possible, idle process otherwise */
    if (!list_empty(&readyqueue)) {

        next = list_first(&readyqueue);
        next_task = list_head_to_task_struct(next);
        // Remove from the ready queue
        update_process_state_rr(next_task, NULL);

    } else {

        // Switch to idle task if no processes are ready
        next_task = idle_task;
    }

    // Reset quantum counter for the next process
    current_quantum = next_task->quantum;
    task_switch((union task_union *)next_task);
}

void scheduler(void) {
    update_sched_data_rr();

    if (needs_sched_rr()) {
        update_process_state_rr(current_task, &readyqueue);
        sched_next_rr();
    }
}

/* ---- Test functions ---- */

void printDebugInfoSched(int from_pid, int to_pid) {
    char buffer[12];

    printk_color("[SCHED] switch from PID ", INFO_COLOR);
    itoa(from_pid, buffer);
    printk_color(buffer, INFO_COLOR);
    printk_color(" to PID ", INFO_COLOR);
    itoa(to_pid, buffer);
    printk_color(buffer, INFO_COLOR);
    printk_color(" and ready: ", INFO_COLOR);

    if (list_empty(&readyqueue)) {
        printk_color("empty\n", WARNING_COLOR);
    } else {
        printk_color("NOT empty\n", INFO_COLOR);
    }
}
