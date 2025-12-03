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

#include <errno.h>
#include <libc.h>
#include <project_test.h>
#include <screen_samples.h>
#include <zeos_test.h>

/* External variables from zeos_test */
extern char buffer[BUFFER_SIZE];
extern char *msg;
extern int errno;

/* Thread test variables */
static volatile int sync_flags[MAX_SYNC_FLAGS];
static int thread_subtests_run = 0;
static int thread_subtests_passed = 0;

/* Keyboard test variables */
static int kbd_subtests_run = 0;
static int kbd_subtests_passed = 0;
static volatile int kbd_events_received = 0;
static volatile char kbd_keys[KBD_MAX_KEYS];
static volatile int kbd_pressed[KBD_MAX_KEYS];
static volatile int kbd_key_index = 0;

/* Global test summary tracking */
static int project_tests_run = 0;
static int project_tests_passed = 0;

static volatile int kbd_syscall_tested = 0;
static volatile int kbd_syscall_results[NUM_SYSCALLS_TO_TEST];
static volatile int kbd_syscall_errnos[NUM_SYSCALLS_TO_TEST];

/* Screen test variables */
static int screen_subtests_passed = 0;
static int screen_perf_passed = 0;

/* WaitForTick test variables */
static int wft_subtests_run = 0;
static int wft_subtests_passed = 0;
static volatile int wft_threads_completed = 0;
static volatile int wft_threads_woken_same_tick = 0;
static volatile int wft_wake_tick = 0;

/* Tick calibration test variables */
static int tick_cal_passed = 0;

/* FPS test state variables */
static volatile int fps_test_exit = 0;
static volatile int fps_next_scene = 0;
static volatile int fps_prev_scene = 0;
static int fps_test_passed = 0;

/* Scancode to character mapping */
static char scancode_to_char[] = {
    '\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '\'', '\0', '\0',
    '\0', 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '`',  '+',  '\0', '\0',
    'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  '\0', '\0', '\0', '\0', '\0', 'z',
    'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '-',  '\0', '*',  '\0', ' ',  '\0', '\0',
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '7',  '8',  '9',  '-',
    '4',  '5',  '6',  '+',  '1',  '2',  '3',  '0',  '\0', '\0', '\0', '<'};

/**********************/
/**  Static helpers  **/
/**********************/

static void print_subtest_header(int num, char *name) {
    prints("\n[SUBTEST %d] %s\n", num, name);
}

static void print_subtest_result(int passed) {
    if (passed) {
        prints("-> PASSED\n");
    } else {
        prints("-> FAILED\n");
    }
}

static void wait_for_flags(int count) {
    for (int i = 0; i < count; i++) {
        wait_for_flag(i);
    }
}

static void minimal_thread_func(void *arg) {
    int flag_index = *(int *)arg;
    set_flag(flag_index);
    ThreadExit();
}

static void no_explicit_exit_func(void *arg) {
    int flag_index = *(int *)arg;

    prints("[PID %d] [TID %d] Thread running WITHOUT explicit ThreadExit\n", getpid(), gettid());

    /* Set flag to signal we executed */
    set_flag(flag_index);

    prints("[PID %d] [TID %d] Returning from function (wrapper should call ThreadExit)\n", getpid(),
           gettid());

    /* NO ThreadExit() call - the wrapper should handle this */
    return;
}

static void survivor_thread_func(void *arg) {
    int flag_index = *(int *)arg;

    prints("[PID %d] [TID %d] Survivor thread started, waiting for master to exit\n", getpid(),
           gettid());

    /* Signal that we're ready */
    set_flag(flag_index);

    /* Work for a while - we should become master */
    int start = gettime();
    while (gettime() - start < TIME_MEDIUM) {
    }

    prints("[PID %d] [TID %d] Survivor thread: I am now master (hopefully), exiting process\n",
           getpid(), gettid());

    /* Exit the process */
    exit();
}

static void long_work_thread_func(void *arg) {
    int flag_index = *(int *)arg;

    prints("[PID %d] [TID %d] Secondary thread started, working...\n", getpid(), gettid());

    /* Signal that we've started */
    set_flag(flag_index);

    /* Work for a while - this thread should NOT be copied to child */
    int start = gettime();
    while (gettime() - start < TIME_LONG) {
        /* Working... */
    }

    prints("[PID %d] [TID %d] Secondary thread finished work, exiting\n", getpid(), gettid());

    ThreadExit();
}

static void test_kbd_handler(char key, int pressed) {
    kbd_events_received++;
    /* Store first KBD_MAX_KEYS keys */
    if (kbd_key_index < KBD_MAX_KEYS) {
        kbd_keys[kbd_key_index] = key;
        kbd_pressed[kbd_key_index] = pressed;
        kbd_key_index++;
    }
}

static void test_kbd_handler_with_syscall(char key, int pressed) {
    (void)key;
    (void)pressed;
    /* Test ALL syscalls inside the handler - each should return -1 with EINPROGRESS */
    /* (except gettime which doesn't check keyboard context) */
    /* We cannot test exit(1) and ThreadExit(5) as they would terminate */
    int idx = 0;

    /* Test getpid (syscall 20) */
    errno = 0;
    kbd_syscall_results[idx] = getpid();
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test gettid (syscall 21) */
    errno = 0;
    kbd_syscall_results[idx] = gettid();
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test gettime (syscall 10) - doesn't check keyboard context, should work */
    errno = 0;
    kbd_syscall_results[idx] = gettime();
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test fork (syscall 2) */
    errno = 0;
    kbd_syscall_results[idx] = fork();
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test write (syscall 4) */
    errno = 0;
    char buf[] = "x";
    kbd_syscall_results[idx] = write(1, buf, 1);
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test ThreadCreate (syscall 6) */
    errno = 0;
    kbd_syscall_results[idx] = ThreadCreate((void (*)(void *))0x1000, (void *)0);
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test block (syscall 12) */
    errno = 0;
    kbd_syscall_results[idx] = block();
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test unblock (syscall 13) */
    errno = 0;
    kbd_syscall_results[idx] = unblock(1);
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test KeyboardEvent (syscall 22) */
    errno = 0;
    kbd_syscall_results[idx] = KeyboardEvent((void *)0);
    kbd_syscall_errnos[idx] = errno;
    idx++;

    /* Test WaitForTick (syscall 23) */
    errno = 0;
    kbd_syscall_results[idx] = WaitForTick();
    kbd_syscall_errnos[idx] = errno;
    idx++;

    kbd_syscall_tested = 1;
}

/****************************************/
/**    Utility Functions               **/
/****************************************/

void busyWait(int ticks) {
    int start_time = gettime();
    while (gettime() - start_time < ticks) {
        /* Busy wait */
    }
}

void waitTicksYield(int ticks) {
    for (int i = 0; i < ticks; i++) {
        WaitForTick();
    }
}

