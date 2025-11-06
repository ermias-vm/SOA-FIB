#ifndef STATS_H
#define STATS_H

/* Process statistics structure for performance monitoring */
struct stats {
    /* Time spent executing in user mode */
    unsigned long user_ticks;

    /* Time spent executing in system/kernel mode */
    unsigned long system_ticks;

    /* Time spent blocked waiting for resources */
    unsigned long blocked_ticks;

    /* Time spent in ready queue waiting for CPU */
    unsigned long ready_ticks;

    /* Total elapsed time since process creation */
    unsigned long elapsed_total_ticks;

    /* Number of times the process got the CPU (READY->RUN transitions) */
    unsigned long total_trans;

    /* Remaining quantum ticks for current execution */
    unsigned long remaining_ticks;
};
#endif /* !STATS_H */
