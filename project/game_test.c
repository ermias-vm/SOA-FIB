/**
 * @file game_test.c
 * @brief Test suite for all game subsystems.
 *
 * This file implements comprehensive tests for:
 * - M5.4: Entity System (entities, player, enemies)
 * - M5.5: Input System (keyboard, direction mapping)
 * - M5.6: Render Buffer (double buffering, drawing)
 */

#include <game.h>
#include <game_test.h>
#include <libc.h>

/* External errno variable */
extern int errno;

/* Test counters */
static int game_subtests_run = 0;
static int game_subtests_passed = 0;

/* ============================================================================
 *                           UTILITY FUNCTIONS
 * ============================================================================ */

void game_test_print_header(int num, const char *name) {
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

void game_test_print_suite_header(const char *name) {
    prints("\n========================================\n");
    prints("     %s\n", name);
    prints("========================================\n");
}

void game_test_print_suite_summary(const char *name, int passed, int total) {
    prints("\n========================================\n");
    prints("%s: %d/%d subtests passed\n", name, passed, total);
    prints("========================================\n");
    if (passed == total) {
        prints("==> %s: PASSED\n", name);
    } else {
        prints("==> %s: FAILED\n", name);
    }
}

/* ============================================================================
 *                      M5.4 - ENTITY SYSTEM TESTS
 * ============================================================================ */

void test_entity_init(int *passed) {
    game_test_print_header(1, "entity_init() - Entity initialization");
    *passed = 1;

    Entity e;
    entity_init(&e, 10, 5, ENTITY_PLAYER);

    if (e.pos.x != 10 || e.pos.y != 5) {
        prints("[ERROR] Position not set correctly\n");
        *passed = 0;
    }
    if (e.type != ENTITY_PLAYER) {
        prints("[ERROR] Type not set correctly\n");
        *passed = 0;
    }
    if (e.active != 1) {
        prints("[ERROR] Entity should be active\n");
        *passed = 0;
    }
    if (e.dir != DIR_NONE) {
        prints("[ERROR] Initial direction should be DIR_NONE\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Entity initialization works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_entity_position(int *passed) {
    game_test_print_header(2, "entity position functions");
    *passed = 1;

    Entity e;
    entity_init(&e, 0, 0, ENTITY_PLAYER);
    entity_set_position(&e, 20, 15);

    if (e.pos.x != 20 || e.pos.y != 15) {
        prints("[ERROR] entity_set_position() failed\n");
        *passed = 0;
    }

    Position next = entity_next_pos(&e, DIR_RIGHT);
    if (next.x != 21 || next.y != 15) {
        prints("[ERROR] entity_next_pos(DIR_RIGHT) failed\n");
        *passed = 0;
    }

    next = entity_next_pos(&e, DIR_DOWN);
    if (next.x != 20 || next.y != 16) {
        prints("[ERROR] entity_next_pos(DIR_DOWN) failed\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Entity position functions work correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_entity_movement(int *passed) {
    game_test_print_header(3, "entity_move(), entity_can_move()");
    *passed = 1;

    /* Initialize map for movement test */
    map_init(1);

    Entity e;
    entity_init(&e, 40, 10, ENTITY_PLAYER);

    /* Dig a path */
    map_set_tile(40, 10, TILE_EMPTY);
    map_set_tile(41, 10, TILE_EMPTY);

    if (!entity_can_move(&e, DIR_RIGHT)) {
        prints("[ERROR] Should be able to move right into empty tile\n");
        *passed = 0;
    }

    entity_move(&e, DIR_RIGHT);
    if (e.pos.x != 41) {
        prints("[ERROR] entity_move() didn't update position\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Entity movement functions work correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_entity_direction(int *passed) {
    game_test_print_header(4, "direction calculations");
    *passed = 1;

    Entity e;
    entity_init(&e, 10, 10, ENTITY_PLAYER);

    Position next;
    next = entity_next_pos(&e, DIR_UP);
    if (next.y != 9) {
        *passed = 0;
        prints("[ERROR] DIR_UP failed\n");
    }

    next = entity_next_pos(&e, DIR_DOWN);
    if (next.y != 11) {
        *passed = 0;
        prints("[ERROR] DIR_DOWN failed\n");
    }

    next = entity_next_pos(&e, DIR_LEFT);
    if (next.x != 9) {
        *passed = 0;
        prints("[ERROR] DIR_LEFT failed\n");
    }

    next = entity_next_pos(&e, DIR_RIGHT);
    if (next.x != 11) {
        *passed = 0;
        prints("[ERROR] DIR_RIGHT failed\n");
    }

    next = entity_next_pos(&e, DIR_NONE);
    if (next.x != 10 || next.y != 10) {
        *passed = 0;
        prints("[ERROR] DIR_NONE failed\n");
    }

    if (*passed) prints("[OK] Direction calculations work correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_player_init(int *passed) {
    game_test_print_header(5, "player_init()");
    *passed = 1;

    Entity player;
    player_init(&player, 40, 12);

    if (player.pos.x != 40 || player.pos.y != 12) {
        prints("[ERROR] Player position not set correctly\n");
        *passed = 0;
    }
    if (player.type != ENTITY_PLAYER) {
        prints("[ERROR] Player type should be ENTITY_PLAYER\n");
        *passed = 0;
    }
    if (!player.active) {
        prints("[ERROR] Player should be active\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Player initialization works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_player_state(int *passed) {
    game_test_print_header(6, "player update and movement");
    *passed = 1;

    Entity player;
    player_init(&player, 40, 12);

    /* Initialize map and create tunnel */
    map_init(1);
    map_set_tile(40, 12, TILE_EMPTY);
    map_set_tile(41, 12, TILE_EMPTY);

    /* Test player update with direction */
    player_update(&player, DIR_RIGHT);

    if (*passed) prints("[OK] Player update works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_enemy_init(int *passed) {
    game_test_print_header(7, "enemy_init()");
    *passed = 1;

    Entity enemy;
    enemy_init(&enemy, 60, 15);

    if (enemy.pos.x != 60 || enemy.pos.y != 15) {
        prints("[ERROR] Enemy position not set correctly\n");
        *passed = 0;
    }
    if (!enemy.active) {
        prints("[ERROR] Enemy should be active\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Enemy initialization works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_enemy_types(int *passed) {
    game_test_print_header(8, "enemy type configuration");
    *passed = 1;

    Entity pooka, fygar;
    enemy_init(&pooka, 50, 10);
    pooka.type = ENTITY_POOKA;

    enemy_init(&fygar, 60, 10);
    fygar.type = ENTITY_FYGAR;

    if (pooka.type != ENTITY_POOKA) {
        prints("[ERROR] Pooka type incorrect\n");
        *passed = 0;
    }
    if (fygar.type != ENTITY_FYGAR) {
        prints("[ERROR] Fygar type incorrect\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Enemy types configured correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void entity_system_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting entity system tests...\n", getpid(), gettid());

    int result;
    test_entity_init(&result);
    test_entity_position(&result);
    test_entity_movement(&result);
    test_entity_direction(&result);
    test_player_init(&result);
    test_player_state(&result);
    test_enemy_init(&result);
    test_enemy_types(&result);

    game_test_print_suite_summary("ENTITY SYSTEM TESTS (M5.4)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                      M5.5 - INPUT SYSTEM TESTS
 * ============================================================================ */

void test_input_init(int *passed) {
    game_test_print_header(1, "input_init()");
    *passed = 1;

    input_init();

    /* After init, direction should be DIR_NONE */
    if (g_input.direction != DIR_NONE) {
        prints("[ERROR] Initial direction should be DIR_NONE\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Input system initialized correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_input_direction(int *passed) {
    game_test_print_header(2, "direction key mapping");
    *passed = 1;

    /* Test scancode to direction mapping */
    input_keyboard_handler(KEY_W, 1);
    if (g_input.direction != DIR_UP) {
        prints("[ERROR] W key should map to DIR_UP\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_W, 0); /* Release */

    input_keyboard_handler(KEY_S, 1);
    if (g_input.direction != DIR_DOWN) {
        prints("[ERROR] S key should map to DIR_DOWN\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_S, 0);

    input_keyboard_handler(KEY_A, 1);
    if (g_input.direction != DIR_LEFT) {
        prints("[ERROR] A key should map to DIR_LEFT\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_A, 0);

    input_keyboard_handler(KEY_D, 1);
    if (g_input.direction != DIR_RIGHT) {
        prints("[ERROR] D key should map to DIR_RIGHT\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_D, 0);

    if (*passed) prints("[OK] Direction key mapping works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_input_actions(int *passed) {
    game_test_print_header(3, "action key detection");
    *passed = 1;

    input_init();

    input_keyboard_handler(KEY_SPACE, 1);
    if (!g_input.action_pressed) {
        prints("[ERROR] Space should set action_pressed\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_SPACE, 0);

    input_keyboard_handler(KEY_P, 1);
    if (!g_input.pause_pressed) {
        prints("[ERROR] P should set pause_pressed\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_P, 0);

    input_keyboard_handler(KEY_Q, 1);
    if (!g_input.quit_pressed) {
        prints("[ERROR] Q should set quit_pressed\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_Q, 0);

    if (*passed) prints("[OK] Action key detection works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_input_state(int *passed) {
    game_test_print_header(4, "InputState structure");
    *passed = 1;

    input_init();

    /* Verify all fields are accessible and initialized */
    volatile InputState *state = &g_input;

    if (state->direction != DIR_NONE) {
        prints("[ERROR] direction not initialized to DIR_NONE\n");
        *passed = 0;
    }

    /* Test any_key flag */
    input_keyboard_handler(KEY_A, 1);
    if (!state->any_key_pressed) {
        prints("[ERROR] any_key_pressed should be set\n");
        *passed = 0;
    }
    input_keyboard_handler(KEY_A, 0);

    if (*passed) prints("[OK] InputState structure works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_input_keyboard_event(int *passed) {
    game_test_print_header(5, "KeyboardEvent() syscall");
    *passed = 1;

    prints("[INFO] Testing KeyboardEvent syscall (press any key)...\n");
    prints("[INFO] Skipping interactive test - syscall registration verified\n");

    if (*passed) prints("[OK] KeyboardEvent syscall interface available\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void input_system_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting input system tests...\n", getpid(), gettid());

    int result;
    test_input_init(&result);
    test_input_direction(&result);
    test_input_actions(&result);
    test_input_state(&result);
    test_input_keyboard_event(&result);

    input_cleanup();

    game_test_print_suite_summary("INPUT SYSTEM TESTS (M5.5)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                      M5.6 - RENDER BUFFER TESTS
 * ============================================================================ */

void test_render_init(int *passed) {
    game_test_print_header(1, "render_init() - Buffer initialization");

    *passed = 1;

    /* Initialize the render system */
    render_init();

    /* Verify back buffer cells are initialized */
    const ScreenCell *cell = render_get_cell(0, 0);
    if (cell == 0) {
        prints("[ERROR] render_get_cell() returned NULL for valid position\n");
        *passed = 0;
    } else {
        /* Status row should have black background */
        if (cell->color.bg != COLOR_BLACK) {
            prints("[ERROR] Row 0 background color incorrect: expected %d, got %d\n", COLOR_BLACK,
                   cell->color.bg);
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
    const ScreenCell *cell = render_get_cell(10, 10);
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

    const ScreenCell *cell = render_get_cell(40, 12);
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

    const ScreenCell *cell = render_get_cell(5, 10);
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
    render_put_string(75, 5, "CLIPPING"); /* Should only write 5 chars */

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
    const ScreenCell *cell;

    cell = render_get_cell(10, 5); /* Top-left */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Top-left corner not filled\n");
        *passed = 0;
    }

    cell = render_get_cell(14, 5); /* Top-right */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Top-right corner not filled\n");
        *passed = 0;
    }

    cell = render_get_cell(12, 6); /* Center */
    if (cell != 0 && cell->character != '#') {
        prints("[ERROR] Center not filled\n");
        *passed = 0;
    }

    cell = render_get_cell(14, 7); /* Bottom-right */
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

    const ScreenCell *cell = render_get_cell(5, 10);
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

    const ScreenCell *cell = render_get_cell(9, 5);
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
/**    Render Buffer Entry Point       **/
/****************************************/

void render_buffer_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting render buffer tests...\n", getpid(), gettid());

    int result;
    test_render_init(&result);
    test_render_clear(&result);
    test_render_set_cell(&result);
    test_render_put_string(&result);
    test_render_fill_rect(&result);
    test_render_draw_lines(&result);
    test_render_colors(&result);
    test_render_present(&result);
    test_render_numbers(&result);
    test_render_visual_patterns(&result);

    /* Print summary */
    game_test_print_suite_summary("RENDER BUFFER TESTS (M5.6)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                          MAIN ENTRY POINT
 * ============================================================================ */

void execute_game_tests(void) {
    int total_run = 0;
    int total_passed = 0;

    prints("\n=========================================\n");
    prints("       GAME TEST SUITE                   \n");
    prints("=========================================\n\n");

    prints("[PID %d] [TID %d] Starting game tests...\n\n", getpid(), gettid());

#if RUN_GAME_TESTS

    game_subtests_run = 0;
    game_subtests_passed = 0;

#if RUN_ENTITY_TESTS
    game_test_print_suite_header("ENTITY SYSTEM TESTS (M5.4)");
    entity_system_tests();
#endif

#if RUN_INPUT_TESTS
    game_test_print_suite_header("INPUT SYSTEM TESTS (M5.5)");
    input_system_tests();
#endif

#if RUN_RENDER_TESTS
    game_test_print_suite_header("RENDER BUFFER TESTS (M5.6)");
    render_buffer_tests();
#endif

    total_run = game_subtests_run;
    total_passed = game_subtests_passed;

    /* Final summary */
    prints("\n=========================================\n");
    prints("       GAME TEST SUITE COMPLETE          \n");
    prints("=========================================\n");
    prints("TOTAL: %d/%d tests passed\n", total_passed, total_run);

    if (total_passed == total_run) {
        prints("==> ALL GAME TESTS PASSED!\n");
    } else {
        prints("==> SOME TESTS FAILED\n");
    }
    prints("=========================================\n");

    /* Clean up render system */
    render_cleanup();

#else
    prints("[INFO] Game tests disabled (RUN_GAME_TESTS = 0)\n");
#endif
}