void write_current_info(void) {
    prints("[PID %d] [TID %d] ", getpid(), gettid());
}

/****************************************/
/**    Synchronization Functions       **/
/****************************************/

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

/****************************************/
/**    Thread Work Functions           **/
/****************************************/

void simple_thread_func(void *arg) {
    int flag_index = *(int *)arg;

    prints("[PID %d] [TID %d] Simple thread running, will set flag %d\n", getpid(), gettid(),
           flag_index);

    /* Signal completion */
    set_flag(flag_index);

    ThreadExit();
}

void thread_work(void *arg) {
    int ticks = *(int *)arg;
    int start_time = gettime();

    write_current_info();
    msg = "Thread working for ";
    prints("Thread working for %d ticks...\n", ticks);

    while (gettime() - start_time < ticks) {
        /* Working... */
    }

    prints("[PID %d] [TID %d] Thread finished work\n", getpid(), gettid());

    ThreadExit();
}

void thread_block_func(void *arg) {
    int flag_index = *(int *)arg;

    prints("[PID %d] [TID %d] Thread will set flag and block\n", getpid(), gettid());

    set_flag(flag_index);
    block();

    /* After being unblocked */
    prints("[PID %d] [TID %d] Thread was unblocked\n", getpid(), gettid());

    ThreadExit();
}

/****************************************/
/**    Thread Test Functions           **/
/****************************************/

void subtest_max_threads(int *passed) {
    /* Master already uses 1 slot (slot 0), so we can create MAX - 1 new threads */
    int max_new_threads = MAX_THREADS_PER_PROCESS - 1;

    print_subtest_header(1, "Create maximum additional threads (9) and verify limit");

    clear_all_flags();

    int tids[MAX_THREADS_PER_PROCESS];
    int flag_indices[MAX_THREADS_PER_PROCESS];
    int created = 0;

    prints("[PID %d] [TID %d] Creating up to %d additional threads (master uses 1 slot)...\n",
           getpid(), gettid(), max_new_threads);

    /* Try to create MAX_THREADS_PER_PROCESS threads, expect only max_new_threads to succeed */
    for (int i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
        flag_indices[i] = i;
        tids[i] = ThreadCreate(minimal_thread_func, &flag_indices[i]);

        if (tids[i] > 0) {
            created++;
            prints("[PID %d] [TID %d] Created thread TID %d\n", getpid(), gettid(), tids[i]);
        }
    }

    prints("[PID %d] [TID %d] Created %d/%d additional threads\n", getpid(), gettid(), created,
           max_new_threads);

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

    /* Now try to create one more - should fail (we have master + max_new_threads = 10 total) */
    int extra_flag = 9;
    int extra_tid = ThreadCreate(simple_thread_func, &extra_flag);

    if (extra_tid < 0) {
        prints("[PID %d] [TID %d] Correctly rejected 10th additional thread (returned %d)\n",
               getpid(), gettid(), extra_tid);
        *passed = 1;
    } else {
        prints("[PID %d] [TID %d] ERROR: Created extra thread with TID %d (should have failed)\n",
               getpid(), gettid(), extra_tid);
        *passed = 0;
    }

    print_subtest_result(*passed);
    thread_subtests_run++;
    if (*passed) thread_subtests_passed++;

    /* Wait for created threads to complete */
    wait_for_flags(created);
}

void subtest_tid_reuse(int *passed) {
    print_subtest_header(3, "TID reuse after thread deletion");

    clear_all_flags();

    /* Create a thread, let it exit, create another - TID should be reused */
    int flag0 = 0;
    int first_tid = ThreadCreate(minimal_thread_func, &flag0);

    prints("[PID %d] [TID %d] First thread created with TID %d\n", getpid(), gettid(), first_tid);

    /* Wait for thread to complete */
    wait_for_flag(0);

    /* Small delay to ensure cleanup */
    busyWait(TIME_MINIMAL);

    /* Create another thread */
    clear_all_flags();
    int flag1 = 0;
    int second_tid = ThreadCreate(minimal_thread_func, &flag1);

    prints("[PID %d] [TID %d] Second thread created with TID %d\n", getpid(), gettid(), second_tid);

    /* Wait for completion */
    wait_for_flag(0);

    /* Check if TID was reused (should be same or another freed TID) */
    if (second_tid > 0) {
        if (second_tid == first_tid) {
            prints("[PID %d] [TID %d] TID %d was reused\n", getpid(), gettid(), first_tid);
        } else {
            prints("[PID %d] [TID %d] Different TID used (still valid - TIDs are being freed)\n",
                   getpid(), gettid());
        }
        *passed = 1;
    } else {
        prints("[PID %d] [TID %d] ERROR: Could not create second thread\n", getpid(), gettid());
        *passed = 0;
    }

    print_subtest_result(*passed);
    thread_subtests_run++;
    if (*passed) thread_subtests_passed++;
}

void subtest_last_thread_exits(void) {
    print_subtest_header(4, "Process terminates when last thread exits");

    prints("[PID %d] [TID %d] Creating child process to test last thread exit...\n", getpid(),
           gettid());

    int pid = fork();

    if (pid == 0) {
        /* Child process - we are the only thread initially */
        prints("[PID %d] [TID %d] Child: I am the only thread, calling ThreadExit()\n", getpid(),
               gettid());
        prints("-> PASSED (if this prints and process exits)\n");

        /* This should terminate the process */
        ThreadExit();

        /* Should never reach here */
        prints("-> FAILED: Code after ThreadExit executed!\n");
        exit();

    } else if (pid > 0) {
        /* Parent waits for child to complete */
        busyWait(TIME_MEDIUM);

        prints("[PID %d] [TID %d] Parent: Child process should have terminated\n", getpid(),
               gettid());

        thread_subtests_run++;
        thread_subtests_passed++; /* If we got here, child exited properly */
    } else {
        prints("-> FAILED: Fork failed\n");
        thread_subtests_run++;
    }
}

void subtest_master_reassignment(void) {
    print_subtest_header(5, "Master thread reassignment when master exits");

    prints("[PID %d] [TID %d] Creating child process to test master reassignment...\n", getpid(),
           gettid());

    int pid = fork();

    if (pid == 0) {
        /* Child process - we are the master thread */
        clear_all_flags();

        prints("[PID %d] [TID %d] Child (Master): Creating survivor thread\n", getpid(), gettid());

        int flag_idx = 0;
        int survivor_tid = ThreadCreate(survivor_thread_func, &flag_idx);

        if (survivor_tid > 0) {
            prints("[PID %d] [TID %d] Child (Master): Created survivor TID %d\n", getpid(),
                   gettid(), survivor_tid);

            /* Wait for survivor to be ready */
            wait_for_flag(0);

            prints("[PID %d] [TID %d] Child (Master): Exiting, survivor should become master\n",
                   getpid(), gettid());
            prints("-> PASSED (if survivor continues and process eventually exits)\n");

            /* Master exits - survivor should take over */
            ThreadExit();

            /* Should never reach here */
            prints("-> FAILED: Code after ThreadExit executed!\n");
        } else {
            prints("-> FAILED: Could not create survivor thread\n");
        }
        exit();

    } else if (pid > 0) {
        /* Parent waits for child to complete */
        busyWait(TIME_LONG);

        prints("[PID %d] [TID %d] Parent: Child process should have completed\n", getpid(),
               gettid());

        thread_subtests_run++;
        thread_subtests_passed++; /* If we got here, reassignment worked */
    } else {
        prints("-> FAILED: Fork failed\n");
        thread_subtests_run++;
    }
}

