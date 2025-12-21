/**
 * @file game.c
 * @brief Main game entry point and game loop implementation.
 *
 * Milestone M5.11 - GameMain & Integration
 * Contains the main game loop, thread functions, and scene management.
 */

#include <debug.h>
#include <game.h>
#include <game_data.h>
#include <game_input.h>
#include <game_logic.h>
#include <game_map.h>
#include <game_render.h>
#include <game_ui.h>
#include <libc.h>
#include <times.h>

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
 *                            GAME CONSTANTS
 * ============================================================================ */

#define INITIAL_LIVES 3
#define GAME_OVER_DELAY TIME_LONG        /* From times.h */
#define MY_ROUND_START_DELAY TIME_SHORT  /* From times.h */
#define MY_LEVEL_CLEAR_DELAY TIME_MEDIUM /* From times.h */

/* Frame timing for 60 FPS limiting */
static int g_last_frame_time = 0;
static int g_frame_ticks = 0;

/* ============================================================================
 *                          FORWARD DECLARATIONS
 * ============================================================================ */

static void process_menu_state(void);
static void process_playing_state(void);
static void process_paused_state(void);
static void process_level_clear_state(void);
static void process_game_over_state(void);
static void process_victory_state(void);
static void process_credits_state(void);
static void sync_logic_to_game_state(void);

/* ============================================================================
 *                          DEBUG FUNCTIONS
 * ============================================================================ */

#if DEBUG_GAME_ENABLED

static void debug_print_state_change(const char *new_state) {
#if DEBUG_GAME_STATE
    printd("[DEBUG] State -> %s\n", new_state);
#else
    (void)new_state;
#endif
}

static void debug_print_input(Direction dir, int x, int y) {
#if DEBUG_GAME_INPUT
    if (dir != DIR_NONE) {
        const char *dir_name = "NONE";
        switch (dir) {
        case DIR_UP:
            dir_name = "UP";
            break;
        case DIR_DOWN:
            dir_name = "DOWN";
            break;
        case DIR_LEFT:
            dir_name = "LEFT";
            break;
        case DIR_RIGHT:
            dir_name = "RIGHT";
            break;
        default:
            break;
        }
        printd("[DEBUG] Input: %s | Pos: (%d, %d)\n", dir_name, x, y);
    }
#else
    (void)dir;
    (void)x;
    (void)y;
#endif
}

static void debug_print_player(int x, int y, int state) {
#if DEBUG_GAME_PLAYER
    printd("[DEBUG] Player: pos=(%d,%d) state=%d\n", x, y, state);
#else
    (void)x;
    (void)y;
    (void)state;
#endif
}

#else /* DEBUG_GAME_ENABLED == 0 */

#define debug_print_state_change(s) ((void)0)
#define debug_print_input(d, x, y) ((void)0)
#define debug_print_player(x, y, s) ((void)0)

#endif /* DEBUG_GAME_ENABLED */

/* ============================================================================
 *                          FRAME RATE CONTROL
 * ============================================================================ */

/**
 * @brief Wait until enough time has passed for the next frame (60 FPS limit).
 *
 * This function ensures consistent frame timing by waiting until at least
 * TICKS_PER_FRAME ticks have passed since the last frame.
 */
static void wait_for_next_frame(void) {
    int ticks_per_frame = TICKS_PER_FRAME;
    if (ticks_per_frame < MIN_TICKS_PER_FRAME) {
        ticks_per_frame = MIN_TICKS_PER_FRAME;
    }

    int current_time = gettime();
    int elapsed = current_time - g_last_frame_time;

    /* If not enough time has passed, busy-wait */
    while (elapsed < ticks_per_frame) {
        current_time = gettime();
        elapsed = current_time - g_last_frame_time;
    }

    g_frame_ticks = elapsed;
    g_last_frame_time = current_time;
}

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

    /* 6. Initialize frame timing */
    g_last_frame_time = gettime();
    g_frame_ticks = 0;

    /* 7. Clear screen */
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
    g_logic_state.round_start_timer = MY_ROUND_START_DELAY;

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
        /* Clear any residual attack input from menu selection */
        input_clear();
        /* Scene is set to SCENE_ROUND_START in game_new_level() */
    }
}

