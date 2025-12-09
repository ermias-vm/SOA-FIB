#ifndef __GAME_H__
#define __GAME_H__

#include <types.h>
#include <game_types.h>
#include <game_config.h>
#include <game_entities.h>
#include <game_map.h>
#include <game_input.h>
#include <game_render.h>
#include <game_ui.h>
#include <game_logic.h>

/**
 * @file game.h
 * @brief Main game header that includes all subsystems and defines global state
 */

/* Forward declarations */
struct task_struct;

/**
 * Global game state - shared between logic and render threads
 * Must be volatile since it's accessed by multiple threads
 */
typedef struct {
    GameScene scene;                    /* Current game scene */
    int score;                          /* Player score */
    int level;                          /* Current level */
    int lives;                          /* Remaining lives */
    int gem_count;                      /* Gems remaining in level */
    
    Entity player;                      /* Player entity */
    Entity enemies[MAX_ENEMIES];        /* Enemy entities array */
    int enemy_count;                    /* Number of active enemies */
    
    int paused;                         /* Game paused flag */
    int game_over;                      /* Game over flag */
    int level_complete;                 /* Level complete flag */
    
    /* Timing and synchronization */
    int ticks_elapsed;                  /* Game ticks elapsed */
    int last_update_tick;               /* Last logic update tick */
} GameState;

/**
 * Global input state - updated by keyboard handler
 * Must be volatile since it's updated by interrupt handler
 */
typedef struct {
    Direction move_dir;                 /* Movement direction from WASD */
    int pause_pressed;                  /* ESC key pressed flag */
    int quit_pressed;                   /* Q key pressed flag */
    int action_pressed;                 /* SPACE/ENTER pressed flag */
    int any_key_pressed;                /* Any key pressed (for menu) */
} InputState;

/* Global state variables */
extern volatile GameState g_game;
extern volatile InputState g_input;

/* Thread synchronization variables */
extern volatile int g_frame_ready;
extern volatile int g_running;

/* Main game functions */
void game_main(void);
void game_init(void);
void game_cleanup(void);
void game_run(void);

/* Thread functions */
void logic_thread_func(void *arg);
void render_thread_func(void *arg);

/* Game state management */
void game_reset(void);
void game_new_level(int level);
void game_restart_level(void);

#endif /* __GAME_H__ */