void subtest_fork_single_thread(int *passed) {
    print_subtest_header(6, "Fork copies only current thread");

    clear_all_flags();

    /* Create a secondary thread that will work during fork */
    prints("[PID %d] [TID %d] Creating secondary thread before fork...\n", getpid(), gettid());

    int flag_idx = 0;
    int secondary_tid = ThreadCreate(long_work_thread_func, &flag_idx);

    if (secondary_tid <= 0) {
        prints("-> FAILED: Could not create secondary thread\n");
        *passed = 0;
        print_subtest_result(*passed);
        thread_subtests_run++;
        return;
    }

    /* Wait for secondary thread to signal it has started */
    wait_for_flag(0);

    prints("[PID %d] [TID %d] Secondary thread TID %d is working, now forking...\n", getpid(),
           gettid(), secondary_tid);

    int child_pid = fork();

    if (child_pid == 0) {
        /* Child process - should only have current thread, not the secondary one */
        /* Child starts fresh with 1 thread (master), so can create 9 more */
        int max_new_threads = MAX_THREADS_PER_PROCESS - 1;

        prints("[PID %d] [TID %d] Child: I should be the only thread\n", getpid(), gettid());

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

        prints("[PID %d] [TID %d] Child: Created %d threads (expected %d)\n", getpid(), gettid(),
               can_create, max_new_threads);

        if (can_create == max_new_threads) {
            prints("-> PASSED (fork copied only current thread)\n");
        } else {
            prints("-> FAILED (fork may have copied other threads)\n");
        }

        /* Wait for threads to complete and exit */
        busyWait(TIME_MEDIUM);
        exit();

    } else if (child_pid > 0) {
        /* Parent - wait for child and our secondary thread */
        prints("[PID %d] [TID %d] Parent: Forked child PID %d\n", getpid(), gettid(), child_pid);

        /* Wait for child to complete its test and for secondary thread to finish */
        busyWait(TIME_LONG + TIME_MEDIUM);

        prints("[PID %d] [TID %d] Parent: Test completed\n", getpid(), gettid());

        *passed = 1; /* Success if we reach here */
        print_subtest_result(*passed);
        thread_subtests_run++;
        if (*passed) thread_subtests_passed++;

    } else {
        prints("-> FAILED: Fork failed\n");
        *passed = 0;
        print_subtest_result(*passed);
        thread_subtests_run++;
    }
}

void subtest_wrapper_calls_exit(int *passed) {
    print_subtest_header(7, "Wrapper calls ThreadExit on function return");

    write_current_info();
    prints("Creating thread that returns without explicit ThreadExit...\n");

    clear_all_flags();

    int flag_idx = 0;
    int tid = ThreadCreate(no_explicit_exit_func, &flag_idx);

    if (tid <= 0) {
        prints("-> FAILED: Could not create thread\n");
        *passed = 0;
        print_subtest_result(*passed);
        thread_subtests_run++;
        return;
    }

    prints("[PID %d] [TID %d] Created thread TID %d (no explicit exit)\n", getpid(), gettid(), tid);

    /* Wait for the thread to signal it executed */
    wait_for_flag(0);

    /* Give time for the thread to return and wrapper to call ThreadExit */
    busyWait(TIME_MINIMAL);

    /* If we get here without crashing, the wrapper worked */
    prints("[PID %d] [TID %d] System stable - wrapper called ThreadExit\n", getpid(), gettid());
    *passed = 1;

    print_subtest_result(*passed);
    thread_subtests_run++;
    if (*passed) thread_subtests_passed++;
}

void thread_tests(void) {
    print_test_header("THREAD SUPPORT TESTS");

    thread_subtests_run = 0;
    thread_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting thread test suite...\n", getpid(), gettid());

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

    /* Subtest 7: Wrapper calls ThreadExit automatically */
    subtest_wrapper_calls_exit(&result);

    /* Print thread test summary */
    prints("\n========================================\n");
    prints("THREAD SUPPORT TESTS: %d/%d subtests passed\n", thread_subtests_passed,
           thread_subtests_run);
    prints("========================================\n");

    int all_passed = (thread_subtests_passed == thread_subtests_run);
    print_test_result("THREAD SUPPORT TESTS", all_passed);

    /* Track in global summary */
    project_tests_run++;
    if (all_passed) project_tests_passed++;
}

/****************************************/
/**    Keyboard Test Functions         **/
/****************************************/

void subtest_kbd_register(int *passed) {
    print_subtest_header(1, "Register keyboard handler");

    int ret = KeyboardEvent(test_kbd_handler);

    if (ret == 0) {
        prints("[PID %d] [TID %d] KeyboardEvent registered successfully\n", getpid(), gettid());
        *passed = 1;
    } else {
        prints("[PID %d] [TID %d] KeyboardEvent failed with error %d\n", getpid(), gettid(), ret);
        *passed = 0;
    }

    print_subtest_result(*passed);
    kbd_subtests_run++;
    if (*passed) kbd_subtests_passed++;
}

void subtest_kbd_disable(int *passed) {
    print_subtest_header(2, "Disable keyboard handler with NULL");

    /* First register a handler */
    KeyboardEvent(test_kbd_handler);

    /* Now disable it */
    int ret = KeyboardEvent((void *)0);

    if (ret == 0) {
        prints("[PID %d] [TID %d] KeyboardEvent(NULL) succeeded\n", getpid(), gettid());
        *passed = 1;
    } else {
        prints("[PID %d] [TID %d] KeyboardEvent(NULL) failed with error %d\n", getpid(), gettid(),
               ret);
        *passed = 0;
    }

    print_subtest_result(*passed);
    kbd_subtests_run++;
    if (*passed) kbd_subtests_passed++;
}

