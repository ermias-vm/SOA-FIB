/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <interrupt.h>
#include <io.h>
#include <libc.h>
#include <mm.h>
#include <sched.h>
#include <segment.h>
#include <utils.h>

/* Global array of all task unions in the system */
union task_union task[NR_TASKS] __attribute__((__section__(".data.task")));

/* Global PID counter for assigning unique process identifiers */
static int next_pid = 1;

struct task_struct *list_head_to_task_struct(struct list_head *l) {
    return list_entry(l, struct task_struct, list);
}

struct list_head freequeue;
struct list_head readyqueue;
struct list_head blockedqueue;

/* Current quantum ticks remaining for the running process */
static int current_quantum = 0;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry *get_DIR(struct task_struct *t) {
    return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry *get_PT(struct task_struct *t) {
    return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr)) << 12);
}

int allocate_DIR(struct task_struct *t) {
    int pos;

    pos = ((int)t - (int)task) / sizeof(union task_union);

    t->dir_pages_baseAddr = (page_table_entry *)&dir_pages[pos];

    return 1;
}

void cpu_idle(void) {
    __asm__ __volatile__("sti" : : : "memory");

    while (1) {
        ;
    }
}

/* Pointer to the idle task (PID 0) - runs when no other process is ready */
struct task_struct *idle_task;

/* Pointer to the initial user task (PID 1) - the first user process */
struct task_struct *init_task;

void init_idle(void) {
    struct list_head *free_task = freequeue.next;
    list_del(free_task);

    idle_task = list_head_to_task_struct(free_task);
    idle_task->PID = 0;

    /* Initialize q*/
    idle_task->quantum = DEFAULT_QUANTUM;

    /* Initialize status */
    idle_task->status = ST_READY;

    /* Initialize process hierarchy */
    idle_task->parent = NULL;
    INIT_LIST_HEAD(&idle_task->children);
    INIT_LIST_HEAD(&idle_task->child_list);

    /* Initialize blocking mechanism */
    idle_task->pending_unblocks = 0;

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

    /* Initialize status */
    init_task->status = ST_RUN;

    /* Initialize process hierarchy */
    init_task->parent = NULL;
    INIT_LIST_HEAD(&init_task->children);
    INIT_LIST_HEAD(&init_task->child_list);

    /* Initialize blocking mechanism */
    init_task->pending_unblocks = 0;

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
    for (int i = 0; i < NR_TASKS; ++i) {
        list_add_tail(&task[i].task.list, &freequeue);
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
    tss.esp0 = KERNEL_ESP(new);
    writeMSR(0x175, (int)tss.esp0);
    set_cr3(get_DIR(&new->task));
    switch_context(&current()->kernel_esp, new->task.kernel_esp);
}

int get_next_pid(void) {
    return ++next_pid; // Pre-increment to get: 2, 3, 4, etc.
}

/* Quantum management functions */
int get_quantum(struct task_struct *t) {
    return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum) {
    t->quantum = new_quantum;
}

/* Round-robin scheduling interface implementation */
void update_sched_data_rr(void) {
    current_quantum--;
}

int needs_sched_rr(void) {
    if ((current_quantum <= 0 && !list_empty(&readyqueue)) || (current()->status == ST_BLOCKED)) {
        return 1;
    } else {
        return 0;
    }
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue) {
    /* If process is not RUNNING, remove from current queue */
    if (!list_empty(&t->list)) {
        list_del(&t->list);
    }

    /* If destination is not NULL, add to new queue */
    if (dst_queue != NULL) {
        list_add_tail(&t->list, dst_queue);
    }
}

void sched_next_rr(void) {
    if (list_empty(&readyqueue)) {
        /* No processes in ready queue, switch to idle if not already */
        printk_color("SCHED: Ready queue empty, switching to idle\n", WARNING_COLOR);
        if (current() != idle_task) {
            idle_task->status = ST_RUN;
            current_quantum = get_quantum(idle_task);
            task_switch((union task_union *)idle_task);
        } else {
            /* Already idle, just reset quantum */
            current_quantum = get_quantum(idle_task);
            printk_color("SCHED: Already idle, reset quantum\n", INFO_COLOR);
        }
        return;
    }

    /* Get next process from ready queue */
    struct list_head *next_node = list_first(&readyqueue);
    list_del(next_node);
    struct task_struct *next_task = list_head_to_task_struct(next_node);

    /* Update status and restore quantum for the new process */
    next_task->status = ST_RUN;
    current_quantum = get_quantum(next_task);

    /* Simple debug message */
    char buffer[12];
    printk_color("SCHED: ", INFO_COLOR);
    itoa(current()->PID, buffer);
    printk_color(buffer, INFO_COLOR);
    printk_color("->", INFO_COLOR);
    itoa(next_task->PID, buffer);
    printk_color(buffer, INFO_COLOR);
    printk_color("\n", INFO_COLOR);

    /* Switch to new process */
    task_switch((union task_union *)next_task);
}

void scheduler(void) {
    /* Update scheduling data - decrease global quantum variable */
    update_sched_data_rr();

    /* Simple debug every 100 ticks */
    static int debug_counter = 0;
    debug_counter++;
    if (debug_counter % 100 == 0) {
        char buffer[12];
        printk_color("TICK: current_quantum=", INFO_COLOR);
        itoa(current_quantum, buffer);
        printk_color(buffer, INFO_COLOR);
        printk_color(" PID=", INFO_COLOR);
        itoa(current()->PID, buffer);
        printk_color(buffer, INFO_COLOR);
        printk_color(" ready_empty=", INFO_COLOR);
        itoa(list_empty(&readyqueue), buffer);
        printk_color(buffer, INFO_COLOR);
        printk_color("\n", INFO_COLOR);
    }

    /* Decide if a context switch is required */
    if (!needs_sched_rr()) {
        return;
    }

    printk_color("SCHED: Context switch needed, current_quantum=", WARNING_COLOR);
    char buffer[12];
    itoa(current_quantum, buffer);
    printk_color(buffer, WARNING_COLOR);
    printk_color(" PID=", WARNING_COLOR);
    itoa(current()->PID, buffer);
    printk_color(buffer, WARNING_COLOR);
    printk_color("\n", WARNING_COLOR);

    /* Context switch is required */
    struct task_struct *current_task = current();

    /* Update the readyqueue, only if current process is not idle and not blocked */
    if (current_task != idle_task && current_task->status != ST_BLOCKED) {
        current_task->status = ST_READY;
        update_process_state_rr(current_task, &readyqueue);
    }

    /* Extract the first process of the readyqueue and perform context switch */
    sched_next_rr();
}
