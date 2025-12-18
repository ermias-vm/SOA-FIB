#ifndef __GAME_H__
#define __GAME_H__

#include <game_config.h>
#include <game_entities.h>
#include <game_input.h>
#include <game_map.h>
#include <game_render.h>
#include <game_types.h>
#include <types.h>

/**
 * @file game.h
 * @brief Main game header that includes all subsystems and defines global state
 *
 * NOTE: GameState and InputState are defined in game_types.h
 * This file only declares the global variables and function prototypes.
 * DO NOT redefine these structures here!
 */

/* Forward declarations */
struct task_struct;

/* ============================================================================
 *                            GLOBAL VARIABLES
 * ============================================================================ */

/**
 * @brief Global game state variable (type defined in game_types.h).
 */
extern volatile GameState g_game;

/**
 * @brief Global input state variable (type defined in game_types.h).
 */
extern volatile InputState g_input;

/* ============================================================================
 *                         THREAD SYNCHRONIZATION
 * ============================================================================ */

/**
 * @brief Frame ready flag for thread synchronization.
 */
extern volatile int g_frame_ready;

/**
 * @brief Game running flag for thread control.
 */
extern volatile int g_running;

/* ============================================================================
 *                            MAIN GAME FUNCTIONS
 * ============================================================================ */

/**
 * @brief Main entry point for the game.
 *
 * This function is called from user.c to start the game. It initializes
 * all subsystems, creates the render thread, and runs the main game loop.
 * Does not return until the game exits.
 */
void game_main(void);

/**
 * @brief Initialize all game subsystems.
 *
 * Initializes the game state, input system, map system, entities,
 * rendering system, and UI system. Must be called before game_run().
 */
void game_init(void);

/**
 * @brief Clean up game resources and subsystems.
 *
 * Cleans up any allocated resources, stops threads, and resets
 * the system to a clean state. Called when exiting the game.
 */
void game_cleanup(void);

/**
 * @brief Main game loop controller.
 *
 * Manages the overall game flow, handling scene transitions
 * (menu -> playing -> game over -> menu). Runs until game exits.
 */
void game_run(void);

/* ============================================================================
 *                             THREAD FUNCTIONS
 * ============================================================================ */

/**
 * @brief Logic thread main function.
 *
 * This function runs in the main thread and handles all game logic:
 * - Player input processing
 * - Entity updates (player and enemies)
 * - Collision detection
 * - Game state transitions
 * - Level management
 *
 * @param arg Thread argument (unused, can be NULL)
 */
void logic_thread_func(void *arg);

/**
 * @brief Render thread main function.
 *
 * This function runs in a separate thread created by ThreadCreate() and
 * handles all rendering operations:
 * - Frame buffer management
 * - Scene rendering (game, menu, UI)
 * - Screen output via write(10, buffer, size)
 * - Frame rate control (30 FPS target)
 *
 * @param arg Thread argument (unused, can be NULL)
 */
void render_thread_func(void *arg);

/* ============================================================================
 *                          GAME STATE MANAGEMENT
 * ============================================================================ */

/**
 * @brief Reset game to initial state.
 *
 * Resets score, lives, level to starting values and initializes
 * the player at starting position. Used for "New Game".
 */
void game_reset(void);

/**
 * @brief Initialize a new level.
 *
 * Sets up the map, places enemies and gems for the specified level.
 * Resets player and enemy positions. Called when advancing levels.
 *
 * @param level Level number to initialize (1-based)
 */
void game_new_level(int level);

/**
 * @brief Restart current level after player death.
 *
 * Resets player and enemy positions without changing score or level.
 * Decrements life count. Used when player dies but has lives remaining.
 */
void game_restart_level(void);

#endif /* __GAME_H__ */