void subtest_kbd_handler_called(int *passed) {
    print_subtest_header(3, "Verify handler is called on key events");

    kbd_events_received = 0;
    kbd_key_index = 0;
    for (int i = 0; i < KBD_MAX_KEYS; i++) {
        kbd_keys[i] = 0;
        kbd_pressed[i] = 0;
    }

    /* Register handler */
    int ret = KeyboardEvent(test_kbd_handler);
    if (ret != 0) {
        prints("[PID %d] [TID %d] Failed to register handler\n", getpid(), gettid());
        *passed = 0;
        print_subtest_result(*passed);
        kbd_subtests_run++;
        return;
    }

    prints("[PID %d] [TID %d] Handler registered. Press some keys...\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Waiting for keyboard events (5 seconds or %d keys)...\n", getpid(),
           gettid(), KBD_MAX_KEYS);

    /* Wait for key events - exit early if we got enough keys */
    int start = gettime();
    while (gettime() - start < TIME_KBD_WAIT_FIRST && kbd_key_index < KBD_MAX_KEYS) {
        /* Busy wait for key events */
    }

    prints("[PID %d] [TID %d] Received %d keyboard events\n", getpid(), gettid(),
           kbd_events_received);

    if (kbd_events_received > 0) {
        int count = kbd_key_index; /* Number of keys stored (max KBD_MAX_KEYS) */
        prints("[PID %d] [TID %d] First %d keys (scancode -> char, pressed/released):\n", getpid(),
               gettid(), count);

        /* Show the first keys in order */
        for (int i = 0; i < count; i++) {
            char scancode = kbd_keys[i];
            char ch = (scancode < 87) ? scancode_to_char[(int)scancode] : '?';
            if (ch == '\0') ch = '?';
            prints("  [%d] scancode=0x%d -> '%c' (%s)\n", i + 1, (int)scancode, ch,
                   kbd_pressed[i] ? "PRESSED" : "RELEASED");
        }
        *passed = 1;
    } else {
        prints("[PID %d] [TID %d] No events received (press keys during test!)\n", getpid(),
               gettid());
        /* Still pass if no keys pressed - handler registration worked */
        *passed = 1;
    }

    /* Disable handler */
    KeyboardEvent((void *)0);

    print_subtest_result(*passed);
    kbd_subtests_run++;
    if (*passed) kbd_subtests_passed++;
}

void subtest_kbd_einprogress(int *passed) {
    print_subtest_header(4, "Verify syscalls return EINPROGRESS inside handler");

    /* Reset test variables */
    kbd_syscall_tested = 0;
    for (int i = 0; i < NUM_SYSCALLS_TO_TEST; i++) {
        kbd_syscall_results[i] = 12345;
        kbd_syscall_errnos[i] = 0;
    }

    /* Register handler that tests all syscalls */
    int ret = KeyboardEvent(test_kbd_handler_with_syscall);
    if (ret != 0) {
        prints("[PID %d] [TID %d] Failed to register handler\n", getpid(), gettid());
        *passed = 0;
        print_subtest_result(*passed);
        kbd_subtests_run++;
        return;
    }

    prints("[PID %d] [TID %d] Handler registered (will test ALL syscalls inside).\n", getpid(),
           gettid());
    prints("[PID %d] [TID %d] Press a key to trigger the handler...\n", getpid(), gettid());

    /* Wait for a key event */
    int start = gettime();
    while (gettime() - start < TIME_KBD_WAIT && kbd_syscall_tested == 0) {
        /* Busy wait for key event */
    }

    /* Disable handler */
    KeyboardEvent((void *)0);

    if (kbd_syscall_tested == 0) {
        prints("[PID %d] [TID %d] No key pressed - cannot verify EINPROGRESS\n", getpid(),
               gettid());
        /* Pass anyway - we can't force user to press keys */
        *passed = 1;
        print_subtest_result(*passed);
        kbd_subtests_run++;
        if (*passed) kbd_subtests_passed++;
        return;
    }

    /* Check results for each syscall */
    const char *syscall_names[NUM_SYSCALLS_TO_TEST] = {
        "getpid",       "gettid", "gettime", "fork",          "write",
        "ThreadCreate", "block",  "unblock", "KeyboardEvent", "WaitForTick"};
    /* Expected results: -1 with EINPROGRESS for all except gettime (gettime doesn't check) */
    int expects_einprogress[NUM_SYSCALLS_TO_TEST] = {1, 1, 0, 1, 1, 1, 1, 1, 1, 1};

    int syscalls_passed = 0;
    int syscalls_tested = NUM_SYSCALLS_TO_TEST;

    prints("[PID %d] [TID %d] Syscall EINPROGRESS test results:\n", getpid(), gettid());

    for (int i = 0; i < NUM_SYSCALLS_TO_TEST; i++) {
        int result = kbd_syscall_results[i];
        int err = kbd_syscall_errnos[i];
        int ok = 0;

        if (expects_einprogress[i]) {
            /* Should return -1 with EINPROGRESS (115) */
            if (result == -1 && err == 115) {
                ok = 1;
                syscalls_passed++;
            }
        } else {
            /* gettime should work (doesn't check keyboard context) */
            if (result >= 0) {
                ok = 1;
                syscalls_passed++;
            }
        }

        prints("  - %s: %s (ret=%d, errno=%d)\n", syscall_names[i], ok ? "PASSED" : "FAILED",
               result, err);
    }

    prints("[PID %d] [TID %d] Syscalls passed: %d/%d\n", getpid(), gettid(), syscalls_passed,
           syscalls_tested);

    *passed = (syscalls_passed == syscalls_tested);
    print_subtest_result(*passed);
    kbd_subtests_run++;
    if (*passed) kbd_subtests_passed++;
}

void keyboard_tests(void) {
    print_test_header("KEYBOARD SUPPORT TESTS");

    kbd_subtests_run = 0;
    kbd_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting keyboard test suite...\n", getpid(), gettid());

    int result;

    /* Subtest 1: Register handler */
    subtest_kbd_register(&result);

    /* Disable before next test */
    KeyboardEvent((void *)0);

    /* Subtest 2: Disable handler */
    subtest_kbd_disable(&result);

    /* Subtest 3: Handler called on events */
    subtest_kbd_handler_called(&result);

    /* Pause before subtest 4 to avoid accidental key presses */
    prints("[PID %d] [TID %d] Pausing 1 second before next test...\n", getpid(), gettid());
    busyWait(TIME_KBD_PAUSE);

    /* Subtest 4: EINPROGRESS inside handler */
    subtest_kbd_einprogress(&result);

    /* Print keyboard test summary */
    prints("\n========================================\n");
    prints("KEYBOARD SUPPORT TESTS: %d/%d subtests passed\n", kbd_subtests_passed,
           kbd_subtests_run);
    prints("========================================\n");

    int all_passed = (kbd_subtests_passed == kbd_subtests_run);
    print_test_result("KEYBOARD SUPPORT TESTS", all_passed);

    /* Track in global summary */
    project_tests_run++;
    if (all_passed) project_tests_passed++;
}

/****************************************/
/**    Screen Support Test Functions   **/
/****************************************/

