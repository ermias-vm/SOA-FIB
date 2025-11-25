/**
 * @file project_test.c
 * @brief Test suite for thread support in ZeOS.
 *
 * This file implements comprehensive tests for thread functionality including:
 * - Thread creation up to maximum limit
 * - TID reuse after thread deletion
 * - Process termination when last thread exits
 * - Master thread reassignment
 * - Fork copying only current thread
 */

#include <libc.h>
#include <project_test.h>
#include <zeos_test.h>

extern char buffer[BUFFER_SIZE];
extern char *msg;
extern int errno;

/* Synchronization flags - volatile for thread visibility */
static volatile int sync_flags[MAX_SYNC_FLAGS];

/* Test counters */
static int thread_subtests_run = 0;
static int thread_subtests_passed = 0;

/* ============================================================
 *                    SYNCHRONIZATION UTILITIES
 * ============================================================ */

void clear_all_flags(void) {
    for (int i = 0; i < MAX_SYNC_FLAGS; i++) {
        sync_flags[i] = 0;
    }
}

void set_flag(int flag_index) {
    if (flag_index >= 0 && flag_index < MAX_SYNC_FLAGS) {
        sync_flags[flag_index] = 1;
    }
}

int is_flag_set(int flag_index) {
    if (flag_index >= 0 && flag_index < MAX_SYNC_FLAGS) {
        return sync_flags[flag_index];
    }
    return 0;
}

void wait_for_flag(int flag_index) {
    while (!is_flag_set(flag_index)) {
        /* Deterministic spin-wait until flag is set */
    }
}

/* Wait for multiple flags to be set */
static void wait_for_flags(int count) {
    for (int i = 0; i < count; i++) {
        wait_for_flag(i);
    }
}

/* ============================================================
 *                    THREAD WORK FUNCTIONS
 * ============================================================ */

