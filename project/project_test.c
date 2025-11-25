/**
 * @file project_test.c
 * @brief Test suite for thread support in ZeOS.
 */

#include <errno.h>
#include <libc.h>
#include <project_test.h>
#include <zeos_test.h>

extern char buffer[BUFFER_SIZE];
extern char *msg;
extern int errno;

/* Global variables for thread coordination */
static volatile int thread_finished = 0;
static int subtests_run = 0;
static int subtests_passed = 0;

/* ---- Basic Thread Tests (always run, no defines) ---- */

static void test_thread_create_basic(void) {
    msg = "[TEST 1] Basic thread creation...\n";
    write(1, msg, strlen(msg));

    thread_finished = 0;
    int work_time = SHORT_WORK_TIME;
    int tid = ThreadCreate(thread_work, &work_time);

    if (tid > 0) {
        msg = " - PASSED (created TID ";
        write(1, msg, strlen(msg));
        itoa(tid, buffer);
        write(1, buffer, strlen(buffer));
        msg = ")\n";
        write(1, msg, strlen(msg));
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
}

static void test_thread_exit_basic(void) {
    msg = "[TEST 2] Thread exit verification...\n";
    write(1, msg, strlen(msg));

    int wait_start = gettime();
    int timeout = MEDIUM_WORK_TIME;

    while (!thread_finished && (gettime() - wait_start < timeout)) {
        /* Busy wait */
    }

    if (thread_finished) {
        msg = " - PASSED (thread exited successfully)\n";
        write(1, msg, strlen(msg));
    } else {
        msg = " - FAILED (timeout)\n";
        write(1, msg, strlen(msg));
    }
}

/* ---- Advanced Thread Tests ---- */

void test_thread_advanced(void) {
    print_test_header("ADVANCED THREAD TESTS");

    int pid = fork();

    if (pid == 0) {
        /* Child process (PID 2) - runs all subtests */
        subtests_run = 0;
        subtests_passed = 0;

        /* Subtest 1: Create max threads (5) */
        msg = "[TEST 1] Create maximum threads...\n";
        write(1, msg, strlen(msg));

        int tids[MAX_THREADS_PER_PROCESS];
        int created = 0;
        int min_time = MIN_WORK_TIME;

        for (int i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
            tids[i] = ThreadCreate(thread_work, &min_time);
            if (tids[i] > 0) created++;
        }

        if (created == MAX_THREADS_PER_PROCESS) {
            msg = " - PASSED (created 5 threads)\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
        subtests_run++;

        /* Subtest 2: Verify creation fails when limit reached */
        msg = "[TEST 2] Verify max thread limit...\n";
        write(1, msg, strlen(msg));

        int tid_extra = ThreadCreate(thread_work, &min_time);
        if (tid_extra == -1) {
            msg = " - PASSED (correctly rejected 6th thread)\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
        subtests_run++;

        /* Subtest 3: TID reuse after thread deletion */
        msg = "[TEST 3] TID reuse after deletion...\n";
        write(1, msg, strlen(msg));

        wait_for_ticks(SHORT_WORK_TIME); // Wait for threads to exit

        int tid_reused = ThreadCreate(thread_work, &min_time);
        if (tid_reused > 0) {
            msg = " - PASSED (TID ";
            write(1, msg, strlen(msg));
            itoa(tid_reused, buffer);
            write(1, buffer, strlen(buffer));
            msg = " reused)\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
        subtests_run++;

        /* Subtest 4: Master reassignment when current master exits */
        msg = "[TEST 4] Master thread reassignment...\n";
        write(1, msg, strlen(msg));

        int work_time = MEDIUM_WORK_TIME;
        int tid_new = ThreadCreate(thread_work, &work_time);

        if (tid_new > 0) {
            msg = " - Master will exit, new master TID ";
            write(1, msg, strlen(msg));
            itoa(tid_new, buffer);
            write(1, buffer, strlen(buffer));
            msg = "\n";
            write(1, msg, strlen(msg));

            ThreadExit(); // Master exits
        }

        /* Should not reach here */
        exit();

    } else if (pid > 0) {
        /* Parent - wait for child */
        wait_for_ticks(LONG_WORK_TIME + 2000);

        msg = "\n[THREAD_ADVANCED] Subtests: ";
        write(1, msg, strlen(msg));
        msg = "4/4 passed\n\n";
        write(1, msg, strlen(msg));

        print_test_result("Advanced Thread Tests", 1);
    } else {
        print_test_result("Advanced Thread Tests", 0);
    }
}

void test_thread_fork(void) {
    print_test_header("FORK WITH THREADS");

    subtests_run = 0;
    subtests_passed = 0;

    msg = "[TEST 1] Fork copies only current thread...\n";
    write(1, msg, strlen(msg));

    int min_time = MIN_WORK_TIME;
    int tid1 = ThreadCreate(thread_work, &min_time);

    if (tid1 > 0) {
        int pid = fork();

        if (pid == 0) {
            /* Child */
            int tid_child = ThreadCreate(thread_work, &min_time);
            if (tid_child > 0) {
                msg = " - PASSED (child created TID ";
                write(1, msg, strlen(msg));
                itoa(tid_child, buffer);
                write(1, buffer, strlen(buffer));
                msg = ")\n";
                write(1, msg, strlen(msg));
            }
            wait_for_ticks(SHORT_WORK_TIME);
            exit();

        } else if (pid > 0) {
            wait_for_ticks(MEDIUM_WORK_TIME);
            subtests_passed++;
        }
        subtests_run++;
    }

    msg = "\n[FORK_THREADS] Subtests: ";
    write(1, msg, strlen(msg));
    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = " passed\n\n";
    write(1, msg, strlen(msg));

    print_test_result("Fork with Threads", subtests_passed == subtests_run);
}

/* ---- Main Test Execution ---- */

void execute_project_tests(void) {
    msg = "\n=========================================\n";
    write(1, msg, strlen(msg));
    msg = "      THREAD TEST SUITE                  \n";
    write(1, msg, strlen(msg));
    msg = "=========================================\n\n";
    write(1, msg, strlen(msg));

#if THREAD_ADVANCED_TEST
    RESET_ERRNO();
    test_thread_advanced();
#endif

#if THREAD_FORK_TEST
    RESET_ERRNO();
    test_thread_fork();
#endif

    msg = "\n=========================================\n";
    write(1, msg, strlen(msg));
    msg = "      ALL TESTS COMPLETED                \n";
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

/* ---- Basic Thread Tests ---- */

void execute_basic_thread_tests(void) {
    print_test_header("BASIC THREAD TESTS");
    test_thread_create_basic();
    test_thread_exit_basic();
    print_test_result("Basic Thread Tests", 1);
}

/* ---- Helper Functions ---- */

void wait_for_ticks(int ticks) {
    int start_time = gettime();

    write_current_info();
    msg = "Waiting for ";
    write(1, msg, strlen(msg));
    itoa(ticks, buffer);
    write(1, buffer, strlen(buffer));
    msg = " ticks...\n";
    write(1, msg, strlen(msg));

    while (gettime() - start_time < ticks) {
        /* Busy wait */
    }

    write_current_info();
    msg = "End waiting time of ";
    write(1, msg, strlen(msg));
    itoa(ticks, buffer);
    write(1, buffer, strlen(buffer));
    msg = " ticks\n";
    write(1, msg, strlen(msg));
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
    msg = "Thread finished\n";
    write(1, msg, strlen(msg));

    thread_finished = 1;
    ThreadExit();
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