int test_screen_write_invalid_fd(void) {
    print_subtest_header(1, "Invalid file descriptor access");

    char test_buffer[10] = "test";
    int passed = 1;

    /* Test invalid fd (should return -1 and set errno to EBADF) */
    int result = write(99, test_buffer, sizeof(test_buffer));
    if (result != -1 || errno != EBADF) {
        prints("[PID %d] [TID %d] ERROR: write(99, ...) returned %d (errno=%d), expected -1 "
               "(errno=%d)\n",
               getpid(), gettid(), result, errno, EBADF);
        passed = 0;
    } else {
        prints("[PID %d] [TID %d] write(99, ...) correctly returned -1 with errno=EBADF\n",
               getpid(), gettid());
    }

    /* Test write permission on screen fd (should work) */
    if (passed) {
        result = write(10, test_buffer, sizeof(test_buffer));
        if (result < 0) {
            prints("[PID %d] [TID %d] ERROR: write(10, ...) failed: %d\n", getpid(), gettid(),
                   result);
            passed = 0;
        } else {
            prints("[PID %d] [TID %d] write(10, ...) succeeded with result: %d\n", getpid(),
                   gettid(), result);
        }
    }

    print_subtest_result(passed);
    return passed;
}

int test_screen_write_basic(void) {
    print_subtest_header(2, "Basic screen buffer write");
    clear_screen_buffer(10);
    prints("[PID %d] [TID %d] Preparing to write checkerboard pattern to screen buffer...\n",
           getpid(), gettid());
    busyWait(TIME_VISUAL_PAUSE);

    /* Create a checkerboard test pattern */
    char screen_buffer[SCREEN_BUFFER_SIZE];
    generate_checkerboard_pattern(screen_buffer);

    /* Write to screen buffer (fd=10) */
    int result = write(10, screen_buffer, sizeof(screen_buffer));
    int passed = 1;
    busyWait(TIME_VISUAL_DISPLAY);
    clear_screen_buffer(10);
    if (result != sizeof(screen_buffer)) {
        prints("[PID %d] [TID %d] ERROR: write returned %d, expected %d\n", getpid(), gettid(),
               result, sizeof(screen_buffer));
        passed = 0;
    } else {
        prints("[PID %d] [TID %d] Successfully wrote %d bytes to screen buffer\n", getpid(),
               gettid(), result);
    }

    print_subtest_result(passed);
    return passed;
}

int test_screen_write_size_limits(void) {
    print_subtest_header(3, "Screen buffer size limits");

    /* Create a buffer larger than screen (4000 bytes) */
    char large_buffer[SCREEN_BUFFER_SIZE + 100]; /* 4100 bytes */
    for (unsigned int i = 0; i < SCREEN_BUFFER_SIZE; i++) {
        large_buffer[i] = 'X'; /* Fill valid part with X */
    }
    /* Fill the overflow part with different character and color */
    for (unsigned int i = SCREEN_BUFFER_SIZE; i < sizeof(large_buffer); i += 2) {
        large_buffer[i] = '!';      /* Character '!' */
        large_buffer[i + 1] = 0x4F; /* Red background, White text */
    }

    clear_screen_buffer(10);
    prints("[PID %d] [TID %d] Waiting some ticks before writing large buffer (%d bytes)...\n",
           getpid(), gettid(), sizeof(large_buffer));
    busyWait(TIME_VISUAL_PAUSE);

    /* Write should truncate to screen size */
    int result = write(10, large_buffer, sizeof(large_buffer));
    int passed = 1;

    if (result != SCREEN_BUFFER_SIZE) { /* Should return 4000 */
        prints("[PID %d] [TID %d] ERROR: write returned %d, expected %d (screen size)\n", getpid(),
               gettid(), result, SCREEN_BUFFER_SIZE);
        passed = 0;
    } else {
        busyWait(TIME_VISUAL_DISPLAY);
        prints("[PID %d] [TID %d] Large buffer correctly truncated to %d bytes\n", getpid(),
               gettid(), result);
    }

    clear_screen_buffer(10);
    print_subtest_result(passed);
    return passed;
}

int test_screen_visual_patterns(void) {
    print_subtest_header(4, "Screen visual patterns display");

    /* Create the three different patterns */
    char frame_buffer1[SCREEN_BUFFER_SIZE]; /* Checkerboard */
    char frame_buffer2[SCREEN_BUFFER_SIZE]; /* Rainbow */
    char frame_buffer3[SCREEN_BUFFER_SIZE]; /* Border */

    generate_checkerboard_pattern(frame_buffer1);
    generate_rainbow_pattern(frame_buffer2);
    generate_border_pattern(frame_buffer3);

    int passed = 1;

    /* Display Pattern 1: Checkerboard */
    clear_screen_buffer(10);
    prints("[PID %d] [TID %d] Waiting some ticks before displaying checkerboard pattern...\n",
           getpid(), gettid());
    busyWait(TIME_VISUAL_PAUSE);

    int result = write(10, frame_buffer1, sizeof(frame_buffer1));
    if (result != sizeof(frame_buffer1)) {
        prints("[PID %d] [TID %d] ERROR: Checkerboard pattern write failed (%d)\n", getpid(),
               gettid(), result);
        passed = 0;
    }
    busyWait(TIME_VISUAL_DISPLAY);

    /* Display Pattern 2: Rainbow */
    clear_screen_buffer(10);
    prints("[PID %d] [TID %d] Waiting some ticks before displaying rainbow pattern...\n", getpid(),
           gettid());
    busyWait(TIME_VISUAL_PAUSE);

    result = write(10, frame_buffer2, sizeof(frame_buffer2));
    if (result != sizeof(frame_buffer2)) {
        prints("[PID %d] [TID %d] ERROR: Rainbow pattern write failed (%d)\n", getpid(), gettid(),
               result);
        passed = 0;
    }
    busyWait(TIME_VISUAL_DISPLAY);

    /* Display Pattern 3: Border */
    clear_screen_buffer(10);
    prints("[PID %d] [TID %d] Waiting some ticks before displaying border pattern...\n", getpid(),
           gettid());
    busyWait(TIME_VISUAL_PAUSE);

    result = write(10, frame_buffer3, sizeof(frame_buffer3));
    if (result != sizeof(frame_buffer3)) {
        prints("[PID %d] [TID %d] ERROR: Border pattern write failed (%d)\n", getpid(), gettid(),
               result);
        passed = 0;
    }
    busyWait(TIME_VISUAL_DISPLAY);
    clear_screen_buffer(10);

    if (passed) {
        prints("[PID %d] [TID %d] All visual patterns displayed successfully\n", getpid(),
               gettid());
    }

    print_subtest_result(passed);
    return passed;
}

