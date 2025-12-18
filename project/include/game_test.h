/**
 * @file game_test.h
 * @brief Test suite for game subsystems (M5.4-M5.6).
 *
 * Milestones:
 *   M5.4 - Entity System (player, enemies, movement)
 *   M5.5 - Input System (keyboard events, direction mapping)
 *   M5.6 - Render Buffer (double buffering, screen output)
 */

#ifndef __GAME_TEST_H__
#define __GAME_TEST_H__

#include <game_config.h>
#include <game_entities.h>
#include <game_input.h>
#include <game_render.h>
#include <game_types.h>

/* ============================================================================
 *                              TEST SWITCHES
 * ============================================================================ */

// clang-format off
#define RUN_GAME_TESTS      1   /**< Master switch: Enable/disable ALL game tests */

#define RUN_ENTITY_TESTS    1   /**< M5.4: Entity system tests */
#define RUN_INPUT_TESTS     1   /**< M5.5: Input system tests */
#define RUN_RENDER_TESTS    1   /**< M5.6: Render buffer tests */
// clang-format on

/* ============================================================================
 *                              TEST TIMING (uses times.h constants)
 * ============================================================================ */

#define GAME_TEST_VISUAL_PAUSE TIME_DEFAULT  /**< Ticks to pause for visual tests */
#define GAME_TEST_VISUAL_DISPLAY TIME_MEDIUM /**< Ticks to display visual patterns */

/* ============================================================================
 *                           UTILITY FUNCTIONS
 * ============================================================================ */

void game_test_print_header(int num, const char *name);
void game_test_print_result(int passed);
void game_test_wait(int ticks);
void game_test_print_suite_header(const char *name);
void game_test_print_suite_summary(const char *name, int passed, int total);

/* ============================================================================
 *                      M5.4 - ENTITY TEST FUNCTIONS
 * ============================================================================ */

void test_entity_init(int *passed);
void test_entity_position(int *passed);
void test_entity_movement(int *passed);
void test_entity_direction(int *passed);
void test_player_init(int *passed);
void test_player_state(int *passed);
void test_enemy_init(int *passed);
void test_enemy_types(int *passed);
void entity_system_tests(void);

/* ============================================================================
 *                      M5.5 - INPUT TEST FUNCTIONS
 * ============================================================================ */

void test_input_init(int *passed);
void test_input_direction(int *passed);
void test_input_actions(int *passed);
void test_input_state(int *passed);
void test_input_keyboard_event(int *passed);
void input_system_tests(void);

/* ============================================================================
 *                      M5.6 - RENDER BUFFER TEST FUNCTIONS
 * ============================================================================ */

void test_render_init(int *passed);
void test_render_clear(int *passed);
void test_render_set_cell(int *passed);
void test_render_put_string(int *passed);
void test_render_fill_rect(int *passed);
void test_render_draw_lines(int *passed);
void test_render_colors(int *passed);
void test_render_present(int *passed);
void test_render_numbers(int *passed);
void test_render_visual_patterns(int *passed);
void render_buffer_tests(void);

/* ============================================================================
 *                          MAIN ENTRY POINT
 * ============================================================================ */

/**
 * @brief Execute all enabled game test suites.
 */
void execute_game_tests(void);

#endif /* __GAME_TEST_H__ */
