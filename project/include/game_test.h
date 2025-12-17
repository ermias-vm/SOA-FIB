/**
 * @file game_test.h
 * @brief Test suite interface for game rendering system (M5.6).
 *
 * This header defines test function prototypes and testing
 * utilities for the game render buffer functionality verification.
 */

#ifndef __GAME_TEST_H__
#define __GAME_TEST_H__

#include <game_render.h>
#include <game_types.h>

/**********************/
/**   Test Enables   **/
/**********************/

// clang-format off
/* MASTER SWITCH */
#define RUN_GAME_TESTS              1   /**< Enable/disable ALL game tests */

/* RENDER BUFFER TESTS (M5.6) */
#define TEST_RENDER_INIT            1   /**< Test render_init() */
#define TEST_RENDER_CLEAR           1   /**< Test render_clear() */
#define TEST_RENDER_SET_CELL        1   /**< Test render_set_cell() */
#define TEST_RENDER_PUT_STRING      1   /**< Test render_put_string() functions */
#define TEST_RENDER_FILL_RECT       1   /**< Test render_fill_rect() */
#define TEST_RENDER_DRAW_LINES      1   /**< Test line drawing functions */
#define TEST_RENDER_COLORS          1   /**< Test color management functions */
#define TEST_RENDER_PRESENT         1   /**< Test render_present() */
#define TEST_RENDER_NUMBERS         1   /**< Test number rendering functions */
#define TEST_RENDER_VISUAL          1   /**< Visual pattern tests */
// clang-format on

/**********************/
/**   Test Timing    **/
/**********************/

#define GAME_TEST_VISUAL_PAUSE      50   /**< Ticks to pause for visual tests */
#define GAME_TEST_VISUAL_DISPLAY    100  /**< Ticks to display visual patterns */

/****************************************/
/**    Utility Functions               **/
/****************************************/

/**
 * @brief Print a subtest header.
 * @param num Subtest number.
 * @param name Subtest name/description.
 */
void game_test_print_header(int num, const char* name);

/**
 * @brief Print subtest result.
 * @param passed 1 if passed, 0 if failed.
 */
void game_test_print_result(int passed);

/**
 * @brief Busy wait for specified ticks.
 * @param ticks Number of ticks to wait.
 */
void game_test_wait(int ticks);

/****************************************/
/**    Render Buffer Test Functions    **/
/****************************************/

/**
 * @brief Test render_init() function.
 * 
 * Verifies that:
 * - Buffers are initialized
 * - Default colors are set
 * - Screen is cleared
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_init(int *passed);

/**
 * @brief Test render_clear() function.
 * 
 * Verifies that:
 * - Back buffer is cleared with spaces
 * - Layer colors are applied correctly
 * - Dirty flag is set
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_clear(int *passed);

/**
 * @brief Test render_set_cell() function.
 * 
 * Verifies that:
 * - Cell character is set correctly
 * - Cell color is set correctly
 * - Out of bounds writes are rejected
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_set_cell(int *passed);

/**
 * @brief Test render_put_string() and render_put_string_colored().
 * 
 * Verifies that:
 * - Strings are written correctly
 * - Colors are applied
 * - Strings are clipped at screen edge
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_put_string(int *passed);

/**
 * @brief Test render_fill_rect() function.
 * 
 * Verifies that:
 * - Rectangle is filled correctly
 * - Clipping at screen edges works
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_fill_rect(int *passed);

/**
 * @brief Test render_draw_horizontal_line() and render_draw_vertical_line().
 * 
 * Verifies that:
 * - Lines are drawn correctly
 * - Clipping works
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_draw_lines(int *passed);

/**
 * @brief Test color management functions.
 * 
 * Verifies that:
 * - render_make_color() creates correct colors
 * - render_get_layer_color() returns correct layer colors
 * - render_set_default_color() works
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_colors(int *passed);

/**
 * @brief Test render_present() function.
 * 
 * Verifies that:
 * - Buffer is written to screen
 * - No crashes occur
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_present(int *passed);

/**
 * @brief Test number rendering functions.
 * 
 * Verifies that:
 * - render_number() displays correctly
 * - render_number_padded() pads with zeros
 * - render_number_padded_char() uses custom padding
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_numbers(int *passed);

/**
 * @brief Visual test of render buffer patterns.
 * 
 * Displays various patterns to visually verify rendering:
 * - Layer colors (sky, layers 1-4, status bars)
 * - Text rendering
 * - Shapes and lines
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void test_render_visual_patterns(int *passed);

/****************************************/
/**    Main Test Entry Points          **/
/****************************************/

/**
 * @brief Run all render buffer tests (M5.6).
 * 
 * Executes all enabled render buffer tests and prints summary.
 */
void render_buffer_tests(void);

/**
 * @brief Execute all game test suites.
 * 
 * This function runs all enabled game test suites and provides
 * a comprehensive test summary.
 */
void execute_game_tests(void);

#endif /* __GAME_TEST_H__ */
