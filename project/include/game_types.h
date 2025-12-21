/**
 * @file game_types.h
 * @brief Game types, enums and structures for Dig Dug clone.
 */

#ifndef __GAME_TYPES_H__
#define __GAME_TYPES_H__

#include <game_config.h>

/* ============================================================================
 *                              ENUMERATIONS
 * ============================================================================ */

/**
 * @brief Movement directions.
 */
typedef enum { DIR_NONE = 0, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

/**
 * @brief Game scenes/states.
 */
typedef enum {
    SCENE_MENU,        /* Main menu */
    SCENE_PLAYING,     /* Active gameplay */
    SCENE_PAUSED,      /* Game paused */
    SCENE_GAME_OVER,   /* Game over screen */
    SCENE_ROUND_CLEAR, /* Round completed */
    SCENE_ROUND_START, /* Starting new round */
    SCENE_VICTORY,     /* Victory screen (all rounds completed) */
    SCENE_CREDITS      /* Credits screen */
} GameScene;

/**
 * @brief Tile types for the game map.
 */
typedef enum {
    TILE_EMPTY, /* Excavated tunnel */
    TILE_DIRT,  /* Solid dirt (unexcavated) */
    TILE_SKY,   /* Sky area (rows 1-2) */
    TILE_WALL,  /* Solid wall (not walkable) */
    TILE_GEM,   /* Gem (collectible) */
    TILE_BONUS, /* Bonus item (100 points, '+') */
    TILE_BORDER /* Bottom border (gray # characters) */
} TileType;

/**
 * @brief Entity types.
 */
typedef enum {
    ENTITY_NONE,
    ENTITY_PLAYER, /* Dig Dug (the player) */
    ENTITY_POOKA,  /* Basic enemy (no special ability) */
    ENTITY_FYGAR,  /* Dragon enemy (spits fire horizontally) */
    ENTITY_ROCK,   /* Falling rock */
    ENTITY_ENEMY   /* Generic enemy type */
} EntityType;

/**
 * @brief Player states.
 */
typedef enum {
    PLAYER_IDLE,      /* Not moving */
    PLAYER_MOVING,    /* Walking/moving */
    PLAYER_DIGGING,   /* Excavating dirt */
    PLAYER_PUMPING,   /* Using pump to inflate enemy */
    PLAYER_ATTACKING, /* Attacking with harpoon */
    PLAYER_DEAD       /* Dead (hit by enemy/fire) */
} PlayerState;

/**
 * @brief Enemy states.
 */
typedef enum {
    ENEMY_NORMAL,    /* Moving normally in tunnels */
    ENEMY_GHOST,     /* Passing through dirt (ghost mode) */
    ENEMY_INFLATING, /* Being inflated by pump */
    ENEMY_PARALYZED, /* Paralyzed by player attack (will die after 2 sec) */
    ENEMY_DEAD       /* Eliminated */
} EnemyState;

/**
 * @brief Rock states.
 */
typedef enum {
    ROCK_STABLE,   /* Resting on solid ground */
    ROCK_WOBBLING, /* About to fall */
    ROCK_FALLING,  /* Currently falling */
    ROCK_LANDED,   /* Just landed (brief state) */
    ROCK_BLINKING  /* Blinking after hitting ground */
} RockState;

/* ============================================================================
 *                              STRUCTURES
 * ============================================================================ */

/**
 * @brief 2D position structure.
 */
typedef struct {
    int x; /* Column (0-79) */
    int y; /* Row (0-24) */
} Position;

/**
 * @brief Base entity structure (common fields only).
 *
 * Type-specific data is in Player, Enemy, or Rock structs.
 */
typedef struct {
    Position pos;      /* Current screen position */
    Direction dir;     /* Current facing direction */
    EntityType type;   /* Entity type identifier */
    int active;        /* 1 = active, 0 = inactive/dead */
    int speed_counter; /* Counter for movement timing */
    int speed_limit;   /* Ticks between movements (lower = faster) */
} Entity;

/**
 * @brief Player structure.
 *
 * Use player.base for generic entity operations.
 */
typedef struct {
    Entity base;          /* Base entity data */
    PlayerState state;    /* Player-specific state */
    Direction facing_dir; /* Direction player is facing (for rendering) */
    int is_pumping;       /* 1 = currently firing pump */
    int pump_length;      /* Current pump extension (0-4 cells) */
    Direction pump_dir;   /* Direction pump is facing */
    int is_attacking;     /* 1 = currently attacking */
    int attack_timer;     /* Frames remaining for attack display */
} Player;

/**
 * @brief Enemy structure (for Pooka and Fygar).
 *
 * fire_* fields only used when base.type == ENTITY_FYGAR.
 */
typedef struct {
    Entity base;         /* Base entity data */
    EnemyState state;    /* Enemy-specific state */
    int inflate_level;   /* Inflation level 0-4 (4 = explodes) */
    int ghost_timer;     /* Ticks until ghost mode activates */
    int fire_start_time; /* Fygar: timestamp when fire started (0 = not active) */
    int fire_end_time;   /* Fygar: timestamp when cooldown ends */
    int fire_active;     /* Fygar: 1 = currently breathing fire */
    int fire_duration;   /* Fygar: unused, kept for compatibility */
    int paralyzed_timer; /* Timer for blink animation */
    int blink_count;     /* Number of blinks remaining (dies at 0) */
    int has_left_tunnel; /* Ghost mode: 1 = has moved through dirt */
} Enemy;

/**
 * @brief Rock structure.
 */
typedef struct {
    Entity base;      /* Base entity data */
    RockState state;  /* Rock-specific state */
    int wobble_timer; /* Frames wobbling before fall */
    int has_crushed;  /* 1 = has crushed something this fall */
    int blink_timer;  /* Timer for blink animation */
    int blink_count;  /* Number of blinks remaining */
} Rock;

/**
 * @brief Input state structure.
 * Updated by keyboard handler, read by game logic.
 * NOTE: This is the ONLY definition - do NOT redefine in other headers!
 */
typedef struct {
    Direction direction; /* Current movement direction (consumed each frame) */
    Direction held_dir;  /* Direction currently being held (for continuous movement) */
    int action_pressed;  /* Action button pressed (enter) */
    int attack_pressed;  /* Attack button pressed once (space) */
    int attack_held;     /* Attack button currently held down */
    int pause_pressed;   /* Pause button pressed (P/ESC) */
    int quit_pressed;    /* Quit button pressed (Q) */
    int any_key_pressed; /* Any key pressed flag */
    char last_key;       /* Last raw key pressed */
    int move_processed;  /* Flag to ensure only one move per frame */
} InputState;

/**
 * @brief Main game state structure.
 * Contains all game data for current session.
 * NOTE: This is the ONLY definition - do NOT redefine in other headers!
 */
typedef struct {
    /* Scene management */
    GameScene scene; /* Current game scene */

    /* Game progress */
    int score;     /* Current score (0-99999) */
    int level;     /* Current level number (1-99) */
    int lives;     /* Remaining lives (0-5) */
    int gem_count; /* Gems remaining in level */

    /* Timing */
    int ticks_elapsed;    /* Total game ticks elapsed */
    int last_update_tick; /* Last logic update tick */

    /* Entities - using simple Entity struct */
    Entity player;               /* The player entity */
    Entity enemies[MAX_ENEMIES]; /* Array of enemies */
    int enemy_count;             /* Total enemies in current round */

    /* Game flags */
    int paused;         /* Game is paused */
    int game_over;      /* Game over flag */
    int level_complete; /* Level complete flag */
} GameState;

/* ============================================================================
 *                              HELPER MACROS
 * ============================================================================ */

/**
 * @brief Get the layer number (1-4) for a given row.
 * Returns 0 if row is not in ground area.
 */
#define GET_LAYER(row)                                                                             \
    (((row) >= LAYER_1_START && (row) <= LAYER_1_END)   ? 1                                        \
     : ((row) >= LAYER_2_START && (row) <= LAYER_2_END) ? 2                                        \
     : ((row) >= LAYER_3_START && (row) <= LAYER_3_END) ? 3                                        \
     : ((row) >= LAYER_4_START && (row) <= LAYER_4_END) ? 4                                        \
                                                        : 0)

/**
 * @brief Get score multiplier for a layer.
 */
#define GET_LAYER_SCORE(layer)                                                                     \
    (((layer) == 1)   ? SCORE_LAYER_1                                                              \
     : ((layer) == 2) ? SCORE_LAYER_2                                                              \
     : ((layer) == 3) ? SCORE_LAYER_3                                                              \
     : ((layer) == 4) ? SCORE_LAYER_4                                                              \
                      : 0)

/**
 * @brief Check if a row is in the sky area.
 */
#define IS_SKY(row) ((row) >= ROW_SKY_START && (row) <= ROW_SKY_END)

/**
 * @brief Check if a row is in the ground area.
 */
#define IS_GROUND(row) ((row) >= ROW_GROUND_START && (row) <= ROW_GROUND_END)

/**
 * @brief Check if a row is a status bar.
 */
#define IS_STATUS_ROW(row) ((row) == ROW_STATUS_TOP || (row) == ROW_STATUS_BOTTOM)

/**
 * @brief Screen position calculation (byte offset in video memory).
 */
#define SCREEN_POS(x, y) (((y) * SCREEN_WIDTH + (x)) * 2)

/**
 * @brief Check if position is within screen bounds.
 */
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < SCREEN_WIDTH && (y) >= 0 && (y) < SCREEN_HEIGHT)

/**
 * @brief Check if position is in playable area (not status bars).
 */
#define IN_PLAYABLE_AREA(x, y)                                                                     \
    ((x) >= 0 && (x) < MAP_WIDTH && (y) >= ROW_SKY_START && (y) <= ROW_GROUND_END)

/**
 * @brief Check if entity type is an enemy (Pooka or Fygar).
 */
#define IS_ENEMY_TYPE(type) ((type) == ENTITY_POOKA || (type) == ENTITY_FYGAR)

/**
 * @brief Check if entity type can breathe fire (only Fygar).
 */
#define CAN_BREATHE_FIRE(type) ((type) == ENTITY_FYGAR)

#endif /* __GAME_TYPES_H__ */
