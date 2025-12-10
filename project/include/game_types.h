/**
 * @file game_types.h
 * @brief Game types, enums and structures for Dig Dug clone.
 *
 * This header defines all game-related types including directions,
 * game scenes, tile types, entity types, and core structures.
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
typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/**
 * @brief Game scenes/states.
 */
typedef enum {
    SCENE_MENU,         /* Main menu */
    SCENE_PLAYING,      /* Active gameplay */
    SCENE_PAUSED,       /* Game paused */
    SCENE_GAME_OVER,    /* Game over screen */
    SCENE_ROUND_CLEAR,  /* Round completed */
    SCENE_ROUND_START   /* Starting new round */
} GameScene;

/**
 * @brief Tile types for the game map.
 */
typedef enum {
    TILE_EMPTY = 0,     /* Excavated tunnel */
    TILE_DIRT,          /* Solid dirt (unexcavated) */
    TILE_SKY,            /* Sky area (rows 1-3) */
    TILE_WALL,              /* Solid wall (not walkable) */
    TILE_GEM                /* Gem (collectible) */
} TileType;

/**
 * @brief Entity types.
 */
typedef enum {
    ENTITY_NONE = 0,
    ENTITY_PLAYER,      /* Dig Dug (the player) */
    ENTITY_POOKA,       /* Basic enemy (no special ability) */
    ENTITY_FYGAR        /* Dragon enemy (spits fire horizontally) */
} EntityType;

/**
 * @brief Player states.
 */
typedef enum {
    PLAYER_IDLE,        /* Not moving */
    PLAYER_MOVING,      /* Walking/moving */
    PLAYER_DIGGING,     /* Excavating dirt */
    PLAYER_PUMPING,     /* Using pump to inflate enemy */
    PLAYER_DEAD         /* Dead (hit by enemy/fire) */
} PlayerState;

/**
 * @brief Enemy states.
 */
typedef enum {
    ENEMY_NORMAL,       /* Moving normally in tunnels */
    ENEMY_GHOST,        /* Passing through dirt (ghost mode) */
    ENEMY_INFLATING,    /* Being inflated by pump */
    ENEMY_DEAD          /* Eliminated */
} EnemyState;

/* ============================================================================
 *                              STRUCTURES
 * ============================================================================ */

/**
 * @brief 2D position structure.
 */
typedef struct {
    int x;              /* Column (0-79) */
    int y;              /* Row (0-24) */
} Position;

/**
 * @brief Base entity structure for all game entities.
 */
typedef struct {
    Position pos;           /* Current position */
    Direction dir;          /* Current facing direction */
    EntityType type;        /* Entity type (player, pooka, fygar) */
    int active;             /* 1 = active, 0 = inactive */
    int speed_counter;      /* Counter for movement timing */
    int speed;              /* Ticks between movements */
    int inflate_level;      /* Inflation level (0-4, for enemies) */
    int fire_cooldown;      /* Fygar fire cooldown counter */
    union {
        PlayerState player_state;
        EnemyState enemy_state;
    } state;
} Entity;

/**
 * @brief Input state structure.
 * Updated by keyboard handler, read by game logic.
 */
typedef struct {
    Direction move_dir;     /* Movement direction from WASD */
    int pump_pressed;       /* Space bar for pump/attack */
    int pause_pressed;      /* ESC or P for pause */
    int quit_pressed;       /* Q to quit */
    int start_pressed;      /* Enter to start/confirm */
} InputState;

/**
 * @brief Main game state structure.
 * Contains all game data for current session.
 */
typedef struct {
    /* Scene management */
    GameScene scene;            /* Current game scene */
    
    /* Score and progress */
    int score;                  /* Current score (0-99999) */
    int round;                  /* Current round number (1-99) */
    int lives;                  /* Remaining lives (0-5) */
    
    /* Timing */
    int time_elapsed;           /* Time in ticks since round start */
    int round_start_timer;      /* Timer for round start delay */
    
    /* Entities */
    Entity player;              /* The player entity */
    Entity enemies[MAX_ENEMIES]; /* Array of enemies */
    int enemy_count;            /* Total enemies in current round */
    int enemies_remaining;      /* Enemies still alive */
    
    /* Game flags */
    int running;                /* Game is running (not quit) */
    int frame_ready;            /* New frame ready to render */
} GameState;

/* ============================================================================
 *                              HELPER MACROS
 * ============================================================================ */

/**
 * @brief Get the layer number (1-4) for a given row.
 * Returns 0 if row is not in ground area.
 */
#define GET_LAYER(row) ( \
    ((row) >= LAYER_1_START && (row) <= LAYER_1_END) ? 1 : \
    ((row) >= LAYER_2_START && (row) <= LAYER_2_END) ? 2 : \
    ((row) >= LAYER_3_START && (row) <= LAYER_3_END) ? 3 : \
    ((row) >= LAYER_4_START && (row) <= LAYER_4_END) ? 4 : 0 \
)

/**
 * @brief Get score multiplier for a layer.
 */
#define GET_LAYER_SCORE(layer) ( \
    ((layer) == 1) ? SCORE_LAYER_1 : \
    ((layer) == 2) ? SCORE_LAYER_2 : \
    ((layer) == 3) ? SCORE_LAYER_3 : \
    ((layer) == 4) ? SCORE_LAYER_4 : 0 \
)

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
 * @brief Get color for a specific layer.
 */
#define GET_LAYER_COLOR(layer) ( \
    ((layer) == 1) ? COLOR_LAYER_1 : \
    ((layer) == 2) ? COLOR_LAYER_2 : \
    ((layer) == 3) ? COLOR_LAYER_3 : \
    ((layer) == 4) ? COLOR_LAYER_4 : COLOR_TUNNEL \
)

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
#define IN_PLAYABLE_AREA(x, y) ((x) >= 0 && (x) < MAP_WIDTH && (y) >= ROW_SKY_START && (y) <= ROW_GROUND_END)

#endif /* __GAME_TYPES_H__ */
