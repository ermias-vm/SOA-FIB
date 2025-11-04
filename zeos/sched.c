/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <interrupt.h>
#include <io.h>
#include <mm.h>
#include <sched.h>
#include <segment.h>

union task_union task[NR_TASKS] __attribute__((__section__(".data.task")));

static int next_pid = 1; // Contador global de PIDs

struct task_struct *list_head_to_task_struct(struct list_head *l) {
    return list_entry(l, struct task_struct, list);
}

extern struct list_head blocked;
struct list_head freequeue;
struct list_head readyqueue;

/* Scheduling variables */
static int current_quantum = 0;
#define DEFAULT_QUANTUM 5

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

    idle_to_init_test();
    while (1) {
        ;
    }
}

struct task_struct *idle_task, *init_task;

void init_idle(void) {
    struct list_head *free_task = freequeue.next;
    list_del(free_task);

    idle_task = list_head_to_task_struct(free_task);
    idle_task->PID = 0;

    /* Initialize scheduling fields */
    idle_task->quantum = DEFAULT_QUANTUM;
    idle_task->remaining_ticks = DEFAULT_QUANTUM;

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

    printk_color("Idle task initialized successfully\n", INFO_COLOR);
}

void init_task1(void) {
    struct list_head *free_task = freequeue.next;
    list_del(free_task);

    init_task = list_head_to_task_struct(free_task);

    init_task->PID = 1;

    /* Initialize scheduling fields */
    init_task->quantum = DEFAULT_QUANTUM;
    init_task->remaining_ticks = DEFAULT_QUANTUM;
    current_quantum = DEFAULT_QUANTUM;

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

    printk_color("Task 1 initialized successfully\n", INFO_COLOR);
}

void init_queues() {
    INIT_LIST_HEAD(&freequeue);
    INIT_LIST_HEAD(&readyqueue);
    INIT_LIST_HEAD(&blocked);
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
    return (current_quantum <= 0 && !list_empty(&readyqueue));
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
        /* No processes in ready queue, keep current or switch to idle */
        if (current() != idle_task) {
            current_quantum = get_quantum(idle_task);
            task_switch((union task_union *)idle_task);
        }
        return;
    }

    /* Get next process from ready queue */
    struct list_head *next_node = list_first(&readyqueue);
    list_del(next_node);
    struct task_struct *next_task = list_head_to_task_struct(next_node);

    /* Restore quantum for the new process */
    current_quantum = get_quantum(next_task);

    /* Switch to new process */
    task_switch((union task_union *)next_task);
}

void schedule(void) {
    /* Update scheduling data */
    update_sched_data_rr();

    /* Check if scheduling is needed */
    if (!needs_sched_rr()) {
        return;
    }

    struct task_struct *current_task = current();

    /* If current is not idle, put it back in ready queue */
    if (current_task != idle_task) {
        update_process_state_rr(current_task, &readyqueue);
    }

    /* Schedule next process */
    sched_next_rr();
}

/* TEST FUNCTIONS */

void init_function(void) {
    __asm__ __volatile__("sti" : : : "memory");

    printk_color("\n[INIT_TASK] In init_function\n", INFO_COLOR);
    while (1) {
        ;
    }
}

void idle_to_init_test(void) {
    printk_color("\n[IDLE_TASK] In cpu_idle\n", INFO_COLOR);
    printk_color("[IDLE_TASK] Switching to init task\n", INFO_COLOR);
    task_switch((union task_union *)init_task);
}