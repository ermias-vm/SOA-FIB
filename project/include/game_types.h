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
    TILE_EMPTY,         /* Excavated tunnel */
    TILE_DIRT,          /* Solid dirt (unexcavated) */
    TILE_SKY,           /* Sky area (rows 1-3) */
    TILE_WALL,          /* Solid wall (not walkable) */
    TILE_GEM            /* Gem (collectible) */
} TileType;

/**
 * @brief Entity types.
 */
typedef enum {
    ENTITY_NONE,
    ENTITY_PLAYER,      /* Dig Dug (the player) */
    ENTITY_POOKA,       /* Basic enemy (no special ability) */
    ENTITY_FYGAR,       /* Dragon enemy (spits fire horizontally) */
    ENTITY_ROCK,        /* Falling rock */
    ENTITY_ENEMY        /* Generic enemy type */
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

/**
 * @brief Rock states.
 */
typedef enum {
    ROCK_STABLE,        /* Resting on solid ground */
    ROCK_WOBBLING,      /* About to fall */
    ROCK_FALLING,       /* Currently falling */
    ROCK_LANDED         /* Just landed (brief state) */
} RockState;

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
 * @brief Base entity structure (common fields only).
 * 
 * Type-specific data is in Player, Enemy, or Rock structs.
 */
typedef struct {
    Position pos;           /* Current screen position */
    Direction dir;          /* Current facing direction */
    EntityType type;        /* Entity type identifier */
    int active;             /* 1 = active, 0 = inactive/dead */
    int speed_counter;      /* Counter for movement timing */
    int speed_limit;        /* Ticks between movements (lower = faster) */
} Entity;

/**
 * @brief Player structure.
 * 
 * Use player.base for generic entity operations.
 */
typedef struct {
    Entity base;            /* Base entity data */
    PlayerState state;      /* Player-specific state */
    int is_pumping;         /* 1 = currently firing pump */
    int pump_length;        /* Current pump extension (0-4 cells) */
    Direction pump_dir;     /* Direction pump is facing */
} Player;

/**
 * @brief Enemy structure (for Pooka and Fygar).
 * 
 * fire_* fields only used when base.type == ENTITY_FYGAR.
 */
typedef struct {
    Entity base;            /* Base entity data */
    EnemyState state;       /* Enemy-specific state */
    int inflate_level;      /* Inflation level 0-4 (4 = explodes) */
    int ghost_timer;        /* Ticks until ghost mode activates */
    int fire_cooldown;      /* Fygar: ticks until can fire again */
    int fire_active;        /* Fygar: 1 = currently breathing fire */
    int fire_duration;      /* Fygar: remaining fire ticks */
} Enemy;

/**
 * @brief Rock structure.
 */
typedef struct {
    Entity base;            /* Base entity data */
    RockState state;        /* Rock-specific state */
    int wobble_timer;       /* Frames wobbling before fall */
    int has_crushed;        /* 1 = has crushed something this fall */
} Rock;

/**
 * @brief Input state structure - UNIFIED VERSION.
 * Updated by keyboard handler, read by game logic.
 */
typedef struct {
    Direction direction;        /* Current movement direction */
    int action_pressed;         /* Action button pressed (space/enter) */
    int pause_pressed;          /* Pause button pressed (P/ESC) */
    int quit_pressed;           /* Quit button pressed (Q) */
    char last_key;              /* Last raw key pressed */
    int any_key_pressed;        /* Any key pressed flag */
    int keys_held[256];         /* Keys currently held down */
} InputState;

/**
 * @brief Main game state structure.
 * Contains all game data for current session.
 */
typedef struct {
    /* Scene management */
    GameScene scene;            /* Current game scene */
    
    /* Game progress (NOT in Player) */
    int score;                  /* Current score (0-99999) */
    int round;                  /* Current round number (1-99) */
    int lives;                  /* Remaining lives (0-5) */
    
    /* Timing */
    int time_elapsed;           /* Time in ticks since round start */
    int round_start_timer;      /* Timer for round start delay */
    
    /* Entities - using wrapper structs */
    Player player;              /* The player */
    Enemy enemies[MAX_ENEMIES]; /* Array of enemies (Pooka and Fygar) */
    Rock rocks[MAX_ROCKS];      /* Array of rocks */
    
    /* Entity counts */
    int enemy_count;            /* Total enemies in current round */
    int enemies_remaining;      /* Enemies still alive */
    int rock_count;             /* Total rocks in current round */
    
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

/**
 * @brief Check if entity type is an enemy (Pooka or Fygar).
 */
#define IS_ENEMY_TYPE(type) ((type) == ENTITY_POOKA || (type) == ENTITY_FYGAR)

/**
 * @brief Check if entity type can breathe fire (only Fygar).
 */
#define CAN_BREATHE_FIRE(type) ((type) == ENTITY_FYGAR)

#endif /* __GAME_TYPES_H__ */