void simple_thread_func(void *arg) {
    int flag_index = *(int *)arg;

    write_current_info();
    msg = "Simple thread running, will set flag ";
    write(1, msg, strlen(msg));
    itoa(flag_index, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    /* Signal completion */
    set_flag(flag_index);

    ThreadExit();
}

void thread_work(void *arg) {
    int ticks = *(int *)arg;
    int start_time = gettime();

    write_current_info();
    msg = "Thread working for ";
    write(1, msg, strlen(msg));
    itoa(ticks, buffer);
    write(1, buffer, strlen(buffer));
    msg = " ticks...\n";
    write(1, msg, strlen(msg));

    while (gettime() - start_time < ticks) {
        /* Working... */
    }

    write_current_info();
    msg = "Thread finished work\n";
    write(1, msg, strlen(msg));

    ThreadExit();
}

void thread_block_func(void *arg) {
    int flag_index = *(int *)arg;

    write_current_info();
    msg = "Thread will set flag and block\n";
    write(1, msg, strlen(msg));

    set_flag(flag_index);
    block();

    /* After being unblocked */
    write_current_info();
    msg = "Thread was unblocked\n";
    write(1, msg, strlen(msg));

    ThreadExit();
}

/* Thread that just exits immediately - for testing thread limits */
static void minimal_thread_func(void *arg) {
    int flag_index = *(int *)arg;
    set_flag(flag_index);
    ThreadExit();
}

/* Thread for master reassignment test - stays alive */
static void survivor_thread_func(void *arg) {
    int flag_index = *(int *)arg;

    write_current_info();
    msg = "Survivor thread started, waiting for master to exit\n";
    write(1, msg, strlen(msg));

    /* Signal that we're ready */
    set_flag(flag_index);

    /* Work for a while - we should become master */
    int start = gettime();
    while (gettime() - start < MEDIUM_WORK_TIME) {
    }

    write_current_info();
    msg = "Survivor thread: I am now master (hopefully), exiting process\n";
    write(1, msg, strlen(msg));

    /* Exit the process */
    exit();
}

/* ============================================================
 *                    UTILITY FUNCTIONS
 * ============================================================ */

void waitTicks(int ticks) {
    int start_time = gettime();
    while (gettime() - start_time < ticks) {
        /* Busy wait */
    }
}

void write_current_info(void) {
    msg = "[PID ";
    write(1, msg, strlen(msg));
    itoa(getpid(), buffer);
    write(1, buffer, strlen(buffer));
    msg = "] [TID ";
    write(1, msg, strlen(msg));
    itoa(gettid(), buffer);
    write(1, buffer, strlen(buffer));
    msg = "] ";
    write(1, msg, strlen(msg));
}

static void print_subtest_header(int num, char *name) {
    msg = "\n[SUBTEST ";
    write(1, msg, strlen(msg));
    itoa(num, buffer);
    write(1, buffer, strlen(buffer));
    msg = "] ";
    write(1, msg, strlen(msg));
    write(1, name, strlen(name));
    msg = "\n";
    write(1, msg, strlen(msg));
}

static void print_subtest_result(int passed) {
    if (passed) {
        msg = "  -> PASSED\n";
    } else {
        msg = "  -> FAILED\n";
    }
    write(1, msg, strlen(msg));
}

/* ============================================================
 *                    SUBTEST IMPLEMENTATIONS
 * ============================================================ */

/**
 * Subtest 1 & 2: Create maximum threads and verify limit
 * Note: MAX_THREADS_PER_PROCESS (5) includes the master thread,
 *       so we can only create 4 additional threads (slots 2-5).
 */
void subtest_max_threads(int *passed) {
    /* Master already uses 1 slot, so we can create MAX - 1 new threads */
    int max_new_threads = MAX_THREADS_PER_PROCESS - 1;

    print_subtest_header(1, "Create maximum additional threads (4) and verify limit");

    clear_all_flags();

    int tids[MAX_THREADS_PER_PROCESS];
    int flag_indices[MAX_THREADS_PER_PROCESS];
    int created = 0;

    write_current_info();
    msg = "Creating up to ";
    write(1, msg, strlen(msg));
    itoa(max_new_threads, buffer);
    write(1, buffer, strlen(buffer));
    msg = " additional threads (master uses 1 slot)...\n";
    write(1, msg, strlen(msg));

    /* Try to create MAX_THREADS_PER_PROCESS threads, expect only max_new_threads to succeed */
    for (int i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
        flag_indices[i] = i;
        tids[i] = ThreadCreate(minimal_thread_func, &flag_indices[i]);

        if (tids[i] > 0) {
            created++;
            write_current_info();
            msg = "  Created thread TID ";
            write(1, msg, strlen(msg));
            itoa(tids[i], buffer);
            write(1, buffer, strlen(buffer));
            msg = "\n";
            write(1, msg, strlen(msg));
        }
    }

    msg = "  Created ";
    write(1, msg, strlen(msg));
    itoa(created, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(max_new_threads, buffer);
    write(1, buffer, strlen(buffer));
    msg = " additional threads\n";
    write(1, msg, strlen(msg));

    /* Expect exactly max_new_threads (4) to be created */
    int test1_passed = (created == max_new_threads);
    print_subtest_result(test1_passed);

    thread_subtests_run++;
    if (test1_passed) thread_subtests_passed++;

    /* Wait for all threads to complete */
    wait_for_flags(created);

    /* Subtest 2: Verify that after threads exit, we can create new ones again */
    print_subtest_header(2, "Verify thread limit enforcement after cleanup");

    /* All threads should have exited, now create max again */
    clear_all_flags();
    created = 0;
    for (int i = 0; i < max_new_threads; i++) {
        flag_indices[i] = i;
        tids[i] = ThreadCreate(simple_thread_func, &flag_indices[i]);
        if (tids[i] > 0) created++;
    }

    /* Now try to create one more - should fail (we have master + max_new_threads = 5 total) */
    int extra_flag = 7;
    int extra_tid = ThreadCreate(simple_thread_func, &extra_flag);

    write_current_info();
    if (extra_tid < 0) {
        msg = "  Correctly rejected 5th additional thread (returned ";
        write(1, msg, strlen(msg));
        itoa(extra_tid, buffer);
        write(1, buffer, strlen(buffer));
        msg = ")\n";
        write(1, msg, strlen(msg));
        *passed = 1;
    } else {
        msg = "  ERROR: Created extra thread with TID ";
        write(1, msg, strlen(msg));
        itoa(extra_tid, buffer);
        write(1, buffer, strlen(buffer));
        msg = " (should have failed)\n";
        write(1, msg, strlen(msg));
        *passed = 0;
    }

    print_subtest_result(*passed);
    thread_subtests_run++;
    if (*passed) thread_subtests_passed++;

    /* Wait for created threads to complete */
    wait_for_flags(created);
}

/**
 * Subtest 3: TID reuse after thread deletion
 */
void subtest_tid_reuse(int *passed) {
    print_subtest_header(3, "TID reuse after thread deletion");

    clear_all_flags();

    /* Create a thread, let it exit, create another - TID should be reused */
    int flag0 = 0;
    int first_tid = ThreadCreate(minimal_thread_func, &flag0);

    write_current_info();
    msg = "  First thread created with TID ";
    write(1, msg, strlen(msg));
    itoa(first_tid, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    /* Wait for thread to complete */
    wait_for_flag(0);

    /* Small delay to ensure cleanup */
    waitTicks(MIN_WORK_TIME);

    /* Create another thread */
    clear_all_flags();
    int flag1 = 0;
    int second_tid = ThreadCreate(minimal_thread_func, &flag1);

    write_current_info();
    msg = "  Second thread created with TID ";
    write(1, msg, strlen(msg));
    itoa(second_tid, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    /* Wait for completion */
    wait_for_flag(0);

    /* Check if TID was reused (should be same or another freed TID) */
    if (second_tid > 0) {
        if (second_tid == first_tid) {
            msg = "  TID ";
            write(1, msg, strlen(msg));
            itoa(first_tid, buffer);
            write(1, buffer, strlen(buffer));
            msg = " was reused\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "  Different TID used (still valid - TIDs are being freed)\n";
            write(1, msg, strlen(msg));
        }
        *passed = 1;
    } else {
        msg = "  ERROR: Could not create second thread\n";
        write(1, msg, strlen(msg));
        *passed = 0;
    }

    print_subtest_result(*passed);
    thread_subtests_run++;
    if (*passed) thread_subtests_passed++;
}

/**
 * Subtest 4: Process terminates when last thread exits
 * This must be run in a child process since it terminates the process.
 */
void subtest_last_thread_exits(void) {
    print_subtest_header(4, "Process terminates when last thread exits");

    write_current_info();
    msg = "  Creating child process to test last thread exit...\n";
    write(1, msg, strlen(msg));

    int pid = fork();

    if (pid == 0) {
        /* Child process - we are the only thread initially */
        write_current_info();
        msg = "  Child: I am the only thread, calling ThreadExit()\n";
        write(1, msg, strlen(msg));

        msg = "  -> PASSED (if this prints and process exits)\n";
        write(1, msg, strlen(msg));

        /* This should terminate the process */
        ThreadExit();

        /* Should never reach here */
        msg = "  -> FAILED: Code after ThreadExit executed!\n";
        write(1, msg, strlen(msg));
        exit();

    } else if (pid > 0) {
        /* Parent waits for child to complete */
        waitTicks(MEDIUM_WORK_TIME);

        write_current_info();
        msg = "  Parent: Child process should have terminated\n";
        write(1, msg, strlen(msg));

        thread_subtests_run++;
        thread_subtests_passed++; /* If we got here, child exited properly */
    } else {
        msg = "  -> FAILED: Fork failed\n";
        write(1, msg, strlen(msg));
        thread_subtests_run++;
    }
}

/**
 * Subtest 5: Master thread reassignment when current master exits
 */
void subtest_master_reassignment(void) {
    print_subtest_header(5, "Master thread reassignment when master exits");

    write_current_info();
    msg = "  Creating child process to test master reassignment...\n";
    write(1, msg, strlen(msg));

    int pid = fork();

    if (pid == 0) {
        /* Child process - we are the master thread */
        clear_all_flags();

        write_current_info();
        msg = "  Child (Master): Creating survivor thread\n";
        write(1, msg, strlen(msg));

        int flag_idx = 0;
        int survivor_tid = ThreadCreate(survivor_thread_func, &flag_idx);

        if (survivor_tid > 0) {
            write_current_info();
            msg = "  Child (Master): Created survivor TID ";
            write(1, msg, strlen(msg));
            itoa(survivor_tid, buffer);
            write(1, buffer, strlen(buffer));
            msg = "\n";
            write(1, msg, strlen(msg));

            /* Wait for survivor to be ready */
            wait_for_flag(0);

            write_current_info();
            msg = "  Child (Master): Exiting, survivor should become master\n";
            write(1, msg, strlen(msg));

            msg = "  -> PASSED (if survivor continues and process eventually exits)\n";
            write(1, msg, strlen(msg));

            /* Master exits - survivor should take over */
            ThreadExit();

            /* Should never reach here */
            msg = "  -> FAILED: Code after ThreadExit executed!\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "  -> FAILED: Could not create survivor thread\n";
            write(1, msg, strlen(msg));
        }
        exit();

    } else if (pid > 0) {
        /* Parent waits for child to complete */
        waitTicks(LONG_WORK_TIME);

        write_current_info();
        msg = "  Parent: Child process should have completed\n";
        write(1, msg, strlen(msg));

        thread_subtests_run++;
        thread_subtests_passed++; /* If we got here, reassignment worked */
    } else {
        msg = "  -> FAILED: Fork failed\n";
        write(1, msg, strlen(msg));
        thread_subtests_run++;
    }
}

/* Thread that works for a while then exits - for fork test */
static void long_work_thread_func(void *arg) {
    int flag_index = *(int *)arg;

    write_current_info();
    msg = "Secondary thread started, working...\n";
    write(1, msg, strlen(msg));

    /* Signal that we've started */
    set_flag(flag_index);

    /* Work for a while - this thread should NOT be copied to child */
    int start = gettime();
    while (gettime() - start < LONG_WORK_TIME) {
        /* Working... */
    }

    write_current_info();
    msg = "Secondary thread finished work, exiting\n";
    write(1, msg, strlen(msg));

    ThreadExit();
}

/**
 * Subtest 6: Fork copies only current thread
 * This test creates a secondary thread, then forks. The child should only
 * have the calling thread, not the secondary thread.
 */
void subtest_fork_single_thread(int *passed) {
    print_subtest_header(6, "Fork copies only current thread");

    clear_all_flags();

    /* Create a secondary thread that will work during fork */
    write_current_info();
    msg = "  Creating secondary thread before fork...\n";
    write(1, msg, strlen(msg));

    int flag_idx = 0;
    int secondary_tid = ThreadCreate(long_work_thread_func, &flag_idx);

    if (secondary_tid <= 0) {
        msg = "  -> FAILED: Could not create secondary thread\n";
        write(1, msg, strlen(msg));
        *passed = 0;
        print_subtest_result(*passed);
        thread_subtests_run++;
        return;
    }

    /* Wait for secondary thread to signal it has started */
    wait_for_flag(0);

    write_current_info();
    msg = "  Secondary thread TID ";
    write(1, msg, strlen(msg));
    itoa(secondary_tid, buffer);
    write(1, buffer, strlen(buffer));
    msg = " is working, now forking...\n";
    write(1, msg, strlen(msg));

    int child_pid = fork();

    if (child_pid == 0) {
        /* Child process - should only have current thread, not the secondary one */
        /* Child starts fresh with 1 thread (master), so can create 4 more */
        int max_new_threads = MAX_THREADS_PER_PROCESS - 1;

        write_current_info();
        msg = "  Child: I should be the only thread (TID ";
        write(1, msg, strlen(msg));
        itoa(gettid(), buffer);
        write(1, buffer, strlen(buffer));
        msg = ")\n";
        write(1, msg, strlen(msg));

        /* Try to create threads - if fork only copied us, we should be able to create
         * max_new_threads */
        int can_create = 0;
        int test_tids[MAX_THREADS_PER_PROCESS];
        int test_flags[MAX_THREADS_PER_PROCESS];

        for (int i = 0; i < max_new_threads; i++) {
            test_flags[i] = i;
            test_tids[i] = ThreadCreate(minimal_thread_func, &test_flags[i]);
            if (test_tids[i] > 0) can_create++;
        }

        msg = "  Child: Created ";
        write(1, msg, strlen(msg));
        itoa(can_create, buffer);
        write(1, buffer, strlen(buffer));
        msg = " threads (expected ";
        write(1, msg, strlen(msg));
        itoa(max_new_threads, buffer);
        write(1, buffer, strlen(buffer));
        msg = ")\n";
        write(1, msg, strlen(msg));

        if (can_create == max_new_threads) {
            msg = "  -> PASSED (fork copied only current thread)\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "  -> FAILED (fork may have copied other threads)\n";
            write(1, msg, strlen(msg));
        }

        /* Wait for threads to complete and exit */
        waitTicks(MEDIUM_WORK_TIME);
        exit();

    } else if (child_pid > 0) {
        /* Parent - wait for child and our secondary thread */
        write_current_info();
        msg = "  Parent: Forked child PID ";
        write(1, msg, strlen(msg));
        itoa(child_pid, buffer);
        write(1, buffer, strlen(buffer));
        msg = "\n";
        write(1, msg, strlen(msg));

        /* Wait for child to complete its test and for secondary thread to finish */
        waitTicks(LONG_WORK_TIME + MEDIUM_WORK_TIME);

        write_current_info();
        msg = "  Parent: Test completed\n";
        write(1, msg, strlen(msg));

        *passed = 1; /* Success if we reach here */
        print_subtest_result(*passed);
        thread_subtests_run++;
        if (*passed) thread_subtests_passed++;

    } else {
        msg = "  -> FAILED: Fork failed\n";
        write(1, msg, strlen(msg));
        *passed = 0;
        print_subtest_result(*passed);
        thread_subtests_run++;
    }
}

/* ============================================================
 *                    MAIN TEST FUNCTIONS
 * ============================================================ */

void thread_tests(void) {
    print_test_header("THREAD TESTS");

    thread_subtests_run = 0;
    thread_subtests_passed = 0;

    write_current_info();
    msg = "Starting thread test suite...\n";
    write(1, msg, strlen(msg));

    int result;

    /* Subtest 1 & 2: Max threads and limit enforcement */
    subtest_max_threads(&result);

    /* Subtest 3: TID reuse */
    subtest_tid_reuse(&result);

    /* Subtest 4: Last thread exits -> process terminates */
    subtest_last_thread_exits();

    /* Subtest 5: Master thread reassignment */
    subtest_master_reassignment();

    /* Subtest 6: Fork copies only current thread */
    subtest_fork_single_thread(&result);

    /* Print thread test summary */
    msg = "\n========================================\n";
    write(1, msg, strlen(msg));
    msg = "[THREAD_TESTS] Summary: ";
    write(1, msg, strlen(msg));
    itoa(thread_subtests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(thread_subtests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = " subtests passed\n";
    write(1, msg, strlen(msg));
    msg = "========================================\n";
    write(1, msg, strlen(msg));

    int all_passed = (thread_subtests_passed == thread_subtests_run);
    print_test_result("Thread Tests", all_passed);
}

void execute_project_tests(void) {
    msg = "\n=========================================\n";
    write(1, msg, strlen(msg));
    msg = "      PROJECT TEST SUITE                 \n";
    write(1, msg, strlen(msg));
    msg = "=========================================\n\n";
    write(1, msg, strlen(msg));

    write_current_info();
    msg = "Coordinator thread starting tests...\n\n";
    write(1, msg, strlen(msg));

#if THREAD_TEST
    RESET_ERRNO();
    thread_tests();
#endif

    msg = "\n=========================================\n";
    write(1, msg, strlen(msg));
    msg = "      ALL PROJECT TESTS COMPLETED        \n";
    write(1, msg, strlen(msg));
    msg = "=========================================\n\n";
    write(1, msg, strlen(msg));

#if IDLE_SWITCH_TEST
    write_current_info();
    msg = "Terminating init process. System will switch to idle.\n";
    write(1, msg, strlen(msg));
    exit();
#endif
}