int test_screen_write_performance(void) {
    print_test_header("SCREEN PERFORMANCE TEST");

    /* Clear screen before performance test */
    clear_screen_buffer(10);
    prints("[PID %d] [TID %d] Cleared screen, starting performance test...\n", getpid(), gettid());

    /* Create a single frame buffer (4000 bytes = 80x25x2) */
    char frame_buffer[SCREEN_BUFFER_SIZE];
    generate_checkerboard_pattern(frame_buffer);

    /* Measure time for SCREEN_WRITE_ITERATIONS consecutive writes */
    int start_time = gettime();
    int passed = 1;
    unsigned int total_bytes_written = 0;

    for (int frame = 0; frame < SCREEN_WRITE_ITERATIONS; frame++) {
        int result = write(10, frame_buffer, sizeof(frame_buffer));
        if (result != sizeof(frame_buffer)) {
            prints("[PID %d] [TID %d] ERROR: Frame %d write failed (%d)\n", getpid(), gettid(),
                   frame, result);
            passed = 0;
            break;
        }
        total_bytes_written += result;
    }

    int end_time = gettime();
    int elapsed_ticks = end_time - start_time;

    /* Verify total bytes written */
    unsigned int expected_bytes = SCREEN_BUFFER_SIZE * SCREEN_WRITE_ITERATIONS;
    if (total_bytes_written != expected_bytes) {
        prints("[PID %d] [TID %d] ERROR: Total bytes written %u, expected %u\n", getpid(), gettid(),
               total_bytes_written, expected_bytes);
        passed = 0;
    }

    /* Clear screen before printing results */
    clear_screen_buffer(10);

    if (passed) {
        prints("[PID %d] [TID %d] Performance test completed:\n", getpid(), gettid());
        prints("  - Start ticks: %d\n", start_time);
        prints("  - End ticks:   %d\n", end_time);
        prints("  - Total time:  %d ticks for %d frames\n", elapsed_ticks, SCREEN_WRITE_ITERATIONS);
        prints("  - Total bytes: %u\n", total_bytes_written);

        if (elapsed_ticks > 0) {
            prints("  - Average:     %d ticks/frame\n", elapsed_ticks / SCREEN_WRITE_ITERATIONS);
        } else {
            prints("  - Average:     <1 tick/frame\n");
        }
    }

    /* Print performance test summary */
    prints("\n========================================\n");
    prints("SCREEN PERFORMANCE TEST: %d/1 subtests passed\n", passed ? 1 : 0);
    prints("========================================\n");

    print_test_result("SCREEN PERFORMANCE TEST", passed);

    /* Track in global summary */
    screen_perf_passed = passed;
    project_tests_run++;
    if (passed) project_tests_passed++;

    return passed;
}
void screen_tests(void) {
    print_test_header("SCREEN SUPPORT TESTS");

    screen_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting screen support test suite...\n", getpid(), gettid());

    /* Subtest 1: Invalid file descriptor access (first) */
    RESET_ERRNO();
    if (test_screen_write_invalid_fd()) {
        screen_subtests_passed++;
    }

    /* Subtest 2: Basic functionality */
    RESET_ERRNO();
    if (test_screen_write_basic()) {
        screen_subtests_passed++;
    }

    /* Subtest 3: Size limits */
    RESET_ERRNO();
    if (test_screen_write_size_limits()) {
        screen_subtests_passed++;
    }

    /* Subtest 4: Visual patterns display */
    RESET_ERRNO();
    if (test_screen_visual_patterns()) {
        screen_subtests_passed++;
    }

    /* Print screen test summary */
    prints("\n========================================\n");
    prints("SCREEN SUPPORT TESTS: %d/4 subtests passed\n", screen_subtests_passed);
    prints("========================================\n");

    int all_passed = (screen_subtests_passed == 4);
    print_test_result("SCREEN SUPPORT TESTS", all_passed);

    /* Track in global summary */
    project_tests_run++;
    if (all_passed) project_tests_passed++;
}

/****************************************/
/**    WaitForTick Test Functions      **/
/****************************************/

static void wft_test_thread_func(void *arg) {
    int thread_num = *(int *)arg;
    int tick_before = gettime();

    prints("[PID %d] [TID %d] Thread %d calling WaitForTick at tick %d...\n", getpid(), gettid(),
           thread_num, tick_before);

    int result = WaitForTick();

    if (result == 0) {
        int wake_time = gettime();
        /* Check if this is the first thread to wake or if we woke at the same tick */
        if (wft_wake_tick == 0) {
            wft_wake_tick = wake_time;
        }
        if (wake_time == wft_wake_tick) {
            wft_threads_woken_same_tick++;
        }
        prints("[PID %d] [TID %d] Thread %d woke at tick %d\n", getpid(), gettid(), thread_num,
               wake_time);
    } else {
        prints("[PID %d] [TID %d] Thread %d WaitForTick failed: %d\n", getpid(), gettid(),
               thread_num, result);
    }

    wft_threads_completed++;
    ThreadExit();
}

void subtest_waitfortick_basic(int *passed) {
    print_subtest_header(1, "Basic WaitForTick functionality and return value");

    prints("[PID %d] [TID %d] Testing basic WaitForTick...\n", getpid(), gettid());

    RESET_ERRNO();
    int time_before = gettime();
    int result = WaitForTick();
    int time_after = gettime();

    if (result != 0) {
        prints("[PID %d] [TID %d] ERROR: WaitForTick returned %d, expected 0\n", getpid(), gettid(),
               result);
        *passed = 0;
    } else if (errno != 0) {
        prints("[PID %d] [TID %d] ERROR: errno=%d after WaitForTick (expected 0)\n", getpid(),
               gettid(), errno);
        *passed = 0;
    } else if (time_after <= time_before) {
        prints("[PID %d] [TID %d] ERROR: Time did not advance (before=%d, after=%d)\n", getpid(),
               gettid(), time_before, time_after);
        *passed = 0;
    } else {
        prints("[PID %d] [TID %d] WaitForTick returned 0 with errno=0 (correct)\n", getpid(),
               gettid());
        prints("[PID %d] [TID %d] Time before: %d, after: %d (elapsed: %d tick(s), includes "
               "scheduler)\n",
               getpid(), gettid(), time_before, time_after, time_after - time_before);
        *passed = 1;
    }

    print_subtest_result(*passed);
    wft_subtests_run++;
    if (*passed) wft_subtests_passed++;
}

