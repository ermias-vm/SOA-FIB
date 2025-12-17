/**
 * @file game_test.c
 * @brief Test suite for game rendering system (M5.6).
 *
 * This file implements comprehensive tests for the render buffer functionality
 * including initialization, drawing primitives, color management, and presentation.
 */

#include <game_test.h>
#include <game_render.h>
#include <game_types.h>
#include <libc.h>

/* External errno variable */
extern int errno;

/* Test counters */
static int game_subtests_run = 0;
static int game_subtests_passed = 0;

/****************************************/
/**    Utility Functions               **/
/****************************************/

void game_test_print_header(int num, const char* name) {
    prints("\n[SUBTEST %d] %s\n", num, name);
}

void game_test_print_result(int passed) {
    if (passed) {
        prints("-> PASSED\n");
    } else {
        prints("-> FAILED\n");
    }
}

void game_test_wait(int ticks) {
    int start = gettime();
    while (gettime() - start < ticks) {
        /* Busy wait */
    }
}

/****************************************/
/**    Render Buffer Test Functions    **/
/****************************************/

void test_render_init(int *passed) {
    game_test_print_header(1, "render_init() - Buffer initialization");
    
    *passed = 1;
    
    /* Initialize the render system */
    render_init();
    
    /* Verify back buffer cells are initialized */
    const ScreenCell* cell = render_get_cell(0, 0);
    if (cell == 0) {
        prints("[ERROR] render_get_cell() returned NULL for valid position\n");
        *passed = 0;
    } else {
        /* Status row should have black background */
        if (cell->color.bg != COLOR_BLACK) {
            prints("[ERROR] Row 0 background color incorrect: expected %d, got %d\n", 
                   COLOR_BLACK, cell->color.bg);
            *passed = 0;
        }
    }
    
    /* Check sky row color */
    cell = render_get_cell(40, 2);
    if (cell != 0) {
        if (cell->color.bg != COLOR_SKY_BG) {
            prints("[ERROR] Sky row background color incorrect: expected %d, got %d\n", 
                   COLOR_SKY_BG, cell->color.bg);
            *passed = 0;
        }
    }
    
    /* Check layer 1 color */
    cell = render_get_cell(40, 5);
    if (cell != 0) {
        if (cell->color.bg != COLOR_LAYER1_BG) {
            prints("[ERROR] Layer 1 background color incorrect: expected %d, got %d\n", 
                   COLOR_LAYER1_BG, cell->color.bg);
            *passed = 0;
        }
    }
    
    if (*passed) {
        prints("[OK] Buffers initialized correctly with layer colors\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_clear(int *passed) {
    game_test_print_header(2, "render_clear() - Clear buffer with layer colors");
    
    *passed = 1;
    
    /* First set some cells to non-space characters */
    Color test_color = render_make_color(COLOR_RED, COLOR_BLUE);
    render_set_cell(10, 10, 'X', test_color);
    render_set_cell(20, 15, 'Y', test_color);
    
    /* Clear the buffer */
    render_clear();
    
    /* Verify cells are cleared to spaces */
    const ScreenCell* cell = render_get_cell(10, 10);
    if (cell != 0) {
        if (cell->character != ' ') {
            prints("[ERROR] Cell not cleared: expected ' ', got '%c'\n", cell->character);
            *passed = 0;
        }
        /* Should have layer color, not test color */
        if (cell->color.bg == COLOR_BLUE) {
            prints("[ERROR] Cell color not reset to layer color\n");
            *passed = 0;
        }
    }
    
    if (*passed) {
        prints("[OK] Buffer cleared correctly with layer colors\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_set_cell(int *passed) {
    game_test_print_header(3, "render_set_cell() - Individual cell writes");
    
    *passed = 1;
    
    render_clear();
    
    /* Test valid cell write */
    Color color = render_make_color(COLOR_YELLOW, COLOR_BLUE);
    render_set_cell(40, 12, 'A', color);
    
    const ScreenCell* cell = render_get_cell(40, 12);
    if (cell != 0) {
        if (cell->character != 'A') {
            prints("[ERROR] Character not set: expected 'A', got '%c'\n", cell->character);
            *passed = 0;
        }
        if (cell->color.fg != COLOR_YELLOW || cell->color.bg != COLOR_BLUE) {
            prints("[ERROR] Color not set correctly\n");
            *passed = 0;
        }
    }
    
    /* Test out of bounds write (should be rejected) */
    render_set_cell(-1, 0, 'X', color);
    render_set_cell(0, -1, 'X', color);
    render_set_cell(SCREEN_WIDTH, 0, 'X', color);
    render_set_cell(0, SCREEN_HEIGHT, 'X', color);
    
    /* If we reach here without crash, out of bounds handling works */
    prints("[OK] Out of bounds writes handled safely\n");
    
    if (*passed) {
        prints("[OK] Cell writes work correctly\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_put_string(int *passed) {
    game_test_print_header(4, "render_put_string() - String rendering");
    
    *passed = 1;
    
    render_clear();
    
    /* Test basic string write */
    render_put_string(5, 10, "Hello");
    
    const ScreenCell* cell = render_get_cell(5, 10);
    if (cell != 0 && cell->character != 'H') {
        prints("[ERROR] First character incorrect: expected 'H', got '%c'\n", cell->character);
        *passed = 0;
    }
    
    cell = render_get_cell(9, 10);
    if (cell != 0 && cell->character != 'o') {
        prints("[ERROR] Last character incorrect: expected 'o', got '%c'\n", cell->character);
        *passed = 0;
    }
    
    /* Test colored string write */
    Color color = render_make_color(COLOR_GREEN, COLOR_RED);
    render_put_string_colored(20, 10, "Test", color);
    
    cell = render_get_cell(20, 10);
    if (cell != 0) {
        if (cell->character != 'T') {
            prints("[ERROR] Colored string character incorrect\n");
            *passed = 0;
        }
        if (cell->color.fg != COLOR_GREEN || cell->color.bg != COLOR_RED) {
            prints("[ERROR] Colored string color incorrect\n");
            *passed = 0;
        }
    }
    
    /* Test string clipping at edge */
    render_put_string(75, 5, "CLIPPING");  /* Should only write 5 chars */
    
    cell = render_get_cell(79, 5);
    if (cell != 0 && cell->character != 'P') {
        prints("[WARN] Edge clipping may not work as expected\n");
    }
    
    if (*passed) {
        prints("[OK] String rendering works correctly\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_fill_rect(int *passed) {
    game_test_print_header(5, "render_fill_rect() - Rectangle filling");
    
    *passed = 1;
    
    render_clear();
    
    /* Fill a rectangle */
    Color color = render_make_color(COLOR_WHITE, COLOR_MAGENTA);
    render_fill_rect(10, 5, 5, 3, '#', color);
    
    /* Check corners and center */
    const ScreenCell* cell;
    
    cell = render_get_cell(10, 5);  /* Top-left */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Top-left corner not filled\n");
        *passed = 0;
    }
    
    cell = render_get_cell(14, 5);  /* Top-right */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Top-right corner not filled\n");
        *passed = 0;
    }
    
    cell = render_get_cell(12, 6);  /* Center */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Center not filled\n");
        *passed = 0;
    }
    
    cell = render_get_cell(14, 7);  /* Bottom-right */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Bottom-right corner not filled\n");
        *passed = 0;
    }
    
    /* Check outside rectangle is not filled */
    cell = render_get_cell(9, 5);
    if (cell != 0 && cell->character == '#') {
        prints("[ERROR] Outside rectangle was filled\n");
        *passed = 0;
    }
    
    if (*passed) {
        prints("[OK] Rectangle filling works correctly\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_draw_lines(int *passed) {
    game_test_print_header(6, "render_draw_horizontal/vertical_line() - Line drawing");
    
    *passed = 1;
    
    render_clear();
    
    Color color = render_make_color(COLOR_CYAN, COLOR_BLACK);
    
    /* Draw horizontal line */
    render_draw_horizontal_line(5, 10, 10, '-', color);
    
    const ScreenCell* cell = render_get_cell(5, 10);
    if (cell != 0 && cell->character != '-') {
        prints("[ERROR] Horizontal line start incorrect\n");
        *passed = 0;
    }
    
    cell = render_get_cell(14, 10);
    if (cell != 0 && cell->character != '-') {
        prints("[ERROR] Horizontal line end incorrect\n");
        *passed = 0;
    }
    
    /* Draw vertical line */
    render_draw_vertical_line(30, 5, 8, '|', color);
    
    cell = render_get_cell(30, 5);
    if (cell != 0 && cell->character != '|') {
        prints("[ERROR] Vertical line start incorrect\n");
        *passed = 0;
    }
    
    cell = render_get_cell(30, 12);
    if (cell != 0 && cell->character != '|') {
        prints("[ERROR] Vertical line end incorrect\n");
        *passed = 0;
    }
    
    if (*passed) {
        prints("[OK] Line drawing works correctly\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_colors(int *passed) {
    game_test_print_header(7, "Color management functions");
    
    *passed = 1;
    
    /* Test render_make_color */
    Color c = render_make_color(COLOR_RED, COLOR_GREEN);
    if (c.fg != COLOR_RED || c.bg != COLOR_GREEN) {
        prints("[ERROR] render_make_color() incorrect\n");
        *passed = 0;
    }
    
    /* Test render_get_layer_color for each region */
    Color layer_color;
    
    /* Status row */
    layer_color = render_get_layer_color(0);
    if (layer_color.bg != COLOR_STATUS_BG) {
        prints("[ERROR] Status row color incorrect\n");
        *passed = 0;
    }
    
    /* Sky */
    layer_color = render_get_layer_color(2);
    if (layer_color.bg != COLOR_SKY_BG) {
        prints("[ERROR] Sky color incorrect\n");
        *passed = 0;
    }
    
    /* Layer 1 */
    layer_color = render_get_layer_color(6);
    if (layer_color.bg != COLOR_LAYER1_BG) {
        prints("[ERROR] Layer 1 color incorrect\n");
        *passed = 0;
    }
    
    /* Layer 2 */
    layer_color = render_get_layer_color(11);
    if (layer_color.bg != COLOR_LAYER2_BG) {
        prints("[ERROR] Layer 2 color incorrect\n");
        *passed = 0;
    }
    
    /* Layer 3 */
    layer_color = render_get_layer_color(16);
    if (layer_color.bg != COLOR_LAYER3_BG) {
        prints("[ERROR] Layer 3 color incorrect\n");
        *passed = 0;
    }
    
    /* Layer 4 */
    layer_color = render_get_layer_color(21);
    if (layer_color.bg != COLOR_LAYER4_BG) {
        prints("[ERROR] Layer 4 color incorrect\n");
        *passed = 0;
    }
    
    if (*passed) {
        prints("[OK] Color management functions work correctly\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_present(int *passed) {
    game_test_print_header(8, "render_present() - Screen buffer write");
    
    *passed = 1;
    
    /* Draw something to the buffer */
    render_clear();
    
    Color color = render_make_color(COLOR_WHITE, COLOR_BLUE);
    render_put_string_colored(30, 12, "PRESENT TEST", color);
    
    /* Present to screen */
    render_present();
    
    /* If we get here without crash, it worked */
    prints("[OK] render_present() executed without errors\n");
    
    /* Short delay to see the result */
    game_test_wait(GAME_TEST_VISUAL_PAUSE);
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_numbers(int *passed) {
    game_test_print_header(9, "Number rendering functions");
    
    *passed = 1;
    
    render_clear();
    
    /* Test render_number */
    render_number(5, 5, 42, 5);
    
    const ScreenCell* cell = render_get_cell(9, 5);
    if (cell != 0 && cell->character != '2') {
        prints("[ERROR] render_number last digit incorrect: got '%c'\n", cell->character);
        *passed = 0;
    }
    
    /* Test render_number_padded (zero padding) */
    render_number_padded(5, 7, 7, 3);
    
    cell = render_get_cell(5, 7);
    if (cell != 0 && cell->character != '0') {
        prints("[ERROR] render_number_padded padding incorrect: got '%c'\n", cell->character);
        *passed = 0;
    }
    
    cell = render_get_cell(7, 7);
    if (cell != 0 && cell->character != '7') {
        prints("[ERROR] render_number_padded digit incorrect: got '%c'\n", cell->character);
        *passed = 0;
    }
    
    if (*passed) {
        prints("[OK] Number rendering works correctly\n");
    }
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_visual_patterns(int *passed) {
    game_test_print_header(10, "Visual pattern test");
    
    *passed = 1;
    
    prints("[INFO] Displaying visual patterns...\n");
    prints("[INFO] You should see colored layers and text.\n");
    
    /* Clear and set up a visual test */
    render_clear();
    
    /* Draw status bar text */
    Color status_color = render_make_color(COLOR_WHITE, COLOR_BLACK);
    render_put_string_colored(0, 0, "00:00", status_color);
    render_put_string_colored(70, 0, "30 FPS", status_color);
    
    /* Draw in sky area */
    Color sky_color = render_make_color(COLOR_YELLOW, COLOR_SKY_BG);
    render_put_string_colored(35, 2, "SKY AREA", sky_color);
    
    /* Draw layer labels */
    Color layer1_color = render_make_color(COLOR_WHITE, COLOR_LAYER1_BG);
    render_put_string_colored(35, 6, "LAYER 1", layer1_color);
    
    Color layer2_color = render_make_color(COLOR_WHITE, COLOR_LAYER2_BG);
    render_put_string_colored(35, 11, "LAYER 2", layer2_color);
    
    Color layer3_color = render_make_color(COLOR_WHITE, COLOR_LAYER3_BG);
    render_put_string_colored(35, 16, "LAYER 3", layer3_color);
    
    Color layer4_color = render_make_color(COLOR_WHITE, COLOR_LAYER4_BG);
    render_put_string_colored(35, 21, "LAYER 4", layer4_color);
    
    /* Draw some shapes */
    Color shape_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);
    render_fill_rect(5, 5, 10, 5, '#', shape_color);
    render_draw_horizontal_line(60, 10, 15, '=', shape_color);
    render_draw_vertical_line(70, 5, 10, '|', shape_color);
    
    /* Draw bottom status */
    Color lives_color = render_make_color(COLOR_RED, COLOR_BLACK);
    render_put_string_colored(0, 24, "VVV", lives_color);
    
    Color score_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);
    render_put_string_colored(31, 24, "SCORE: ", score_color);
    render_number_padded(38, 24, 12345, 5);
    
    Color round_color = render_make_color(COLOR_CYAN, COLOR_BLACK);
    render_put_string_colored(70, 24, "ROUND 1", round_color);
    
    /* Present to screen */
    render_present();
    
    prints("[INFO] Pattern displayed. Waiting for visual inspection...\n");
    
    /* Wait for visual inspection */
    game_test_wait(GAME_TEST_VISUAL_DISPLAY);
    
    prints("[OK] Visual pattern test completed\n");
    
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/****************************************/
/**    Main Test Entry Points          **/
/****************************************/

void render_buffer_tests(void) {
    prints("\n========================================\n");
    prints("     RENDER BUFFER TESTS (M5.6)         \n");
    prints("========================================\n");
    
    prints("[PID %d] [TID %d] Starting render buffer tests...\n", getpid(), gettid());
    
    game_subtests_run = 0;
    game_subtests_passed = 0;
    
    int result;
    
#if TEST_RENDER_INIT
    test_render_init(&result);
#endif

#if TEST_RENDER_CLEAR
    test_render_clear(&result);
#endif

#if TEST_RENDER_SET_CELL
    test_render_set_cell(&result);
#endif

#if TEST_RENDER_PUT_STRING
    test_render_put_string(&result);
#endif

#if TEST_RENDER_FILL_RECT
    test_render_fill_rect(&result);
#endif

#if TEST_RENDER_DRAW_LINES
    test_render_draw_lines(&result);
#endif

#if TEST_RENDER_COLORS
    test_render_colors(&result);
#endif

#if TEST_RENDER_PRESENT
    test_render_present(&result);
#endif

#if TEST_RENDER_NUMBERS
    test_render_numbers(&result);
#endif

#if TEST_RENDER_VISUAL
    test_render_visual_patterns(&result);
#endif

    /* Print summary */
    prints("\n========================================\n");
    prints("RENDER BUFFER TESTS: %d/%d subtests passed\n", 
           game_subtests_passed, game_subtests_run);
    prints("========================================\n");
    
    int all_passed = (game_subtests_passed == game_subtests_run);
    if (all_passed) {
        prints("==> RENDER BUFFER TESTS: PASSED\n");
    } else {
        prints("==> RENDER BUFFER TESTS: FAILED\n");
    }
}

void execute_game_tests(void) {
    prints("\n=========================================\n");
    prints("       GAME TEST SUITE                   \n");
    prints("=========================================\n\n");
    
    prints("[PID %d] [TID %d] Starting game tests...\n\n", getpid(), gettid());
    
#if RUN_GAME_TESTS
    /* Run render buffer tests (M5.6) */
    render_buffer_tests();
    
    /* Clear screen after tests */
    render_cleanup();
    
    prints("\n=========================================\n");
    prints("       GAME TEST SUITE COMPLETE          \n");
    prints("=========================================\n");
#else
    prints("[INFO] Game tests disabled (RUN_GAME_TESTS = 0)\n");
#endif
}
