/**
 * @file game.c
 * @brief Main game entry point and game loop implementation.
 *
 * Milestone M5.11 - GameMain & Integration
 * Contains the main game loop, thread functions, and scene management.
 */

#include <game.h>
#include <game_data.h>
#include <game_input.h>
#include <game_logic.h>
#include <game_map.h>
#include <game_render.h>
#include <game_ui.h>
#include <libc.h>

/* ============================================================================
 *                          GLOBAL VARIABLES
 * ============================================================================ */

volatile GameState g_game;
/* g_input is defined in game_input.c */
volatile int g_frame_ready = 0;
volatile int g_running = 0;

/* Game logic state for extended logic operations */
static GameLogicState g_logic_state;

/* ============================================================================
 *                            TIMING CONSTANTS
 * ============================================================================ */

#define TICKS_PER_FRAME_LOGIC 2  /* ~50 logic updates per second at 100Hz */
#define TICKS_PER_FRAME_RENDER 3 /* ~33 FPS rendering */
#define INITIAL_LIVES 3
#define GAME_OVER_DELAY 200 /* Ticks to show game over screen */
/* NOTE: LEVEL_CLEAR_DELAY is defined in game_logic.h */
/* NOTE: ROUND_START_DELAY is defined in game_config.h */

/* ============================================================================
 *                          FORWARD DECLARATIONS
 * ============================================================================ */

static void process_menu_state(void);
static void process_playing_state(void);
static void process_paused_state(void);
static void process_level_clear_state(void);
static void process_game_over_state(void);
static void sync_logic_to_game_state(void);

/* ============================================================================
 *                          INITIALIZATION
 * ============================================================================ */

void game_init(void) {
    /* 1. Initialize render system (buffers) */
    render_init();

    /* 2. Initialize input system */
    input_init();

    /* 3. Initialize game logic state */
    logic_init(&g_logic_state);

    /* 4. Initialize global game state */
    g_game.scene = SCENE_MENU;
    g_game.score = 0;
    g_game.level = 1;
    g_game.lives = INITIAL_LIVES;
    g_game.ticks_elapsed = 0;
    g_game.enemy_count = 0;

    /* 5. Set running flag */
    g_running = 1;
    g_frame_ready = 0;

    /* 6. Clear screen */
    render_clear();
    render_present();
}

void game_cleanup(void) {
    g_running = 0;
    render_cleanup();
}

/* ============================================================================
 *                          GAME STATE MANAGEMENT
 * ============================================================================ */

void game_reset(void) {
    g_game.score = 0;
    g_game.lives = INITIAL_LIVES;
    g_game.level = 1;
    g_game.ticks_elapsed = 0;
    g_game.enemy_count = 0;
    g_game.scene = SCENE_MENU;

    /* Reset logic state */
    logic_init(&g_logic_state);
    g_logic_state.score = 0;
    g_logic_state.lives = INITIAL_LIVES;
    g_logic_state.round = 1;
}

GameState *game_get_state(void) {
    return (GameState *)&g_game;
}

int game_is_running(void) {
    return g_running;
}

int game_new_level(void) {
    int level = g_game.level;

    /* Validate level number */
    if (level < 1 || level > data_get_num_levels()) {
        return -1;
    }

    /* Load level data into logic state */
    data_load_level(level, &g_logic_state);

    /* Sync with global game state */
    g_game.enemy_count = g_logic_state.enemy_count;

    /* Set scene to ROUND_START with delay timer */
    g_game.scene = SCENE_ROUND_START;
    g_logic_state.scene = SCENE_ROUND_START;
    g_logic_state.round_start_timer = ROUND_START_DELAY;

    return 0;
}

void game_restart_level(void) {
    /* Reload current level */
    data_load_level(g_game.level, &g_logic_state);
    g_game.enemy_count = g_logic_state.enemy_count;
}

static void sync_logic_to_game_state(void) {
    /* Sync from logic state to global game state */
    g_game.score = g_logic_state.score;
    g_game.lives = g_logic_state.lives;
    g_game.enemy_count = g_logic_state.enemy_count;
    g_game.ticks_elapsed = g_logic_state.time_elapsed;

    /* Only sync scene if it changed in logic */
    if (g_logic_state.scene != (GameScene)g_game.scene) {
        g_game.scene = g_logic_state.scene;
    }
}

/* ============================================================================
 *                          SCENE PROCESSORS
 * ============================================================================ */

static void process_menu_state(void) {
    /* Wait for action key (Space/Enter) to start */
    if (input_is_action_pressed()) {
        game_reset();
        g_game.level = 1;
        game_new_level();
        /* Scene is set to SCENE_ROUND_START in game_new_level() */
    }
}

static void process_playing_state(void) {
    /* Check for pause */
    if (input_is_pause_pressed()) {
        g_game.scene = SCENE_PAUSED;
        g_logic_state.scene = SCENE_PAUSED;
        return;
    }

    /* Get player input direction */
    Direction dir = input_get_direction();
    int pumping = input_is_action_pressed();

    /* Update player input in logic state */
    if (dir != DIR_NONE) {
        g_logic_state.player.base.dir = dir;
    }
    g_logic_state.player.is_pumping = pumping;

    /* Run game logic update */
    logic_update(&g_logic_state);

    /* Sync state */
    sync_logic_to_game_state();

    /* Check for level clear */
    if (g_logic_state.enemies_remaining <= 0) {
        g_game.scene = SCENE_ROUND_CLEAR;
        g_logic_state.scene = SCENE_ROUND_CLEAR;
        g_logic_state.round_start_timer = LEVEL_CLEAR_DELAY;
    }

    /* Check for game over */
    if (g_logic_state.lives <= 0) {
        g_game.scene = SCENE_GAME_OVER;
        g_logic_state.scene = SCENE_GAME_OVER;
        g_logic_state.round_start_timer = GAME_OVER_DELAY;
    }
}

