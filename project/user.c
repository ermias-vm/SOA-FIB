/**
 * @file user.c
 * @brief User space initialization and test program for ZeOS.
 *
 * This file contains the initial user process code that runs
 * system tests and demonstrates ZeOS functionality.
 */

#include <game.h>
#include <game_test.h>
#include <libc.h>
#include <project_test.h>
#include <zeos_test.h>

// clang-format off
#define ZEOS_TESTS      0   /**< Run ZeOS base syscall tests (fork, exit, write...) */
#define PROJECT_TESTS   0   /**< Run project milestone tests (threads, keyboard...) */
#define GAME_TESTS      0   /**< Run game subsystem tests (render buffer, entity system, input system...)*/
#define EXECUTE_GAME    1   /**< Execute the actual game (Dig Dug) */
// clang-format on

__attribute__((__section__(".text.main"))) int main(void) {
    write_current_pid();

#if ZEOS_TESTS
    char *msg_zeos = "[USER] Running ZeOS base tests...\n";
    write(1, msg_zeos, strlen(msg_zeos));
    test_zeos();
#endif

#if PROJECT_TESTS
    char *msg_proj = "[USER] Running Project milestone tests...\n";
    write(1, msg_proj, strlen(msg_proj));
    execute_project_tests();
#endif

#if GAME_TESTS
    char *msg_game = "[USER] Running Game subsystem tests...\n";
    write(1, msg_game, strlen(msg_game));
    execute_game_tests();
#endif

#if EXECUTE_GAME
    char *msg_run = "[USER] Starting Dig Dug game...\n";
    write(1, msg_run, strlen(msg_run));
    game_main();
#endif

    prints("[PID %d] [TID %d] [USER] All tasks completed. System idle.\n", getpid(), gettid());

    while (1) {
        // Infinite loop to keep system running
    }
}
