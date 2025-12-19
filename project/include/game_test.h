/**
 * @file game_test.h
 * @brief Test suite for game subsystems (M5.4-M5.10).
 *
 * Milestones:
 *   M5.4 - Entity System (player, enemies, movement)
 *   M5.5 - Input System (keyboard events, direction mapping)
 *   M5.6 - Render Buffer (double buffering, screen output)
 *   M5.7 - UI System (HUD, menus, overlays)
 *   M5.8 - Game Logic (player logic, enemy AI, collisions, scoring)
 *   M5.9 - Game Render (game rendering functions)
 *   M5.10 - Game Data (level definitions, spawn, tunnels)
 */

#ifndef __GAME_TEST_H__
#define __GAME_TEST_H__

#include <game_config.h>
#include <game_data.h>
#include <game_entities.h>
#include <game_input.h>
#include <game_logic.h>
#include <game_render.h>
#include <game_types.h>
#include <game_ui.h>

/* ============================================================================
 *                              TEST SWITCHES
 * ============================================================================ */

// clang-format off
#define RUN_GAME_TESTS      1   /**< Master switch: Enable/disable ALL game tests */

#define RUN_ENTITY_TESTS    1   /**< M5.4: Entity system tests */
#define RUN_INPUT_TESTS     1   /**< M5.5: Input system tests */
#define RUN_RENDER_TESTS    1   /**< M5.6: Render buffer tests */
#define RUN_UI_TESTS        1   /**< M5.7: UI system tests */
#define RUN_LOGIC_TESTS     1   /**< M5.8: Game logic tests */
#define RUN_RENDER_GAME_TESTS 1 /**< M5.9: Game rendering tests */
#define RUN_DATA_TESTS      1   /**< M5.10: Game data/level tests */
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
 *                      M5.7 - UI SYSTEM TEST FUNCTIONS
 * ============================================================================ */

void test_ui_strlen(int *passed);
void test_ui_number_to_string(int *passed);
void test_ui_draw_hud(int *passed);
void test_ui_draw_time(int *passed);
void test_ui_draw_lives(int *passed);
void test_ui_draw_score(int *passed);
void test_ui_draw_round(int *passed);
void test_ui_draw_centered_text(int *passed);
void test_ui_draw_border(int *passed);
void test_ui_draw_menu_screen(int *passed);
void test_ui_draw_pause_screen(int *passed);
void test_ui_draw_game_over_screen(int *passed);
void test_ui_draw_level_clear_screen(int *passed);
void test_ui_draw_victory_screen(int *passed);
void test_ui_flash_effects(int *passed);
void test_ui_clear_game_area(int *passed);
void ui_system_tests(void);

/* ============================================================================
 *                      M5.8 - GAME LOGIC TEST FUNCTIONS
 * ============================================================================ */

void test_logic_init(int *passed);
void test_logic_player_init(int *passed);
void test_logic_enemy_init(int *passed);
void test_logic_rock_init(int *passed);
void test_logic_player_move(int *passed);
void test_logic_player_pump(int *passed);
void test_logic_enemy_ai(int *passed);
void test_logic_enemy_inflate_deflate(int *passed);
void test_logic_rock_fall(int *passed);
void test_logic_collision_detection(int *passed);
void test_logic_pump_hit(int *passed);
void test_logic_scoring(int *passed);
void test_logic_game_state(int *passed);
void test_logic_fygar_fire(int *passed);
void game_logic_tests(void);

/* ============================================================================
 *                      M5.9 - GAME RENDER TEST FUNCTIONS
 * ============================================================================ */

void test_render_game_init(int *passed);
void test_render_map_basic(int *passed);
void test_render_player_display(int *passed);
void test_render_enemy_display(int *passed);
void test_render_rock_display(int *passed);
void test_render_pump_display(int *passed);
void test_render_explosion_effect(int *passed);
void test_render_fire_effect(int *passed);
void test_render_game_complete(int *passed);
void game_render_tests(void);

/* ============================================================================
 *                      M5.10 - GAME DATA TEST FUNCTIONS
 * ============================================================================ */

void test_data_get_level(int *passed);
void test_data_get_num_levels(int *passed);
void test_data_level_validity(int *passed);
void test_data_spawn_enemies(int *passed);
void test_data_spawn_rocks(int *passed);
void test_data_create_tunnels(int *passed);
void test_data_load_level(int *passed);
void game_data_tests(void);

/* ============================================================================
 *                          MAIN ENTRY POINT
 * ============================================================================ */

/**
 * @brief Execute all enabled game test suites.
 */
void execute_game_tests(void);

#endif /* __GAME_TEST_H__ */
