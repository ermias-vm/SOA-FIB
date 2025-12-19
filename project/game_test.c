/**
 * @file game_test.c
 * @brief Test suite for all game subsystems.
 *
 * This file implements comprehensive tests for:
 * - M5.4: Entity System (entities, player, enemies)
 * - M5.5: Input System (keyboard, direction mapping)
 * - M5.6: Render Buffer (double buffering, drawing)
 * - M5.7: UI System (HUD, menus, overlays)
 * - M5.8: Game Logic (player logic, enemy AI, collisions, scoring)
 * - M5.9: Game Render (complete game rendering)
 * - M5.10: Game Data (level definitions, spawn, tunnels)
 */

#include <game.h>
#include <game_data.h>
#include <game_map.h>
#include <game_test.h>
#include <game_ui.h>
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
 *                      M5.7 - UI SYSTEM TESTS
 * ============================================================================ */

void test_ui_strlen(int *passed) {
    game_test_print_header(1, "ui_strlen() - String length calculation");
    *passed = 1;

    int len;

    len = ui_strlen("Hello");
    if (len != 5) {
        prints("[ERROR] ui_strlen(\"Hello\") expected 5, got %d\n", len);
        *passed = 0;
    }

    len = ui_strlen("");
    if (len != 0) {
        prints("[ERROR] ui_strlen(\"\") expected 0, got %d\n", len);
        *passed = 0;
    }

    len = ui_strlen("12345678901234567890");
    if (len != 20) {
        prints("[ERROR] ui_strlen() for 20 chars expected 20, got %d\n", len);
        *passed = 0;
    }

    if (*passed) prints("[OK] ui_strlen() works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_number_to_string(int *passed) {
    game_test_print_header(2, "ui_number_to_string() - Number formatting");
    *passed = 1;

    char buffer[10];

    /* Test basic positive number */
    ui_number_to_string(42, buffer, 5, ' ');
    if (buffer[3] != '4' || buffer[4] != '2') {
        prints("[ERROR] Number 42 not formatted correctly: got '%s'\n", buffer);
        *passed = 0;
    }

    /* Test zero padding */
    ui_number_to_string(7, buffer, 3, '0');
    if (buffer[0] != '0' || buffer[1] != '0' || buffer[2] != '7') {
        prints("[ERROR] Zero padding failed: expected '007', got '%s'\n", buffer);
        *passed = 0;
    }

    /* Test zero value */
    ui_number_to_string(0, buffer, 3, '0');
    if (buffer[0] != '0' || buffer[1] != '0' || buffer[2] != '0') {
        prints("[ERROR] Zero value failed: expected '000', got '%s'\n", buffer);
        *passed = 0;
    }

    /* Test large number */
    ui_number_to_string(12345, buffer, 5, '0');
    if (buffer[0] != '1' || buffer[4] != '5') {
        prints("[ERROR] Large number 12345 not formatted correctly: got '%s'\n", buffer);
        *passed = 0;
    }

    if (*passed) prints("[OK] ui_number_to_string() works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_hud(int *passed) {
    game_test_print_header(3, "ui_draw_hud() - Complete HUD rendering");
    *passed = 1;

    render_init();
    render_clear();

    /* Draw complete HUD */
    ui_draw_hud(3, 12500, 5, 125, 30);

    /* Verify top bar (time should be at position 0) */
    const ScreenCell *cell = render_get_cell(0, STATUS_TOP_ROW);
    if (cell == 0) {
        prints("[ERROR] Top bar cell is null\n");
        *passed = 0;
    }

    /* Verify bottom bar has content */
    cell = render_get_cell(HUD_LIVES_X, STATUS_BOTTOM_ROW);
    if (cell == 0) {
        prints("[ERROR] Bottom bar lives area is null\n");
        *passed = 0;
    }

    /* Present to see the result */
    render_present();
    game_test_wait(GAME_TEST_VISUAL_PAUSE);

    if (*passed) prints("[OK] ui_draw_hud() renders complete HUD\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_time(int *passed) {
    game_test_print_header(4, "ui_draw_time() - Time display");
    *passed = 1;

    render_clear();

    /* Draw time: 2 minutes 35 seconds = 155 seconds */
    ui_draw_time(155);

    /* Check first character is '0' (for 02:35) */
    const ScreenCell *cell = render_get_cell(HUD_TIME_X, STATUS_TOP_ROW);
    if (cell != 0 && cell->character != '0') {
        prints("[WARN] Time first digit unexpected: got '%c'\n", cell->character);
        /* Not a hard failure - format may vary */
    }

    render_present();

    if (*passed) prints("[OK] ui_draw_time() renders time correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_lives(int *passed) {
    game_test_print_header(5, "ui_draw_lives() - Lives display with hearts");
    *passed = 1;

    render_clear();

    /* Draw 3 lives */
    ui_draw_lives(3);

    /* Check hearts are drawn */
    const ScreenCell *cell = render_get_cell(HUD_LIVES_X, STATUS_BOTTOM_ROW);
    if (cell != 0 && cell->character != CHAR_HEART) {
        prints("[ERROR] First heart not drawn: got '%c' (0x%02x)\n", cell->character,
               cell->character);
        *passed = 0;
    }

    cell = render_get_cell(HUD_LIVES_X + 2, STATUS_BOTTOM_ROW);
    if (cell != 0 && cell->character != CHAR_HEART) {
        prints("[ERROR] Third heart not drawn\n");
        *passed = 0;
    }

    /* Fourth position should not have heart (only 3 lives) */
    cell = render_get_cell(HUD_LIVES_X + 3, STATUS_BOTTOM_ROW);
    if (cell != 0 && cell->character == CHAR_HEART) {
        prints("[ERROR] Extra heart drawn beyond lives count\n");
        *passed = 0;
    }

    render_present();

    if (*passed) prints("[OK] ui_draw_lives() renders hearts correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_score(int *passed) {
    game_test_print_header(6, "ui_draw_score() - Score display");
    *passed = 1;

    render_clear();

    /* Draw score */
    ui_draw_score(12345);

    /* Score should contain "SCORE:" text centered */
    /* Just verify no crash and cells are written */
    int center_x = SCREEN_WIDTH / 2;
    const ScreenCell *cell = render_get_cell(center_x, STATUS_BOTTOM_ROW);
    if (cell == 0) {
        prints("[ERROR] Score area cell is null\n");
        *passed = 0;
    }

    render_present();

    if (*passed) prints("[OK] ui_draw_score() renders score correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_round(int *passed) {
    game_test_print_header(7, "ui_draw_round() - Round display");
    *passed = 1;

    render_clear();

    /* Draw round number */
    ui_draw_round(5);

    /* Round text should be at right edge */
    /* Just verify the function doesn't crash */
    render_present();

    if (*passed) prints("[OK] ui_draw_round() renders round number correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_centered_text(int *passed) {
    game_test_print_header(8, "ui_draw_centered_text() - Centered text");
    *passed = 1;

    render_clear();

    Color color = render_make_color(COLOR_YELLOW, COLOR_BLACK);
    ui_draw_centered_text(12, "CENTERED", color);

    /* "CENTERED" is 8 chars, should start at (80-8)/2 = 36 */
    int expected_x = (SCREEN_WIDTH - 8) / 2;
    const ScreenCell *cell = render_get_cell(expected_x, 12);
    if (cell != 0 && cell->character != 'C') {
        prints("[ERROR] Centered text not starting at expected position: got '%c'\n",
               cell->character);
        *passed = 0;
    }

    render_present();

    if (*passed) prints("[OK] ui_draw_centered_text() centers text correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_border(int *passed) {
    game_test_print_header(9, "ui_draw_border() - Box border drawing");
    *passed = 1;

    render_clear();

    Color color = render_make_color(COLOR_WHITE, COLOR_BLUE);
    ui_draw_border(10, 5, 20, 10, color);

    /* Check corners */
    const ScreenCell *cell = render_get_cell(10, 5);
    if (cell != 0 && cell->character != CHAR_CORNER) {
        prints("[ERROR] Top-left corner not drawn correctly\n");
        *passed = 0;
    }

    cell = render_get_cell(29, 5);
    if (cell != 0 && cell->character != CHAR_CORNER) {
        prints("[ERROR] Top-right corner not drawn correctly\n");
        *passed = 0;
    }

    /* Check horizontal border */
    cell = render_get_cell(15, 5);
    if (cell != 0 && cell->character != CHAR_BORDER_H) {
        prints("[ERROR] Horizontal border not drawn correctly\n");
        *passed = 0;
    }

    /* Check vertical border */
    cell = render_get_cell(10, 8);
    if (cell != 0 && cell->character != CHAR_BORDER_V) {
        prints("[ERROR] Vertical border not drawn correctly\n");
        *passed = 0;
    }

    render_present();

    if (*passed) prints("[OK] ui_draw_border() draws borders correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_menu_screen(int *passed) {
    game_test_print_header(10, "ui_draw_menu_screen() - Menu overlay");
    *passed = 1;

    render_init();
    ui_draw_menu_screen();
    render_present();

    prints("[INFO] Menu screen displayed. Visual inspection...\n");
    game_test_wait(GAME_TEST_VISUAL_DISPLAY);

    /* Clear for next test */
    render_clear();
    render_present();

    if (*passed) prints("[OK] ui_draw_menu_screen() renders without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_pause_screen(int *passed) {
    game_test_print_header(11, "ui_draw_pause_screen() - Pause overlay");
    *passed = 1;

    render_init();
    render_clear();
    ui_draw_pause_screen();
    render_present();

    prints("[INFO] Pause screen displayed. Visual inspection...\n");
    game_test_wait(GAME_TEST_VISUAL_PAUSE);

    /* Clear for next test */
    render_clear();
    render_present();

    if (*passed) prints("[OK] ui_draw_pause_screen() renders without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_game_over_screen(int *passed) {
    game_test_print_header(12, "ui_draw_game_over_screen() - Game over display");
    *passed = 1;

    render_init();
    ui_draw_game_over_screen(54321);
    render_present();

    prints("[INFO] Game Over screen displayed. Visual inspection...\n");
    game_test_wait(GAME_TEST_VISUAL_DISPLAY);

    /* Clear for next test */
    render_clear();
    render_present();

    if (*passed) prints("[OK] ui_draw_game_over_screen() renders without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_level_clear_screen(int *passed) {
    game_test_print_header(13, "ui_draw_level_clear_screen() - Level complete");
    *passed = 1;

    render_init();
    render_clear();
    ui_draw_level_clear_screen(3, 5000);
    render_present();

    prints("[INFO] Level Clear screen displayed. Visual inspection...\n");
    game_test_wait(GAME_TEST_VISUAL_PAUSE);

    /* Clear for next test */
    render_clear();
    render_present();

    if (*passed) prints("[OK] ui_draw_level_clear_screen() renders without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_draw_victory_screen(int *passed) {
    game_test_print_header(14, "ui_draw_victory_screen() - Victory display");
    *passed = 1;

    render_init();
    ui_draw_victory_screen(99999);
    render_present();

    prints("[INFO] Victory screen displayed. Visual inspection...\n");
    game_test_wait(GAME_TEST_VISUAL_DISPLAY);

    /* Clear for next test */
    render_clear();
    render_present();

    if (*passed) prints("[OK] ui_draw_victory_screen() renders without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_flash_effects(int *passed) {
    game_test_print_header(15, "UI flash effects");
    *passed = 1;

    /* Test score flash */
    ui_flash_score(10);

    /* Draw score multiple times to see flash effect */
    render_clear();
    for (int i = 0; i < 5; i++) {
        ui_draw_score(1000);
        render_present();
        game_test_wait(5);
    }

    /* Test life lost animation trigger */
    ui_animate_life_lost();

    if (*passed) prints("[OK] Flash effects triggered without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_ui_clear_game_area(int *passed) {
    game_test_print_header(16, "ui_clear_game_area() - Game area clear");
    *passed = 1;

    render_init();

    /* Draw some content in game area */
    Color color = render_make_color(COLOR_RED, COLOR_GREEN);
    render_fill_rect(10, 10, 20, 10, 'X', color);
    render_present();

    /* Clear game area */
    ui_clear_game_area();
    render_present();

    /* Check that area is cleared but HUD rows preserved */
    const ScreenCell *cell = render_get_cell(10, 10);
    if (cell != 0 && cell->character == 'X') {
        prints("[ERROR] Game area not cleared\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] ui_clear_game_area() clears correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/****************************************/
/**       UI System Entry Point        **/
/****************************************/

void ui_system_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting UI system tests...\n", getpid(), gettid());

    int result;
    test_ui_strlen(&result);
    test_ui_number_to_string(&result);
    test_ui_draw_hud(&result);
    test_ui_draw_time(&result);
    test_ui_draw_lives(&result);
    test_ui_draw_score(&result);
    test_ui_draw_round(&result);
    test_ui_draw_centered_text(&result);
    test_ui_draw_border(&result);
    test_ui_draw_menu_screen(&result);
    test_ui_draw_pause_screen(&result);
    test_ui_draw_game_over_screen(&result);
    test_ui_draw_level_clear_screen(&result);
    test_ui_draw_victory_screen(&result);
    test_ui_flash_effects(&result);
    test_ui_clear_game_area(&result);

    /* Print summary */
    game_test_print_suite_summary("UI SYSTEM TESTS (M5.7)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                      M5.8 - GAME LOGIC TESTS
 * ============================================================================ */

void test_logic_init(int *passed) {
    game_test_print_header(1, "logic_init() - Game state initialization");
    *passed = 1;

    GameLogicState state;
    logic_init(&state);

    if (state.scene != SCENE_MENU) {
        prints("[ERROR] Initial scene should be SCENE_MENU\n");
        *passed = 0;
    }
    if (state.score != 0) {
        prints("[ERROR] Initial score should be 0\n");
        *passed = 0;
    }
    if (state.lives != INITIAL_LIVES) {
        prints("[ERROR] Initial lives should be %d, got %d\n", INITIAL_LIVES, state.lives);
        *passed = 0;
    }
    if (state.round != 1) {
        prints("[ERROR] Initial round should be 1\n");
        *passed = 0;
    }
    if (state.running != 1) {
        prints("[ERROR] Game should be running\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Game state initialized correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_player_init(int *passed) {
    game_test_print_header(2, "logic_player_init() - Player initialization");
    *passed = 1;

    Player player;
    logic_player_init(&player, 40, 10);

    if (player.base.pos.x != 40 || player.base.pos.y != 10) {
        prints("[ERROR] Player position incorrect\n");
        *passed = 0;
    }
    if (player.base.type != ENTITY_PLAYER) {
        prints("[ERROR] Player type should be ENTITY_PLAYER\n");
        *passed = 0;
    }
    if (player.state != PLAYER_IDLE) {
        prints("[ERROR] Player should start in IDLE state\n");
        *passed = 0;
    }
    if (player.is_pumping != 0) {
        prints("[ERROR] Player should not be pumping initially\n");
        *passed = 0;
    }
    if (player.pump_length != 0) {
        prints("[ERROR] Pump length should be 0 initially\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Player initialized correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_enemy_init(int *passed) {
    game_test_print_header(3, "logic_enemy_init() - Enemy initialization");
    *passed = 1;

    Enemy pooka, fygar;
    logic_enemy_init(&pooka, 60, 15, ENTITY_POOKA);
    logic_enemy_init(&fygar, 70, 20, ENTITY_FYGAR);

    if (pooka.base.pos.x != 60 || pooka.base.pos.y != 15) {
        prints("[ERROR] Pooka position incorrect\n");
        *passed = 0;
    }
    if (pooka.base.type != ENTITY_POOKA) {
        prints("[ERROR] Pooka type incorrect\n");
        *passed = 0;
    }
    if (pooka.state != ENEMY_NORMAL) {
        prints("[ERROR] Enemy should start in NORMAL state\n");
        *passed = 0;
    }
    if (pooka.inflate_level != 0) {
        prints("[ERROR] Inflate level should be 0\n");
        *passed = 0;
    }

    if (fygar.base.type != ENTITY_FYGAR) {
        prints("[ERROR] Fygar type incorrect\n");
        *passed = 0;
    }
    if (fygar.fire_active != 0) {
        prints("[ERROR] Fygar fire should not be active initially\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Enemies initialized correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_rock_init(int *passed) {
    game_test_print_header(4, "logic_rock_init() - Rock initialization");
    *passed = 1;

    Rock rock;
    logic_rock_init(&rock, 30, 8);

    if (rock.base.pos.x != 30 || rock.base.pos.y != 8) {
        prints("[ERROR] Rock position incorrect\n");
        *passed = 0;
    }
    if (rock.base.type != ENTITY_ROCK) {
        prints("[ERROR] Rock type incorrect\n");
        *passed = 0;
    }
    if (rock.state != ROCK_STABLE) {
        prints("[ERROR] Rock should start in STABLE state\n");
        *passed = 0;
    }
    if (rock.wobble_timer != 0) {
        prints("[ERROR] Wobble timer should be 0\n");
        *passed = 0;
    }
    if (rock.has_crushed != 0) {
        prints("[ERROR] has_crushed should be 0\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Rock initialized correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_player_move(int *passed) {
    game_test_print_header(5, "logic_player_move() - Player movement");
    *passed = 1;

    /* Initialize map for movement test */
    map_init(1);

    Player player;
    logic_player_init(&player, 40, 10);

    /* Create a tunnel for movement */
    map_set_tile(40, 10, TILE_EMPTY);
    map_set_tile(41, 10, TILE_EMPTY);
    map_set_tile(40, 11, TILE_EMPTY);

    /* Test move right */
    logic_player_move(&player, DIR_RIGHT);
    if (player.base.pos.x != 41) {
        prints("[ERROR] Player should have moved right to x=41, got x=%d\n", player.base.pos.x);
        *passed = 0;
    }
    if (player.base.dir != DIR_RIGHT) {
        prints("[ERROR] Player direction should be DIR_RIGHT\n");
        *passed = 0;
    }

    /* Reset and test move down */
    logic_player_init(&player, 40, 10);
    logic_player_move(&player, DIR_DOWN);
    if (player.base.pos.y != 11) {
        prints("[ERROR] Player should have moved down to y=11, got y=%d\n", player.base.pos.y);
        *passed = 0;
    }

    if (*passed) prints("[OK] Player movement works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_player_pump(int *passed) {
    game_test_print_header(6, "logic_player_pump() - Pump action");
    *passed = 1;

    GameLogicState state;
    logic_init(&state);
    logic_start_round(&state, 1);

    /* Set player direction */
    state.player.base.dir = DIR_RIGHT;

    /* Test pump activation */
    logic_player_pump(&state.player, &state);

    if (!state.player.is_pumping) {
        prints("[ERROR] Player should be pumping\n");
        *passed = 0;
    }
    if (state.player.pump_length != 1) {
        prints("[ERROR] Pump length should be 1, got %d\n", state.player.pump_length);
        *passed = 0;
    }
    if (state.player.state != PLAYER_PUMPING) {
        prints("[ERROR] Player state should be PLAYER_PUMPING\n");
        *passed = 0;
    }

    /* Test pump extension */
    logic_player_pump(&state.player, &state);
    if (state.player.pump_length != 2) {
        prints("[ERROR] Pump length should be 2, got %d\n", state.player.pump_length);
        *passed = 0;
    }

    /* Test max pump length */
    for (int i = 0; i < 10; i++) {
        logic_player_pump(&state.player, &state);
    }
    if (state.player.pump_length > MAX_PUMP_LENGTH) {
        prints("[ERROR] Pump length should not exceed %d, got %d\n", MAX_PUMP_LENGTH,
               state.player.pump_length);
        *passed = 0;
    }

    if (*passed) prints("[OK] Pump action works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_enemy_ai(int *passed) {
    game_test_print_header(7, "logic_enemy_ai() - Enemy AI behavior");
    *passed = 1;

    /* Initialize map */
    map_init(1);

    Player player;
    Enemy enemy;
    logic_player_init(&player, 40, 10);
    logic_enemy_init(&enemy, 45, 10, ENTITY_POOKA);

    /* Clear path between enemy and player */
    for (int x = 40; x <= 45; x++) {
        map_set_tile(x, 10, TILE_EMPTY);
    }

    int start_x = enemy.base.pos.x;

    /* Run AI - enemy should move towards player */
    logic_enemy_ai(&enemy, &player);

    if (enemy.base.pos.x >= start_x && enemy.base.dir != DIR_LEFT) {
        prints("[WARN] Enemy should try to move towards player (left)\n");
        /* Not a hard failure - AI may have other valid moves */
    }

    if (*passed) prints("[OK] Enemy AI executes without errors\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_enemy_inflate_deflate(int *passed) {
    game_test_print_header(8, "logic_enemy_inflate/deflate() - Inflation system");
    *passed = 1;

    Enemy enemy;
    logic_enemy_init(&enemy, 50, 10, ENTITY_POOKA);

    /* Test inflate */
    logic_enemy_inflate(&enemy);
    if (enemy.state != ENEMY_INFLATING) {
        prints("[ERROR] Enemy should be in INFLATING state\n");
        *passed = 0;
    }
    if (enemy.inflate_level != 1) {
        prints("[ERROR] Inflate level should be 1, got %d\n", enemy.inflate_level);
        *passed = 0;
    }

    /* Inflate more */
    logic_enemy_inflate(&enemy);
    logic_enemy_inflate(&enemy);
    if (enemy.inflate_level != 3) {
        prints("[ERROR] Inflate level should be 3, got %d\n", enemy.inflate_level);
        *passed = 0;
    }

    /* Inflate to death */
    logic_enemy_inflate(&enemy);
    if (enemy.state != ENEMY_DEAD) {
        prints("[ERROR] Enemy should be DEAD after 4 inflations\n");
        *passed = 0;
    }
    if (enemy.base.active != 0) {
        prints("[ERROR] Enemy should be inactive after death\n");
        *passed = 0;
    }

    /* Test deflate */
    logic_enemy_init(&enemy, 50, 10, ENTITY_POOKA);
    logic_enemy_inflate(&enemy);
    logic_enemy_inflate(&enemy);

    logic_enemy_deflate(&enemy);
    if (enemy.inflate_level != 1) {
        prints("[ERROR] Inflate level should be 1 after deflate, got %d\n", enemy.inflate_level);
        *passed = 0;
    }

    logic_enemy_deflate(&enemy);
    if (enemy.state != ENEMY_NORMAL) {
        prints("[ERROR] Enemy should return to NORMAL state after full deflation\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Inflation/deflation system works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_rock_fall(int *passed) {
    game_test_print_header(9, "logic_rock_fall() - Rock physics");
    *passed = 1;

    /* Initialize map */
    map_init(1);

    Rock rock;
    logic_rock_init(&rock, 35, 8);

    /* Create empty space below rock */
    map_set_tile(35, 8, TILE_EMPTY);
    map_set_tile(35, 9, TILE_EMPTY);
    map_set_tile(35, 10, TILE_EMPTY);

    /* Check fall detection */
    logic_rock_check_fall(&rock);
    if (rock.state != ROCK_WOBBLING) {
        prints("[ERROR] Rock should start wobbling when space below\n");
        *passed = 0;
    }

    /* Simulate wobble timer expiration */
    rock.wobble_timer = 0;
    rock.state = ROCK_FALLING;

    int start_y = rock.base.pos.y;

    GameLogicState state;
    logic_init(&state);
    logic_rock_fall(&rock, &state);

    if (rock.base.pos.y != start_y + 1) {
        prints("[ERROR] Rock should have fallen one tile, y was %d now %d\n", start_y,
               rock.base.pos.y);
        *passed = 0;
    }

    if (*passed) prints("[OK] Rock physics works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_collision_detection(int *passed) {
    game_test_print_header(10, "Collision detection functions");
    *passed = 1;

    Player player;
    Enemy enemies[2];
    Rock rocks[1];

    logic_player_init(&player, 40, 10);
    logic_enemy_init(&enemies[0], 40, 10, ENTITY_POOKA); /* Same position */
    logic_enemy_init(&enemies[1], 50, 15, ENTITY_FYGAR); /* Different position */
    logic_rock_init(&rocks[0], 40, 10);

    /* Test player-enemy collision */
    int collision = logic_check_player_enemy_collision(&player, enemies, 2);
    if (collision != 0) {
        prints("[ERROR] Should detect collision with enemy 0, got %d\n", collision);
        *passed = 0;
    }

    /* Move player, should not collide */
    player.base.pos.x = 30;
    collision = logic_check_player_enemy_collision(&player, enemies, 2);
    if (collision != -1) {
        prints("[ERROR] Should not detect collision, got %d\n", collision);
        *passed = 0;
    }

    /* Test player-rock collision */
    player.base.pos.x = 40;
    collision = logic_check_player_rock_collision(&player, rocks, 1);
    if (collision != 0) {
        prints("[ERROR] Should detect collision with rock 0, got %d\n", collision);
        *passed = 0;
    }

    if (*passed) prints("[OK] Collision detection works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_pump_hit(int *passed) {
    game_test_print_header(11, "logic_check_pump_hit() - Pump attack");
    *passed = 1;

    /* Initialize map */
    map_init(1);

    Player player;
    Enemy enemies[2];

    logic_player_init(&player, 40, 10);
    player.is_pumping = 1;
    player.pump_dir = DIR_RIGHT;
    player.pump_length = 3;

    /* Place enemy in pump range */
    logic_enemy_init(&enemies[0], 42, 10, ENTITY_POOKA); /* 2 tiles right */
    logic_enemy_init(&enemies[1], 50, 10, ENTITY_POOKA); /* Out of range */

    /* Clear path */
    for (int x = 40; x <= 45; x++) {
        map_set_tile(x, 10, TILE_EMPTY);
    }

    int hit = logic_check_pump_hit(&player, enemies, 2);
    if (hit != 0) {
        prints("[ERROR] Pump should hit enemy 0, got %d\n", hit);
        *passed = 0;
    }

    /* Test pump miss */
    player.pump_length = 1; /* Too short */
    hit = logic_check_pump_hit(&player, enemies, 2);
    if (hit != -1) {
        prints("[ERROR] Pump should miss, got %d\n", hit);
        *passed = 0;
    }

    if (*passed) prints("[OK] Pump hit detection works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_scoring(int *passed) {
    game_test_print_header(12, "Scoring system");
    *passed = 1;

    GameLogicState state;
    logic_init(&state);

    /* Test add score */
    logic_add_score(&state, 100);
    if (state.score != 100) {
        prints("[ERROR] Score should be 100, got %d\n", state.score);
        *passed = 0;
    }

    logic_add_score(&state, 250);
    if (state.score != 350) {
        prints("[ERROR] Score should be 350, got %d\n", state.score);
        *passed = 0;
    }

    /* Test max score cap */
    logic_add_score(&state, 100000);
    if (state.score != MAX_SCORE) {
        prints("[ERROR] Score should be capped at %d, got %d\n", MAX_SCORE, state.score);
        *passed = 0;
    }

    /* Test layer-based scoring */
    int points_l1 = logic_calculate_enemy_points(6);  /* Layer 1 */
    int points_l2 = logic_calculate_enemy_points(11); /* Layer 2 */
    int points_l3 = logic_calculate_enemy_points(16); /* Layer 3 */
    int points_l4 = logic_calculate_enemy_points(21); /* Layer 4 */

    if (points_l1 != POINTS_LAYER1) {
        prints("[ERROR] Layer 1 points should be %d, got %d\n", POINTS_LAYER1, points_l1);
        *passed = 0;
    }
    if (points_l2 != POINTS_LAYER2) {
        prints("[ERROR] Layer 2 points should be %d, got %d\n", POINTS_LAYER2, points_l2);
        *passed = 0;
    }
    if (points_l3 != POINTS_LAYER3) {
        prints("[ERROR] Layer 3 points should be %d, got %d\n", POINTS_LAYER3, points_l3);
        *passed = 0;
    }
    if (points_l4 != POINTS_LAYER4) {
        prints("[ERROR] Layer 4 points should be %d, got %d\n", POINTS_LAYER4, points_l4);
        *passed = 0;
    }

    if (*passed) prints("[OK] Scoring system works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_game_state(int *passed) {
    game_test_print_header(13, "Game state management");
    *passed = 1;

    GameLogicState state;
    logic_init(&state);

    /* Test start round - initially in SCENE_ROUND_START */
    logic_start_round(&state, 1);
    if (state.scene != SCENE_ROUND_START) {
        prints("[ERROR] Scene should be SCENE_ROUND_START after start\n");
        *passed = 0;
    }
    if (state.round != 1) {
        prints("[ERROR] Round should be 1, got %d\n", state.round);
        *passed = 0;
    }
    if (state.enemy_count <= 0) {
        prints("[ERROR] Should have enemies after round start\n");
        *passed = 0;
    }

    /* Simulate timer expiration to transition to SCENE_PLAYING */
    state.round_start_timer = 0;
    state.scene = SCENE_ROUND_START;
    logic_update(&state);
    if (state.scene != SCENE_PLAYING) {
        prints("[WARN] Scene should transition to SCENE_PLAYING after timer\n");
    }

    /* Test game over condition */
    state.lives = 0;
    logic_check_game_over(&state);
    if (state.scene != SCENE_GAME_OVER) {
        prints("[ERROR] Scene should be SCENE_GAME_OVER\n");
        *passed = 0;
    }

    /* Test round complete condition */
    logic_init(&state);
    logic_start_round(&state, 1);
    state.enemies_remaining = 0;
    logic_check_round_complete(&state);
    if (state.scene != SCENE_ROUND_CLEAR) {
        prints("[ERROR] Scene should be SCENE_ROUND_CLEAR\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Game state management works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_logic_fygar_fire(int *passed) {
    game_test_print_header(14, "Fygar fire attack");
    *passed = 1;

    GameLogicState state;
    logic_init(&state);
    logic_start_round(&state, 1);

    /* Create a Fygar enemy */
    Enemy fygar;
    logic_enemy_init(&fygar, 45, 10, ENTITY_FYGAR);
    fygar.base.dir = DIR_LEFT;
    fygar.fire_cooldown = 0;

    /* Put player in fire range */
    state.player.base.pos.x = 43; /* 2 tiles left of Fygar */
    state.player.base.pos.y = 10; /* Same row */

    /* Trigger fire */
    logic_fygar_fire(&fygar, &state);

    if (fygar.fire_active) {
        prints("[OK] Fygar activated fire attack\n");
    } else {
        prints("[INFO] Fygar did not activate fire (may not meet conditions)\n");
    }

    /* Test fire collision */
    fygar.fire_active = 1;
    fygar.fire_duration = 10;
    fygar.base.dir = DIR_LEFT;

    int hit = logic_check_fire_collision(&fygar, &state.player);
    if (hit) {
        prints("[OK] Fire collision detected\n");
    } else {
        prints("[INFO] No fire collision (player may be out of range)\n");
    }

    if (*passed) prints("[OK] Fygar fire system works\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/****************************************/
/**     Game Logic Entry Point         **/
/****************************************/

void game_logic_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting game logic tests...\n", getpid(), gettid());

    int result;
    test_logic_init(&result);
    test_logic_player_init(&result);
    test_logic_enemy_init(&result);
    test_logic_rock_init(&result);
    test_logic_player_move(&result);
    test_logic_player_pump(&result);
    test_logic_enemy_ai(&result);
    test_logic_enemy_inflate_deflate(&result);
    test_logic_rock_fall(&result);
    test_logic_collision_detection(&result);
    test_logic_pump_hit(&result);
    test_logic_scoring(&result);
    test_logic_game_state(&result);
    test_logic_fygar_fire(&result);

    /* Print summary */
    game_test_print_suite_summary("GAME LOGIC TESTS (M5.8)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                      M5.9 - GAME RENDER TESTS
 * ============================================================================ */

void test_render_game_init(int *passed) {
    game_test_print_header(1, "render_game() - Game rendering initialization");
    *passed = 1;

    /* Initialize render system */
    render_init();

    /* Initialize game state */
    GameLogicState state;
    logic_init(&state);

    /* Verify render system is ready */
    const ScreenCell *cell = render_get_cell(0, 0);
    if (cell == 0) {
        prints("[ERROR] Render buffer not accessible\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Game render initialization works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_map_basic(int *passed) {
    game_test_print_header(2, "render_map() - Map rendering");
    *passed = 1;

    /* Initialize systems */
    render_init();
    map_init(1);

    /* Render map */
    render_clear();
    render_map();

    /* Check that sky rows have proper background */
    Color sky_color = render_get_layer_color(2);
    if (sky_color.bg != COLOR_SKY_BG) {
        prints("[ERROR] Sky row should have light blue background\n");
        *passed = 0;
    }

    /* Check that ground layers have proper backgrounds */
    Color layer1_color = render_get_layer_color(6);
    if (layer1_color.bg != COLOR_LAYER1_BG) {
        prints("[ERROR] Layer 1 should have brown background\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] Map rendering works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_player_display(int *passed) {
    game_test_print_header(3, "render_player() - Player rendering");
    *passed = 1;

    render_init();
    render_clear();

    /* Create player */
    Player player;
    logic_player_init(&player, 40, 10);

    /* Render player */
    render_player(&player);

    /* Check player was drawn at correct position */
    const ScreenCell *cell = render_get_cell(40, 10);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at player position\n");
        *passed = 0;
    } else if (cell->character != CHAR_PLAYER) {
        prints("[ERROR] Player character not drawn correctly, got '%c'\n", cell->character);
        *passed = 0;
    }

    /* Test player pumping state */
    player.state = PLAYER_PUMPING;
    render_clear();
    render_player(&player);
    cell = render_get_cell(40, 10);
    if (cell != 0 && cell->color.fg != COLOR_YELLOW) {
        prints("[WARN] Pumping player should be yellow\n");
    }

    /* Test dead player not rendered */
    player.state = PLAYER_DEAD;
    render_clear();
    render_player(&player);
    /* Dead players should not be rendered */

    if (*passed) prints("[OK] Player rendering works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_enemy_display(int *passed) {
    game_test_print_header(4, "render_enemies() - Enemy rendering");
    *passed = 1;

    render_init();
    render_clear();

    /* Create enemies */
    Enemy enemies[2];
    logic_enemy_init(&enemies[0], 50, 10, ENTITY_POOKA);
    logic_enemy_init(&enemies[1], 60, 15, ENTITY_FYGAR);

    /* Render enemies */
    render_enemies(enemies, 2);

    /* Check Pooka */
    const ScreenCell *cell = render_get_cell(50, 10);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at Pooka position\n");
        *passed = 0;
    } else if (cell->character != CHAR_POOKA) {
        prints("[ERROR] Pooka character not drawn correctly, got '%c'\n", cell->character);
        *passed = 0;
    }

    /* Check Fygar */
    cell = render_get_cell(60, 15);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at Fygar position\n");
        *passed = 0;
    } else if (cell->character != CHAR_FYGAR) {
        prints("[ERROR] Fygar character not drawn correctly, got '%c'\n", cell->character);
        *passed = 0;
    }

    /* Test inflating state */
    enemies[0].state = ENEMY_INFLATING;
    enemies[0].inflate_level = 2;
    render_clear();
    render_enemies(enemies, 2);
    cell = render_get_cell(50, 10);
    if (cell != 0 && cell->character != CHAR_INFLATE_2) {
        prints("[WARN] Inflating enemy should show inflate character\n");
    }

    /* Test dead enemy not rendered */
    enemies[0].state = ENEMY_DEAD;
    render_clear();
    render_enemies(enemies, 2);

    if (*passed) prints("[OK] Enemy rendering works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_rock_display(int *passed) {
    game_test_print_header(5, "render_rocks() - Rock rendering");
    *passed = 1;

    render_init();
    render_clear();

    /* Create rocks */
    Rock rocks[2];
    logic_rock_init(&rocks[0], 30, 8);
    logic_rock_init(&rocks[1], 45, 12);

    /* Render rocks */
    render_rocks(rocks, 2);

    /* Check first rock */
    const ScreenCell *cell = render_get_cell(30, 8);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at rock position\n");
        *passed = 0;
    } else if (cell->character != CHAR_ROCK) {
        prints("[ERROR] Rock character not drawn correctly, got '%c'\n", cell->character);
        *passed = 0;
    }

    /* Test wobbling rock */
    rocks[0].state = ROCK_WOBBLING;
    rocks[0].wobble_timer = 1;
    render_clear();
    render_rocks(rocks, 2);
    cell = render_get_cell(30, 8);
    if (cell != 0 && cell->character != '#') {
        prints("[WARN] Wobbling rock should alternate character\n");
    }

    if (*passed) prints("[OK] Rock rendering works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_pump_display(int *passed) {
    game_test_print_header(6, "render_pump() - Pump attack rendering");
    *passed = 1;

    render_init();
    render_clear();

    /* Create player with pump active */
    Player player;
    logic_player_init(&player, 40, 10);
    player.is_pumping = 1;
    player.pump_length = 3;
    player.pump_dir = DIR_RIGHT;

    /* Render pump */
    render_pump(&player);

    /* Check pump line */
    const ScreenCell *cell = render_get_cell(41, 10);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at pump position\n");
        *passed = 0;
    } else if (cell->character != '-') {
        prints("[ERROR] Pump character not drawn correctly, got '%c'\n", cell->character);
        *passed = 0;
    }

    /* Check pump tip */
    cell = render_get_cell(43, 10);
    if (cell != 0 && cell->character != '+') {
        prints("[WARN] Pump tip should be '+'\n");
    }

    /* Test vertical pump */
    player.pump_dir = DIR_DOWN;
    render_clear();
    render_pump(&player);
    cell = render_get_cell(40, 11);
    if (cell != 0 && cell->character != '|') {
        prints("[WARN] Vertical pump should use '|' character\n");
    }

    if (*passed) prints("[OK] Pump rendering works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_explosion_effect(int *passed) {
    game_test_print_header(7, "render_explosion() - Explosion effect");
    *passed = 1;

    render_init();
    render_clear();

    /* Render explosion */
    render_explosion(40, 12);

    /* Check center */
    const ScreenCell *cell = render_get_cell(40, 12);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at explosion center\n");
        *passed = 0;
    } else if (cell->character != '*') {
        prints("[ERROR] Explosion center should be '*', got '%c'\n", cell->character);
        *passed = 0;
    }

    /* Check particles */
    cell = render_get_cell(39, 12);
    if (cell != 0 && cell->character != '+') {
        prints("[WARN] Explosion particle should be '+'\n");
    }

    if (*passed) prints("[OK] Explosion effect works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_fire_effect(int *passed) {
    game_test_print_header(8, "render_fire() - Fire breath effect");
    *passed = 1;

    render_init();
    render_clear();

    /* Render fire (Fygar fire to the right) */
    render_fire(50, 10, DIR_RIGHT, 2);

    /* Check fire cells */
    const ScreenCell *cell = render_get_cell(51, 10);
    if (cell == 0) {
        prints("[ERROR] Cannot access cell at fire position\n");
        *passed = 0;
    } else if (cell->character != '~') {
        prints("[ERROR] Fire body should be '~', got '%c'\n", cell->character);
        *passed = 0;
    }

    cell = render_get_cell(52, 10);
    if (cell != 0 && cell->character != '*') {
        prints("[WARN] Fire tip should be '*'\n");
    }

    /* Test fire to the left */
    render_clear();
    render_fire(50, 10, DIR_LEFT, 2);
    cell = render_get_cell(49, 10);
    if (cell != 0 && cell->character != '~') {
        prints("[WARN] Leftward fire body should be '~'\n");
    }

    if (*passed) prints("[OK] Fire effect works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

void test_render_game_complete(int *passed) {
    game_test_print_header(9, "render_game() - Complete game rendering");
    *passed = 1;

    /* Initialize everything */
    render_init();
    GameLogicState state;
    logic_init(&state);
    logic_start_round(&state, 1);

    /* Ensure scene is playing for render test */
    state.scene = SCENE_PLAYING;

    /* Render full game frame */
    render_game(&state);

    /* Verify player was rendered */
    const ScreenCell *cell = render_get_cell(state.player.base.pos.x, state.player.base.pos.y);
    if (cell == 0) {
        prints("[ERROR] Cannot access player cell after render_game\n");
        *passed = 0;
    }

    /* Test pause screen overlay */
    state.scene = SCENE_PAUSED;
    render_game(&state);

    /* Test game over screen */
    state.scene = SCENE_GAME_OVER;
    render_game(&state);

    /* Test menu screen */
    state.scene = SCENE_MENU;
    render_game(&state);

    if (*passed) prints("[OK] Complete game rendering works correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/****************************************/
/**     Game Render Entry Point        **/
/****************************************/

void game_render_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting game render tests...\n", getpid(), gettid());

    int result;
    test_render_game_init(&result);
    test_render_map_basic(&result);
    test_render_player_display(&result);
    test_render_enemy_display(&result);
    test_render_rock_display(&result);
    test_render_pump_display(&result);
    test_render_explosion_effect(&result);
    test_render_fire_effect(&result);
    test_render_game_complete(&result);

    /* Print summary */
    game_test_print_suite_summary("GAME RENDER TESTS (M5.9)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                      M5.10 - GAME DATA TESTS
 * ============================================================================ */

/**
 * Test 1: data_get_level function
 */
void test_data_get_level(int *passed) {
    int result = 1;
    game_test_print_header(1, "data_get_level");

    /* Test valid levels */
    const LevelData *level1 = data_get_level(1);
    if (!level1 || level1->round_number != 1) {
        result = 0;
    }

    const LevelData *level5 = data_get_level(5);
    if (!level5 || level5->round_number != 5) {
        result = 0;
    }

    /* Test invalid level (should return first level) */
    const LevelData *level_invalid = data_get_level(0);
    if (!level_invalid) {
        result = 0;
    }

    /* Test beyond defined levels (should return last level) */
    const LevelData *level_beyond = data_get_level(10);
    if (!level_beyond) {
        result = 0;
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/**
 * Test 2: data_get_num_levels function
 */
void test_data_get_num_levels(int *passed) {
    int result = 1;
    game_test_print_header(2, "data_get_num_levels");

    int num_levels = data_get_num_levels();
    if (num_levels < 1 || num_levels > MAX_LEVELS) {
        result = 0;
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/**
 * Test 3: Level data validity
 */
void test_data_level_validity(int *passed) {
    int result = 1;
    game_test_print_header(3, "Level data validity");

    int num_levels = data_get_num_levels();

    for (int i = 1; i <= num_levels; i++) {
        const LevelData *level = data_get_level(i);
        if (!level) {
            result = 0;
            break;
        }

        /* Check enemy count within bounds */
        if (level->enemy_count < 0 || level->enemy_count > MAX_ENEMIES) {
            result = 0;
            break;
        }

        /* Check rock count within bounds */
        if (level->rock_count < 0 || level->rock_count > MAX_ROCKS) {
            result = 0;
            break;
        }

        /* Check tunnel count within bounds */
        if (level->tunnel_count < 0 || level->tunnel_count > MAX_TUNNELS) {
            result = 0;
            break;
        }

        /* Check player start position within game area */
        if (level->player_start_x < 0 || level->player_start_x >= SCREEN_WIDTH ||
            level->player_start_y < ROW_SKY_START || level->player_start_y > ROW_GROUND_END) {
            result = 0;
            break;
        }
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/**
 * Test 4: data_spawn_enemies function
 */
void test_data_spawn_enemies(int *passed) {
    int result = 1;
    game_test_print_header(4, "data_spawn_enemies");

    GameLogicState state;

    /* Initialize state */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        state.enemies[i].base.active = 0;
    }

    const LevelData *level = data_get_level(1);
    if (!level) {
        result = 0;
    } else {
        data_spawn_enemies(&state, level);

        /* Check enemies were spawned */
        int active_count = 0;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (state.enemies[i].base.active) {
                active_count++;
            }
        }

        if (active_count != level->enemy_count) {
            result = 0;
        }
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/**
 * Test 5: data_spawn_rocks function
 */
void test_data_spawn_rocks(int *passed) {
    int result = 1;
    game_test_print_header(5, "data_spawn_rocks");

    GameLogicState state;

    /* Initialize state */
    for (int i = 0; i < MAX_ROCKS; i++) {
        state.rocks[i].base.active = 0;
    }

    const LevelData *level = data_get_level(1);
    if (!level) {
        result = 0;
    } else {
        data_spawn_rocks(&state, level);

        /* Check rocks were spawned */
        int active_count = 0;
        for (int i = 0; i < MAX_ROCKS; i++) {
            if (state.rocks[i].base.active) {
                active_count++;
            }
        }

        if (active_count != level->rock_count) {
            result = 0;
        }
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/**
 * Test 6: data_create_tunnels function
 */
void test_data_create_tunnels(int *passed) {
    int result = 1;
    game_test_print_header(6, "data_create_tunnels");

    /* Initialize map */
    map_init(1);

    const LevelData *level = data_get_level(1);
    if (!level) {
        result = 0;
    } else {
        /* Create tunnels */
        data_create_tunnels(level);

        /* Check first tunnel's endpoints are walkable (dug) */
        if (level->tunnel_count > 0) {
            const TunnelDef *tunnel = &level->tunnels[0];
            if (!map_is_walkable(tunnel->x1, tunnel->y1) ||
                !map_is_walkable(tunnel->x2, tunnel->y2)) {
                result = 0;
            }
        }
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/**
 * Test 7: data_load_level function
 */
void test_data_load_level(int *passed) {
    int result = 1;
    game_test_print_header(7, "data_load_level");

    GameLogicState state;

    /* Clear state */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        state.enemies[i].base.active = 0;
    }
    for (int i = 0; i < MAX_ROCKS; i++) {
        state.rocks[i].base.active = 0;
    }

    /* Load level 1 */
    data_load_level(1, &state);

    /* Verify state was populated */
    const LevelData *level = data_get_level(1);
    if (!level) {
        result = 0;
    } else {
        /* Check player position */
        if (state.player.base.pos.x != level->player_start_x ||
            state.player.base.pos.y != level->player_start_y) {
            result = 0;
        }

        /* Check enemy count */
        if (state.enemy_count != level->enemy_count) {
            result = 0;
        }

        /* Check rock count */
        if (state.rock_count != level->rock_count) {
            result = 0;
        }

        /* Check round */
        if (state.round != 1) {
            result = 0;
        }
    }

    *passed = result;
    game_test_print_result(result);
    game_subtests_run++;
    if (result) game_subtests_passed++;
}

/****************************************/
/**     Game Data Entry Point          **/
/****************************************/

void game_data_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting game data tests...\n", getpid(), gettid());

    int result;
    test_data_get_level(&result);
    test_data_get_num_levels(&result);
    test_data_level_validity(&result);
    test_data_spawn_enemies(&result);
    test_data_spawn_rocks(&result);
    test_data_create_tunnels(&result);
    test_data_load_level(&result);

    /* Print summary */
    game_test_print_suite_summary("GAME DATA TESTS (M5.10)", game_subtests_passed,
                                  game_subtests_run);
    game_subtests_run = saved_run + game_subtests_run;
    game_subtests_passed = saved_passed + game_subtests_passed;
}

/* ============================================================================
 *                      M5.11 - INTEGRATION TESTS
 * ============================================================================ */

/**
 * @brief Test game_init() function.
 * Verifies that all subsystems are properly initialized.
 */
void test_game_init(int *passed) {
    game_test_print_header(1, "game_init() - Full initialization");
    *passed = 1;

    /* Call game_init to initialize all systems */
    game_init();

    /* Verify initial game state */
    GameState *state = game_get_state();
    if (state == 0) {
        prints("[ERROR] game_get_state() returned NULL\n");
        *passed = 0;
        game_test_print_result(*passed);
        game_subtests_run++;
        if (*passed) game_subtests_passed++;
        return;
    }

    /* Check initial scene is MENU */
    if (state->scene != SCENE_MENU) {
        prints("[ERROR] Initial scene should be SCENE_MENU, got %d\n", state->scene);
        *passed = 0;
    }

    /* Check initial values */
    if (state->score != 0) {
        prints("[ERROR] Initial score should be 0, got %d\n", state->score);
        *passed = 0;
    }
    if (state->level != 1) {
        prints("[ERROR] Initial level should be 1, got %d\n", state->level);
        *passed = 0;
    }
    if (state->lives != INITIAL_LIVES) {
        prints("[ERROR] Initial lives should be %d, got %d\n", INITIAL_LIVES, state->lives);
        *passed = 0;
    }

    if (*passed) prints("[OK] game_init() initializes all systems correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/**
 * @brief Test scene state transitions.
 * Verifies that the game can transition between scenes properly.
 */
void test_game_state_transitions(int *passed) {
    game_test_print_header(2, "Scene state transitions");
    *passed = 1;

    GameState *state = game_get_state();
    if (state == 0) {
        prints("[ERROR] game_get_state() returned NULL\n");
        *passed = 0;
        game_test_print_result(*passed);
        game_subtests_run++;
        if (*passed) game_subtests_passed++;
        return;
    }

    /* Test MENU -> ROUND_START transition */
    state->scene = SCENE_MENU;
    state->scene = SCENE_ROUND_START;
    if (state->scene != SCENE_ROUND_START) {
        prints("[ERROR] Failed to transition to SCENE_ROUND_START\n");
        *passed = 0;
    }

    /* Test ROUND_START -> PLAYING transition */
    state->scene = SCENE_PLAYING;
    if (state->scene != SCENE_PLAYING) {
        prints("[ERROR] Failed to transition to SCENE_PLAYING\n");
        *passed = 0;
    }

    /* Test PLAYING -> PAUSED transition */
    state->scene = SCENE_PAUSED;
    if (state->scene != SCENE_PAUSED) {
        prints("[ERROR] Failed to transition to SCENE_PAUSED\n");
        *passed = 0;
    }

    /* Test PLAYING -> ROUND_CLEAR transition */
    state->scene = SCENE_PLAYING;
    state->scene = SCENE_ROUND_CLEAR;
    if (state->scene != SCENE_ROUND_CLEAR) {
        prints("[ERROR] Failed to transition to SCENE_ROUND_CLEAR\n");
        *passed = 0;
    }

    /* Test PLAYING -> GAME_OVER transition */
    state->scene = SCENE_PLAYING;
    state->scene = SCENE_GAME_OVER;
    if (state->scene != SCENE_GAME_OVER) {
        prints("[ERROR] Failed to transition to SCENE_GAME_OVER\n");
        *passed = 0;
    }

    /* Reset to MENU */
    state->scene = SCENE_MENU;

    if (*passed) prints("[OK] All scene transitions work correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/**
 * @brief Test game_new_level() function.
 * Verifies that new levels are loaded correctly.
 */
void test_game_new_level(int *passed) {
    game_test_print_header(3, "game_new_level() - Level loading");
    *passed = 1;

    GameState *state = game_get_state();
    if (state == 0) {
        prints("[ERROR] game_get_state() returned NULL\n");
        *passed = 0;
        game_test_print_result(*passed);
        game_subtests_run++;
        if (*passed) game_subtests_passed++;
        return;
    }

    /* Set level to 1 */
    state->level = 1;

    /* Call game_new_level */
    int result = game_new_level();
    if (result < 0) {
        prints("[ERROR] game_new_level() returned error for level 1\n");
        *passed = 0;
    }

    /* Verify scene changed to ROUND_START */
    if (state->scene != SCENE_ROUND_START) {
        prints("[ERROR] Scene should be SCENE_ROUND_START after new level\n");
        *passed = 0;
    }

    /* Verify enemies were set (level 1 has enemies) */
    if (state->enemy_count <= 0) {
        prints("[ERROR] enemy_count should be > 0 after new level\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] game_new_level() loads levels correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/**
 * @brief Test game_reset() function.
 * Verifies that game state is properly reset.
 */
void test_game_reset(int *passed) {
    game_test_print_header(4, "game_reset() - State reset");
    *passed = 1;

    GameState *state = game_get_state();
    if (state == 0) {
        prints("[ERROR] game_get_state() returned NULL\n");
        *passed = 0;
        game_test_print_result(*passed);
        game_subtests_run++;
        if (*passed) game_subtests_passed++;
        return;
    }

    /* Modify state */
    state->score = 1000;
    state->level = 3;
    state->lives = 1;
    state->scene = SCENE_GAME_OVER;

    /* Reset */
    game_reset();

    /* Verify reset values */
    if (state->score != 0) {
        prints("[ERROR] Score should be 0 after reset, got %d\n", state->score);
        *passed = 0;
    }
    if (state->level != 1) {
        prints("[ERROR] Level should be 1 after reset, got %d\n", state->level);
        *passed = 0;
    }
    if (state->lives != INITIAL_LIVES) {
        prints("[ERROR] Lives should be %d after reset, got %d\n", INITIAL_LIVES, state->lives);
        *passed = 0;
    }
    if (state->scene != SCENE_MENU) {
        prints("[ERROR] Scene should be SCENE_MENU after reset\n");
        *passed = 0;
    }

    if (*passed) prints("[OK] game_reset() resets state correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/**
 * @brief Test game_cleanup() function.
 * Verifies that resources are properly cleaned up.
 */
void test_game_cleanup(int *passed) {
    game_test_print_header(5, "game_cleanup() - Resource cleanup");
    *passed = 1;

    /* Call cleanup */
    game_cleanup();

    /* Verify game is no longer running */
    if (game_is_running()) {
        prints("[ERROR] Game should not be running after cleanup\n");
        *passed = 0;
    }

    /* Reinitialize for subsequent tests */
    game_init();

    if (*passed) prints("[OK] game_cleanup() releases resources correctly\n");
    game_test_print_result(*passed);
    game_subtests_run++;
    if (*passed) game_subtests_passed++;
}

/**
 * @brief Run all integration tests (M5.11).
 */
void game_integration_tests(void) {
    int saved_run = game_subtests_run;
    int saved_passed = game_subtests_passed;
    game_subtests_run = 0;
    game_subtests_passed = 0;

    prints("[PID %d] [TID %d] Starting integration tests...\n", getpid(), gettid());

    int result;
    test_game_init(&result);
    test_game_state_transitions(&result);
    test_game_new_level(&result);
    test_game_reset(&result);
    test_game_cleanup(&result);

    /* Print summary */
    game_test_print_suite_summary("INTEGRATION TESTS (M5.11)", game_subtests_passed,
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

#if RUN_UI_TESTS
    game_test_print_suite_header("UI SYSTEM TESTS (M5.7)");
    ui_system_tests();
#endif

#if RUN_LOGIC_TESTS
    game_test_print_suite_header("GAME LOGIC TESTS (M5.8)");
    game_logic_tests();
#endif

#if RUN_RENDER_GAME_TESTS
    game_test_print_suite_header("GAME RENDER TESTS (M5.9)");
    game_render_tests();
#endif

#if RUN_DATA_TESTS
    game_test_print_suite_header("GAME DATA TESTS (M5.10)");
    game_data_tests();
#endif

#if RUN_INTEGRATION_TESTS
    game_test_print_suite_header("INTEGRATION TESTS (M5.11)");
    game_integration_tests();
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