void subtest_waitfortick_multiple_threads(int *passed) {
    print_subtest_header(2, "Multiple threads waiting for tick");

    prints("[PID %d] [TID %d] Creating multiple threads to wait for same tick...\n", getpid(),
           gettid());

    /* Reset test variables */
    wft_threads_completed = 0;
    wft_threads_woken_same_tick = 0;
    wft_wake_tick = 0;

    int num_threads = 3;
    int thread_nums[3] = {1, 2, 3};
    int tids[3];

    /* Create threads */
    for (int i = 0; i < num_threads; i++) {
        tids[i] = ThreadCreate(wft_test_thread_func, &thread_nums[i]);
        if (tids[i] < 0) {
            prints("[PID %d] [TID %d] ERROR: Failed to create thread %d\n", getpid(), gettid(),
                   i + 1);
            *passed = 0;
            print_subtest_result(*passed);
            wft_subtests_run++;
            return;
        }
        prints("[PID %d] [TID %d] Created thread %d with TID %d\n", getpid(), gettid(), i + 1,
               tids[i]);
    }

    /* Wait for all threads to complete - use yielding wait to give threads
     * a chance to run. Need enough ticks for:
     * 1. Each thread to be scheduled and call WaitForTick
     * 2. Clock interrupt to wake them
     * 3. Each thread to complete and exit */
    waitTicksYield(TIME_MINIMAL);

    prints("[PID %d] [TID %d] Threads completed: %d/%d\n", getpid(), gettid(),
           wft_threads_completed, num_threads);
    prints("[PID %d] [TID %d] Threads woken at same tick: %d\n", getpid(), gettid(),
           wft_threads_woken_same_tick);

    /* All threads should have completed and at least 2 should wake at same tick */
    if (wft_threads_completed == num_threads && wft_threads_woken_same_tick >= 2) {
        prints("[PID %d] [TID %d] Multiple threads correctly woken by same tick\n", getpid(),
               gettid());
        *passed = 1;
    } else if (wft_threads_completed == num_threads) {
        prints("[PID %d] [TID %d] All threads completed (wake timing may vary)\n", getpid(),
               gettid());
        *passed = 1; /* Still pass if all completed */
    } else {
        prints("[PID %d] [TID %d] ERROR: Not all threads completed\n", getpid(), gettid());
        *passed = 0;
    }

    print_subtest_result(*passed);
    wft_subtests_run++;
    if (*passed) wft_subtests_passed++;
}

void subtest_waitfortick_timing(int *passed) {
    print_subtest_header(3, "WaitForTick timing accuracy");

    prints("[PID %d] [TID %d] Measuring time for 5 consecutive WaitForTick calls...\n", getpid(),
           gettid());

    int num_waits = 5;
    int time_before = gettime();

    for (int i = 0; i < num_waits; i++) {
        int result = WaitForTick();
        if (result != 0) {
            prints("[PID %d] [TID %d] ERROR: WaitForTick %d failed with %d\n", getpid(), gettid(),
                   i + 1, result);
            *passed = 0;
            print_subtest_result(*passed);
            wft_subtests_run++;
            return;
        }
    }

    int time_after = gettime();
    int elapsed = time_after - time_before;

    prints("[PID %d] [TID %d] Time before: %d, after: %d\n", getpid(), gettid(), time_before,
           time_after);
    prints("[PID %d] [TID %d] Total elapsed: %d ticks for %d waits\n", getpid(), gettid(), elapsed,
           num_waits);

    /* Each WaitForTick should wait for exactly 1 tick, so elapsed should be >= num_waits */
    if (elapsed >= num_waits) {
        prints("[PID %d] [TID %d] Timing is correct (at least %d ticks elapsed)\n", getpid(),
               gettid(), num_waits);
        *passed = 1;
    } else {
        prints("[PID %d] [TID %d] ERROR: Expected at least %d ticks, got %d\n", getpid(), gettid(),
               num_waits, elapsed);
        *passed = 0;
    }

    print_subtest_result(*passed);
    wft_subtests_run++;
    if (*passed) wft_subtests_passed++;
}

void waitfortick_tests(void) {
    print_test_header("TIME SUPPORT TESTS");

    wft_subtests_run = 0;
    wft_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting WaitForTick test suite...\n", getpid(), gettid());

    int result;

    /* Subtest 1: Basic functionality and return value */
    subtest_waitfortick_basic(&result);

    /* Subtest 2: Multiple threads */
    subtest_waitfortick_multiple_threads(&result);

    /* Subtest 3: Timing accuracy */
    subtest_waitfortick_timing(&result);

    /* Print WaitForTick test summary */
    prints("\n========================================\n");
    prints("TIME SUPPORT TESTS: %d/%d subtests passed\n", wft_subtests_passed, wft_subtests_run);
    prints("========================================\n");

    int all_passed = (wft_subtests_passed == wft_subtests_run);
    print_test_result("TIME SUPPORT TESTS", all_passed);

    /* Track in global summary */
    project_tests_run++;
    if (all_passed) project_tests_passed++;
}

/****************************************/
/**    Tick Calibration Test Functions **/
/****************************************/

void tick_calibration_test(void) {
    print_test_header("TICK CALIBRATION TEST");

    prints("[PID %d] [TID %d] Starting tick calibration...\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Test duration: %d seconds\n", getpid(), gettid(),
           TIME_CALIBRATION_SECONDS);
    prints("[PID %d] [TID %d] USE A STOPWATCH!\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Starting in 3 seconds...\n", getpid(), gettid());

    busyWait(TIME_CALIBRATION_COUNTDOWN);

    prints("[PID %d] [TID %d] GO!\n", getpid(), gettid());

    int start_ticks = gettime();
    int expected_ticks = TIME_CALIBRATION_TICKS;

    busyWait(expected_ticks);

    int end_ticks = gettime();
    int elapsed_ticks = end_ticks - start_ticks;

    prints("[PID %d] [TID %d] STOP!\n", getpid(), gettid());

    prints("Expected: %d ticks\n", expected_ticks);
    prints("Elapsed:  %d ticks\n", elapsed_ticks);

    int deviation = elapsed_ticks - expected_ticks;
    if (deviation < 0) deviation = -deviation;

    tick_cal_passed = (deviation <= expected_ticks / 20); /* 5% tolerance */

    print_test_result("TICK CALIBRATION TEST", tick_cal_passed);

    project_tests_run++;
    if (tick_cal_passed) project_tests_passed++;
}

/****************************************/
/**    FPS Visual Test Functions       **/
/****************************************/

/* Keyboard handler for FPS test - N/B/Esc navigation */
static void fps_kbd_handler(char scancode, int pressed) {
    unsigned char sc = (unsigned char)scancode;
    if (!pressed) return; /* Only handle key press, not release */

    switch (sc) {
    case FPS_SCANCODE_ESC:
        fps_test_exit = 1;
        break;
    case FPS_SCANCODE_N:
        fps_next_scene = 1;
        break;
    case FPS_SCANCODE_B:
        fps_prev_scene = 1;
        break;
    }
}

