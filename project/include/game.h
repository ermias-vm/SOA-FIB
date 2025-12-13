#ifndef __GAME_H__
#define __GAME_H__

#include <types.h>
#include <game_types.h>
#include <game_config.h>
#include <game_entities.h>
#include <game_map.h>
#include <game_input.h>
#include <game_render.h>
/*
#include <game_ui.h>
#include <game_logic.h>
*/

/**
 * @file game.h
 * @brief Main game header that includes all subsystems and defines global state
 */

/* Forward declarations */
struct task_struct;

/* ============================================================================
 *                            GLOBAL STATE STRUCTURES
 * ============================================================================ */

/**
 * @brief Global game state shared between logic and render threads.
 * 
 * This structure contains all the game state information that needs to be
 * accessed by both the logic thread and render thread. All fields must be
 * volatile since they're accessed by multiple threads.
 */
typedef struct {
    GameScene scene;                    /**< Current game scene (menu, playing, paused, etc.) */
    int score;                          /**< Player's current score */
    int level;                          /**< Current level number (1-based) */
    int lives;                          /**< Remaining player lives */
    int gem_count;                      /**< Number of gems remaining in current level */
    
    Entity player;                      /**< Player entity structure */
    Entity enemies[MAX_ENEMIES];        /**< Array of enemy entities */
    int enemy_count;                    /**< Number of active enemies in current level */
    
    int paused;                         /**< Game paused flag (1=paused, 0=running) */
    int game_over;                      /**< Game over flag (1=game over, 0=playing) */
    int level_complete;                 /**< Level complete flag (1=completed, 0=in progress) */
    
    /* Timing and synchronization */
    int ticks_elapsed;                  /**< Total game ticks elapsed since start */
    int last_update_tick;               /**< Last tick when logic was updated */
} GameState;

/**
 * @brief Global input state updated by keyboard handler.
 * 
 * This structure contains the current input state from the keyboard.
 * Must be volatile since it's updated by interrupt handler and read
 * by the main game logic thread.
 */
typedef struct {
    Direction move_dir;                 /**< Current movement direction from WASD keys */
    int pause_pressed;                  /**< ESC key pressed flag (1=pressed this frame) */
    int quit_pressed;                   /**< Q key pressed flag (1=pressed this frame) */
    int action_pressed;                 /**< SPACE/ENTER pressed flag (1=pressed this frame) */
    int any_key_pressed;                /**< Any key pressed flag (for menu navigation) */
} InputState;

/* ============================================================================
 *                            GLOBAL VARIABLES
 * ============================================================================ */

/**
 * @brief Global game state variable.
 * 
 * This variable holds the current state of the game and is shared between
 * the logic thread and render thread. Must be volatile for thread safety.
 */
extern volatile GameState g_game;

/**
 * @brief Global input state variable.
 * 
 * This variable holds the current input state from the keyboard handler.
 * Updated by interrupt handler, read by main logic thread.
 */
extern volatile InputState g_input;

/* ============================================================================
 *                         THREAD SYNCHRONIZATION
 * ============================================================================ */

/**
 * @brief Frame ready flag for thread synchronization.
 * 
 * Set to 1 by logic thread when a new frame is ready to render.
 * Reset to 0 by render thread after rendering is complete.
 */
extern volatile int g_frame_ready;

/**
 * @brief Game running flag for thread control.
 * 
 * Set to 1 when game is running, 0 when game should exit.
 * Used by both threads to know when to terminate.
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