/**
 * @file stats.h
 * @brief Process statistics and performance monitoring definitions for ZeOS.
 *
 * This header defines structures for tracking process execution statistics,
 * timing information, and performance metrics in the ZeOS kernel.
 */

#ifndef __STATS_H__
#define __STATS_H__

/* Process statistics structure for performance monitoring */
struct stats {
    unsigned long user_ticks;          /* Time spent executing in user mode */
    unsigned long system_ticks;        /* Time spent executing in system/kernel mode */
    unsigned long blocked_ticks;       /* Time spent blocked waiting for resources */
    unsigned long ready_ticks;         /* Time spent in ready queue waiting for CPU */
    unsigned long elapsed_total_ticks; /* Total elapsed time since process creation */
    unsigned long
        total_trans; /* Number of times the process got the CPU (READY->RUN transitions) */
    unsigned long remaining_ticks; /* Remaining quantum ticks for current execution */
};

#endif /* __STATS_H__ */