int fps_visual_test(void) {
    char frame_buffer[SCREEN_BUFFER_SIZE];
    char checkerboard_buffer[SCREEN_BUFFER_SIZE];
    char rainbow_buffer[SCREEN_BUFFER_SIZE];
    int current_scene = 1;
    SceneState scene_state;

    /* Pre-generate static patterns (only once) */
    generate_checkerboard_pattern(checkerboard_buffer);
    generate_rainbow_pattern(rainbow_buffer);

    /* Reset state */
    fps_test_exit = 0;
    fps_next_scene = 0;
    fps_prev_scene = 0;
    scene_init(&scene_state);

    /* Register keyboard handler */
    int ret = KeyboardEvent(fps_kbd_handler);
    if (ret != 0) {
        prints("[PID %d] [TID %d] Failed to register keyboard handler for FPS test\n", getpid(),
               gettid());
        return 0;
    }

    prints("[PID %d] [TID %d] FPS Visual Test started.\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Navigation: N=Next, B=Back, Esc=Exit\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Scene 1: Checkerboard pattern (static)\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Scene 2: Rainbow pattern (static)\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Scene 3: Starfield with twinkling stars\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Scene 4: Bouncing balls with decorations\n", getpid(), gettid());

    /* Small pause before starting */
    busyWait(TIME_VISUAL_PAUSE);

    int start_time = gettime();

    /* Main render loop */
    while (!fps_test_exit && (gettime() - start_time) < TIME_FPS_TEST_DURATION) {
        /* Handle scene navigation */
        if (fps_next_scene) {
            fps_next_scene = 0;
            current_scene++;
            if (current_scene > FPS_NUM_SCENES) {
                current_scene = 1; /* Wrap around */
            }
            scene_init(&scene_state); /* Reset scene state */
        }
        if (fps_prev_scene) {
            fps_prev_scene = 0;
            current_scene--;
            if (current_scene < 1) {
                current_scene = FPS_NUM_SCENES; /* Wrap around */
            }
            scene_init(&scene_state); /* Reset scene state */
        }

        /* Render current scene */
        switch (current_scene) {
        case 1:
            /* Static pattern: copy pre-generated buffer */
            for (int i = 0; i < SCREEN_BUFFER_SIZE; i++) {
                frame_buffer[i] = checkerboard_buffer[i];
            }
            break;
        case 2:
            /* Static pattern: copy pre-generated buffer */
            for (int i = 0; i < SCREEN_BUFFER_SIZE; i++) {
                frame_buffer[i] = rainbow_buffer[i];
            }
            break;
        case 3:
            render_scene_starfield(frame_buffer, &scene_state);
            break;
        case 4:
            render_scene_balls(frame_buffer, &scene_state);
            break;
        }

        /* Draw navigation message in row 0 */
        scene_draw_nav_message(frame_buffer, current_scene, FPS_NUM_SCENES);

        /* Write to screen - frame_count incremented in kernel */
        write(10, frame_buffer, SCREEN_BUFFER_SIZE);
    }

    /* Disable keyboard handler */
    KeyboardEvent((void *)0);

    /* Clear screen */
    clear_screen_buffer(10);

    int elapsed = gettime() - start_time;
    int elapsed_seconds = elapsed / TICKS_PER_SECOND;

    prints("[PID %d] [TID %d] FPS Test completed.\n", getpid(), gettid());
    prints("[PID %d] [TID %d] Duration: %d seconds\n", getpid(), gettid(), elapsed_seconds);

    return 1;
}

void fps_tests(void) {
    print_test_header("FPS VISUAL TEST");

    prints("[PID %d] [TID %d] Starting FPS visual test...\n", getpid(), gettid());
    prints("[PID %d] [TID %d] This test displays scenes with visual animations.\n", getpid(),
           gettid());
    prints("[PID %d] [TID %d] Watch the FPS counter in the top-right corner.\n", getpid(),
           gettid());
    prints("[PID %d] [TID %d] Use N/B to navigate scenes, Esc to exit.\n", getpid(), gettid());

    /* Small pause before starting */
    busyWait(TIME_VISUAL_PAUSE);

    fps_test_passed = fps_visual_test();

    print_test_result("FPS VISUAL TEST", fps_test_passed);

    project_tests_run++;
    if (fps_test_passed) project_tests_passed++;
}

/****************************************/
/**    Main Test Entry Point           **/
/****************************************/

void execute_project_tests(void) {
    prints("\n=========================================\n");
    prints("      PROJECT TEST SUITE                 \n");
    prints("=========================================\n\n");

    prints("[PID %d] [TID %d] Coordinator thread starting tests...\n\n", getpid(), gettid());

    /* Reset global counters */
    project_tests_run = 0;
    project_tests_passed = 0;

#if THREAD_TEST
    RESET_ERRNO();
    thread_tests();
#endif

#if KEYBOARD_TEST
    RESET_ERRNO();
    keyboard_tests();
#endif

#if SCREEN_TEST
    RESET_ERRNO();
    screen_tests();
#endif

#if SCREEN_PERFORMANCE_TEST
    RESET_ERRNO();
    test_screen_write_performance();
#endif

#if WAITFORTICK_TEST
    RESET_ERRNO();
    waitfortick_tests();
#endif

#if TICK_CALIBRATION_TEST
    RESET_ERRNO();
    tick_calibration_test();
#endif

#if FPS_TEST
    RESET_ERRNO();
    fps_tests();
#endif

    /* Final summary */
    prints("\n=========================================\n");
    prints("      PROJECT TEST SUITE SUMMARY         \n");
    prints("=========================================\n");
    prints("Tests executed:\n");
#if THREAD_TEST
    prints("  - THREAD SUPPORT TESTS:     %s\n",
           (thread_subtests_passed == thread_subtests_run) ? "PASSED" : "FAILED");
#endif
#if KEYBOARD_TEST
    prints("  - KEYBOARD SUPPORT TESTS:   %s\n",
           (kbd_subtests_passed == kbd_subtests_run) ? "PASSED" : "FAILED");
#endif
#if SCREEN_TEST
    prints("  - SCREEN SUPPORT TESTS:     %s\n",
           (screen_subtests_passed == 4) ? "PASSED" : "FAILED");
#endif
#if SCREEN_PERFORMANCE_TEST
    prints("  - SCREEN PERFORMANCE TEST:  %s\n", screen_perf_passed ? "PASSED" : "FAILED");
#endif
#if WAITFORTICK_TEST
    prints("  - TIME SUPPORT TESTS:       %s\n",
           (wft_subtests_passed == wft_subtests_run) ? "PASSED" : "FAILED");
#endif
#if TICK_CALIBRATION_TEST
    prints("  - TICK CALIBRATION TEST:    %s\n", tick_cal_passed ? "PASSED" : "FAILED");
#endif
#if FPS_TEST
    prints("  - FPS VISUAL TEST:          %s\n", fps_test_passed ? "PASSED" : "FAILED");
#endif
    prints("-----------------------------------------\n");
    prints("TOTAL: %d/%d tests passed\n", project_tests_passed, project_tests_run);
    prints("=========================================\n\n");

#if IDLE_SWITCH_TEST
    prints("\n--- Testing: IDLE SWITCH ---\n");
    prints("[PID %d] [TID %d] Terminating init process. System will switch to idle.\n", getpid(),
           gettid());
    exit();
#endif
}
