/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

void *get_ebp();

int check_fd(int fd, int permissions) {
    if (fd != 1) return -EBADF;
    if (permissions != ESCRIPTURA) return -EACCES;
    return 0;
}

void user_to_system(void) {
    update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void) {
    update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall() {
    return -ENOSYS;
}

int sys_getpid() {
    return current()->PID;
}

int global_PID = 1000;

int ret_from_fork() {
    return 0;
}

int sys_fork(void) {
    struct list_head *lhcurrent = NULL;
    union task_union *uchild;

    /* Any free task_struct? */
    if (list_empty(&freequeue)) return -ENOMEM;

    lhcurrent = list_first(&freequeue);

    list_del(lhcurrent);

    uchild = (union task_union *)list_head_to_task_struct(lhcurrent);

    /* Copy the parent's task struct to child's */
    copy_data(current(), uchild, sizeof(union task_union));

    /* new pages dir */
    allocate_DIR((struct task_struct *)uchild);

    /* Allocate pages for DATA+STACK */
    int new_ph_pag, pag, i;
    page_table_entry *process_PT = get_PT(&uchild->task);
    for (pag = 0; pag < NUM_PAG_DATA; pag++) {
        new_ph_pag = alloc_frame();
        if (new_ph_pag != -1) /* One page allocated */
        {
            set_ss_pag(process_PT, PAG_LOG_INIT_DATA + pag, new_ph_pag);
        } else /* No more free pages left. Deallocate everything */
        {
            /* Deallocate allocated pages. Up to pag. */
            for (i = 0; i < pag; i++) {
                free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
                del_ss_pag(process_PT, PAG_LOG_INIT_DATA + i);
            }
            /* Deallocate task_struct */
            list_add_tail(lhcurrent, &freequeue);

            /* Return error */
            return -EAGAIN;
        }
    }

    /* Copy parent's SYSTEM and CODE to child. */
    page_table_entry *parent_PT = get_PT(current());
    for (pag = 0; pag < NUM_PAG_KERNEL; pag++) {
        set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
    }
    for (pag = 0; pag < NUM_PAG_CODE; pag++) {
        set_ss_pag(process_PT, PAG_LOG_INIT_CODE + pag,
                   get_frame(parent_PT, PAG_LOG_INIT_CODE + pag));
    }
    /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
    for (pag = NUM_PAG_KERNEL + NUM_PAG_CODE; pag < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA;
         pag++) {
        /* Map one child page to parent's address space. */
        set_ss_pag(parent_PT, pag + NUM_PAG_DATA, get_frame(process_PT, pag));
        copy_data((void *)(pag << 12), (void *)((pag + NUM_PAG_DATA) << 12), PAGE_SIZE);
        del_ss_pag(parent_PT, pag + NUM_PAG_DATA);
    }
    /* Deny access to the child's memory space */
    set_cr3(get_DIR(current()));

    uchild->task.PID = ++global_PID;
    uchild->task.state = ST_READY;

    int register_ebp; /* frame pointer */
    /* Map Parent's ebp to child's stack */
    register_ebp = (int)get_ebp();
    register_ebp = (register_ebp - (int)current()) + (int)(uchild);

    uchild->task.register_esp = register_ebp + sizeof(DWord);

    DWord temp_ebp = *(DWord *)register_ebp;
    /* Prepare child stack for context switch */
    uchild->task.register_esp -= sizeof(DWord);
    *(DWord *)(uchild->task.register_esp) = (DWord)&ret_from_fork;
    uchild->task.register_esp -= sizeof(DWord);
    *(DWord *)(uchild->task.register_esp) = temp_ebp;

    /* Set stats to 0 */
    init_stats(&(uchild->task.p_stats));

    /* Queue child process into readyqueue */
    uchild->task.state = ST_READY;
    list_add_tail(&(uchild->task.list), &readyqueue);

    return uchild->task.PID;
}

#define TAM_BUFFER 512

char temp_buff[24];
int sys_write(int fd, char *buffer, int nbytes) {
    int ret;

    if ((ret = check_fd(fd, ESCRIPTURA))) return ret;
    if (nbytes < 0) return -EINVAL;
    if (!access_ok(VERIFY_READ, buffer, nbytes)) return -EFAULT;

    char *kernel_buffer = (char *)0x3FF000; // Dirección lógica fija para zero copy
    page_table_entry *PT = get_PT(current());
    int page_buffer = ((unsigned int)buffer) >> 12;
    int frame = get_frame(PT, page_buffer); // Pagina fisica de buffer
    if (frame < 1) return -EFAULT;
    printk("\n------------Frame inicial: ");
    itoa(frame, temp_buff);
    printk(temp_buff);
    printk(": ");
    set_ss_pag(PT, 0x3FF, frame); // Mapear frame a 0x3FF000

    int bytes_left = nbytes;
    int written_bytes = 0;

    while (bytes_left > 0) {
        int to_write = (bytes_left > TAM_BUFFER) ? TAM_BUFFER : bytes_left;
        written_bytes = sys_write_console(kernel_buffer, to_write);
        bytes_left -= written_bytes;
        kernel_buffer += written_bytes;

        // Si se alcanza el final de la página mapeada, remapear la siguiente página de usuario
        if (kernel_buffer >= (char *)0x400000) {
            printk("\n------------REMAPAEO A NUEVA PAGINA------------\n");
            del_ss_pag(PT, 0x3FF);                  // Desmapear la página actual
            frame = get_frame(PT, (++page_buffer)); // Obtener frame de la nueva página
            if (frame < 1) return -EFAULT;
            set_ss_pag(PT, 0x3FF, frame);     // Mapear la nueva página
            kernel_buffer = (char *)0x3FF000; // Resetear kernel_buffer
        }
    }
    printk("\n------------FIN DE ESCRITURA------------\n");
    del_ss_pag(PT, 0x3FF); // Desmapear

    return nbytes - bytes_left; // Retornar total escrito
}

extern int zeos_ticks;

int sys_gettime() {
    return zeos_ticks;
}

void sys_exit() {
    int i;

    page_table_entry *process_PT = get_PT(current());

    // Deallocate all the propietary physical pages
    for (i = 0; i < NUM_PAG_DATA; i++) {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA + i);
    }

    /* Free task_struct */
    list_add_tail(&(current()->list), &freequeue);

    current()->PID = -1;

    /* Restarts execution of the next process */
    sched_next_rr();
}

/* System call to force a task switch */
int sys_yield() {
    force_task_switch();
    return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st) {
    int i;

    if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;

    if (pid < 0) return -EINVAL;
    for (i = 0; i < NR_TASKS; i++) {
        if (task[i].task.PID == pid) {
            task[i].task.p_stats.remaining_ticks = remaining_quantum;
            copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
            return 0;
        }
    }
    return -ESRCH; /*ESRCH */
}