static void process_paused_state(void) {
    /* Wait for pause key to resume */
    if (input_is_pause_pressed()) {
        g_game.scene = SCENE_PLAYING;
        g_logic_state.scene = SCENE_PLAYING;
    }
}

static void process_level_clear_state(void) {
    g_logic_state.round_start_timer--;

    if (g_logic_state.round_start_timer <= 0) {
        g_game.level++;
        g_logic_state.round++;

        if (g_game.level > MAX_ROUNDS) {
            /* Victory - return to menu with high score */
            g_game.scene = SCENE_MENU;
            g_logic_state.scene = SCENE_MENU;
        } else {
            game_new_level();
        }
    }
}

static void process_game_over_state(void) {
    g_logic_state.round_start_timer--;

    if (g_logic_state.round_start_timer <= 0) {
        /* Wait for action key to restart or quit */
        if (input_is_action_pressed()) {
            game_reset();
            g_game.level = 1;
            game_new_level();
        }
    }
}

/* ============================================================================
 *                          THREAD FUNCTIONS
 * ============================================================================ */

void logic_thread_func(void *arg) {
    (void)arg; /* Unused */

    int last_tick = gettime();

    while (g_running) {
        int current_tick = gettime();

        /* Frame rate control for logic */
        if (current_tick - last_tick < TICKS_PER_FRAME_LOGIC) {
            continue;
        }
        last_tick = current_tick;

        /* Update input state */
        input_update();

        /* Check for quit (ESC key) */
        if (input_is_quit_pressed()) {
            g_running = 0;
            break;
        }

        /* Process based on current scene */
        switch (g_game.scene) {
        case SCENE_MENU:
            process_menu_state();
            break;

        case SCENE_PLAYING:
            process_playing_state();
            break;

        case SCENE_PAUSED:
            process_paused_state();
            break;

        case SCENE_ROUND_CLEAR:
            process_level_clear_state();
            break;

        case SCENE_GAME_OVER:
            process_game_over_state();
            break;

        case SCENE_ROUND_START:
            /* Brief delay before round starts */
            g_logic_state.round_start_timer--;
            if (g_logic_state.round_start_timer <= 0) {
                g_game.scene = SCENE_PLAYING;
                g_logic_state.scene = SCENE_PLAYING;
            }
            break;

        default:
            break;
        }

        /* Signal render thread that frame is ready */
        g_frame_ready = 1;

        /* Update time */
        g_game.ticks_elapsed++;
    }
}

void render_thread_func(void *arg) {
    (void)arg; /* Unused */

    int last_tick = gettime();

    while (g_running) {
        int current_tick = gettime();

        /* Frame rate control for rendering */
        if (current_tick - last_tick < TICKS_PER_FRAME_RENDER) {
            continue;
        }
        last_tick = current_tick;

        /* Wait for logic to signal frame ready (optional sync) */
        /* For now, render continuously */

        /* Clear buffer */
        render_clear();

        /* Render based on current scene */
        switch (g_game.scene) {
        case SCENE_MENU:
            ui_draw_menu_screen();
            break;

        case SCENE_PLAYING:
            /* Render game world */
            render_map();
            render_entities(&g_logic_state);
            render_player(&g_logic_state.player);
            render_enemies(g_logic_state.enemies, g_logic_state.enemy_count);
            render_rocks(g_logic_state.rocks, g_logic_state.rock_count);

            /* Render HUD */
            ui_draw_hud((int)g_game.lives, (int)g_game.score, (int)g_game.level,
                        (int)g_game.ticks_elapsed / 100, 0);
            break;

        case SCENE_PAUSED:
            /* Render game underneath pause overlay */
            render_map();
            render_entities(&g_logic_state);
            ui_draw_pause_screen();
            break;

        case SCENE_ROUND_CLEAR:
            ui_draw_level_clear_screen((int)g_game.level, (int)g_game.score);
            break;

        case SCENE_GAME_OVER:
            ui_draw_game_over_screen((int)g_game.score);
            break;

        case SCENE_ROUND_START:
            /* Show round starting message */
            ui_draw_level_clear_screen((int)g_game.level, (int)g_game.score);
            break;

        default:
            break;
        }

        /* Present to screen */
        render_present();

        /* Clear frame ready flag */
        g_frame_ready = 0;
    }

    /* Thread exit */
    ThreadExit();
}

/* ============================================================================
 *                          MAIN ENTRY POINT
 * ============================================================================ */

void game_run(void) {
    /* Main game loop runs in the calling thread (logic thread) */
    logic_thread_func(NULL);
}

void game_main(void) {
    prints("[GAME] Initializing game systems...\n");

    /* Initialize all game systems */
    game_init();

    prints("[GAME] Creating render thread...\n");

    /* Create render thread */
    int tid = ThreadCreate(render_thread_func, NULL);
    if (tid < 0) {
        prints("[GAME] ERROR: Failed to create render thread!\n");
        game_cleanup();
        return;
    }

    prints("[GAME] Render thread created (TID=%d)\n", tid);
    prints("[GAME] Starting game loop...\n");

    /* Run the main game loop (logic in main thread) */
    game_run();

    prints("[GAME] Game loop ended. Cleaning up...\n");

    /* Cleanup */
    game_cleanup();

    prints("[GAME] Game exited successfully.\n");
}