static void process_playing_state(void) {
    /* Check for pause */
    if (input_is_pause_pressed()) {
        debug_print_state_change("PAUSED");
        g_game.scene = SCENE_PAUSED;
        g_logic_state.scene = SCENE_PAUSED;
        return;
    }

    /* Get player input direction */
    Direction dir = input_get_direction();
    int pumping = input_is_action_pressed();
    int attack_just_pressed = input_is_attack_pressed(); /* Consume first press */
    int attack_held = input_is_attack_held();            /* Check if still held */

    /* Debug print for input */
    debug_print_input(dir, g_logic_state.player.base.pos.x, g_logic_state.player.base.pos.y);

    /* Update player input in logic state */
    if (dir != DIR_NONE) {
        g_logic_state.player.base.dir = dir;
    }
    g_logic_state.player.is_pumping = pumping;

    /* Attack: trigger on first press, maintain while held */
    if (attack_just_pressed) {
        /* Start a new attack */
        logic_player_attack(&g_logic_state.player, &g_logic_state);
    } else if (attack_held && g_logic_state.player.is_attacking) {
        /* Maintain attack while space is held */
        g_logic_state.player.attack_timer = ATTACK_DISPLAY_FRAMES;
    }

    /* Run game logic update */
    logic_update(&g_logic_state);

    /* Sync state */
    sync_logic_to_game_state();

    /* Check for level clear */
    if (g_logic_state.enemies_remaining <= 0) {
        g_game.scene = SCENE_ROUND_CLEAR;
        g_logic_state.scene = SCENE_ROUND_CLEAR;
        g_logic_state.round_start_timer = ROUND_CLEAR_DELAY;
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
            /* Victory - go to victory screen */
            g_game.scene = SCENE_VICTORY;
            g_logic_state.scene = SCENE_VICTORY;
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

static void process_victory_state(void) {
    /* Handle victory screen inputs */
    if (input_is_action_pressed()) {
        /* SPACE - play again */
        game_reset();
        g_game.level = 1;
        game_new_level();
    } else if (input_is_quit_pressed()) {
        /* ESC - return to menu */
        input_clear_quit();
        g_game.scene = SCENE_MENU;
        g_logic_state.scene = SCENE_MENU;
    } else {
        /* Check for 'C' key for credits */
        char last_key = input_get_last_key();
        if (last_key == 'c' || last_key == 'C') {
            g_game.scene = SCENE_CREDITS;
            g_logic_state.scene = SCENE_CREDITS;
        }
    }
}

static void process_credits_state(void) {
    /* Return to victory screen or menu on ESC */
    if (input_is_quit_pressed()) {
        input_clear_quit();
        g_game.scene = SCENE_VICTORY;
        g_logic_state.scene = SCENE_VICTORY;
    }
}

/* ============================================================================
 *                          THREAD FUNCTIONS
 * ============================================================================ */

void logic_thread_func(void *arg) {
    (void)arg; /* Unused */

    while (g_running) {
        /* Wait for next frame (60 FPS limit) */
        wait_for_next_frame();

        /* Start new input frame (reset move_processed, prepare held direction) */
        input_new_frame();

        /* Update input state */
        input_update();

        /* Check for quit (ESC key) - only in menu or playing scenes */
        if (input_is_quit_pressed()) {
            if (g_game.scene == SCENE_MENU) {
                g_running = 0;
                break;
            }
            /* Other scenes handle ESC themselves */
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
                /* Clear input to prevent accidental attack on round start */
                input_clear();
            }
            break;

        case SCENE_VICTORY:
            process_victory_state();
            break;

        case SCENE_CREDITS:
            process_credits_state();
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
    (void)arg;

    while (g_running) {
        /* Wait for frame ready signal from logic thread */
        while (!g_frame_ready && g_running) {
            /* Busy wait - could yield here */
        }

        if (!g_running) break;

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

            /* Render HUD with enemies remaining */
            ui_draw_hud_extended((int)g_game.lives, (int)g_game.score, (int)g_game.level,
                                 (int)g_logic_state.time_elapsed, 0,
                                 (int)g_logic_state.enemies_remaining);
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

        case SCENE_VICTORY:
            ui_draw_victory_screen((int)g_game.score);
            break;

        case SCENE_CREDITS:
            ui_draw_credits_screen();
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
    printd("[GAME] Initializing game systems...\n");

    /* Initialize all game systems */
    game_init();

    printd("[GAME] Creating render thread...\n");

    /* Create render thread */
    int tid = ThreadCreate(render_thread_func, NULL);
    if (tid < 0) {
        printd("[GAME] ERROR: Failed to create render thread!\n");
        game_cleanup();
        return;
    }

    printd("[GAME] Render thread created (TID=%d)\n", tid);
    printd("[GAME] Starting game loop...\n");

    /* Run the main game loop (logic in main thread) */
    game_run();

    printd("[GAME] Game loop ended. Cleaning up...\n");

    /* Cleanup */
    game_cleanup();

    printd("[GAME] Game exited successfully.\n");
}
