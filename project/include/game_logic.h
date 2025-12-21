/**
 * @file game_logic.h
 * @brief Game logic system prototypes for Dig Dug clone.
 *
 * Milestone M5.8 - GameLogic
 * This header defines all game logic functions including:
 * - Player movement and actions
 * - Enemy AI and behavior
 * - Rock physics
 * - Collision detection
 * - Scoring system
 * - Game state management
 */

#ifndef __GAME_LOGIC_H__
#define __GAME_LOGIC_H__

#include <game_types.h>

/* ============================================================================
 *                              GLOBAL STATE
 * ============================================================================ */

/* Forward declaration for GameLogicState */
typedef struct GameLogicState_s GameLogicState;

/* Global pointer to current game state (for rock collision checks) */
extern GameLogicState *g_current_logic_state;

/* ============================================================================
 *                              CONSTANTS
 * ============================================================================ */

/* Movement delays (ticks between movements) */
#define PLAYER_MOVE_DELAY 6
#define ENEMY_MOVE_DELAY 8
#define GHOST_MODE_THRESHOLD (5 * TICKS_PER_SECOND) /* Ghost mode every 12 seconds */
#define RESPAWN_DELAY QUARTER_SECOND                /* Ticks before player respawns */
#define LEVEL_CLEAR_DELAY HALF_SECOND               /* Frames before next level (1 sec at 60 FPS) */
/* Note: ROUND_START_DELAY is already defined in game_config.h */

/* Rock mechanics */
#define ROCK_WOBBLE_TICKS EIGHTH_SECOND      /* Ticks rock wobbles before falling */
#define ROCK_BLINK_COUNT 4                   /* Number of times rock blinks when hitting earth */
#define ROCK_BLINK_DURATION SIXTEENTH_SECOND /* Ticks per blink cycle */

/* Scoring */
#define POINTS_LAYER1 200
#define POINTS_LAYER2 300
#define POINTS_LAYER3 400
#define POINTS_LAYER4 500
#define POINTS_ROCK_BONUS 2 /* Multiplier for rock kills */

/* Player starting position */
#define PLAYER_START_X 40
#define PLAYER_START_Y 2

/* Maximum pump extension */
#define MAX_PUMP_LENGTH 4

/* Enemy spawn configuration */
#define ENEMY_SPAWN_BASE_X 60  /* Base X position for enemy spawns */
#define ENEMY_SPAWN_BASE_Y 8   /* Base Y position for enemy spawns */
#define ENEMY_SPAWN_OFFSET_X 5 /* X offset between enemies */
#define ENEMY_SPAWN_OFFSET_Y 4 /* Y offset between enemies */
#define ENEMY_SPAWN_AREA_X 15  /* Area width for enemy spawns */
#define ENEMY_SPAWN_AREA_Y 12  /* Area height for enemy spawns */

/* Rock spawn configuration */
#define ROCK_SPAWN_BASE_X 20   /* Base X position for rocks */
#define ROCK_SPAWN_BASE_Y 6    /* Base Y position for rocks */
#define ROCK_SPAWN_OFFSET_X 15 /* X offset between rocks */
#define ROCK_SPAWN_OFFSET_Y 3  /* Y offset between rocks */

/* ============================================================================
 *                    EXTENDED GAME STATE STRUCTURE
 * ============================================================================ */

/**
 * @brief Extended game state for full game logic.
 *
 * This structure extends the basic GameState with additional fields
 * needed for complete game logic (enemies as Enemy struct, rocks, etc.)
 */
struct GameLogicState_s {
    /* Scene management */
    GameScene scene; /* Current game scene */

    /* Game progress */
    int score;             /* Current score (0-99999) */
    int round;             /* Current round number (1-99) */
    int lives;             /* Remaining lives (0-5) */
    int enemies_remaining; /* Enemies still alive */

    /* Timing */
    int time_elapsed;      /* Total game ticks elapsed */
    int round_start_timer; /* Timer for round transitions/respawn */

    /* Entities */
    Player player;              /* The player */
    Enemy enemies[MAX_ENEMIES]; /* Array of enemies */
    int enemy_count;            /* Total enemies in current round */
    Rock rocks[MAX_ROCKS];      /* Array of rocks */
    int rock_count;             /* Total rocks in current level */

    /* Game flags */
    int running; /* Game is running */
};

/* ============================================================================
 *                           INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize game logic state to defaults.
 * @param state Pointer to GameLogicState structure
 */
void logic_init(GameLogicState *state);

/**
 * @brief Start a new round.
 * @param state Pointer to GameLogicState structure
 * @param round Round number to start
 */
void logic_start_round(GameLogicState *state, int round);

/* ============================================================================
 *                          MAIN UPDATE LOOP
 * ============================================================================ */

/**
 * @brief Main logic update function (called each frame).
 * @param state Pointer to GameLogicState structure
 */
void logic_update(GameLogicState *state);

/* ============================================================================
 *                          PLAYER FUNCTIONS
 * ============================================================================ */

/**
 * @brief Update player based on input.
 * @param state Pointer to GameLogicState structure
 */
void logic_update_player(GameLogicState *state);

/**
 * @brief Move player in specified direction.
 * @param player Pointer to Player structure
 * @param dir Direction to move
 */
void logic_player_move(Player *player, Direction dir);

/**
 * @brief Player pump action (inflate enemies).
 * @param player Pointer to Player structure
 * @param state Pointer to GameLogicState structure
 */
void logic_player_pump(Player *player, GameLogicState *state);

/**
 * @brief Perform player attack.
 *
 * Attacks in the direction the player is facing.
 * Vertical (up/down): 2 blocks with '|'
 * Horizontal (left/right): 3 blocks with '-'
 * Enemies hit become paralyzed and die after 2 seconds.
 *
 * @param player Pointer to Player structure
 * @param state Pointer to GameLogicState structure
 * @return 1 if attack was performed, 0 otherwise
 */
int logic_player_attack(Player *player, GameLogicState *state);

/**
 * @brief Handle player death.
 * @param state Pointer to GameLogicState structure
 */
void logic_player_die(GameLogicState *state);

/**
 * @brief Respawn player after death.
 * @param state Pointer to GameLogicState structure
 */
void logic_player_respawn(GameLogicState *state);

/* ============================================================================
 *                          ENEMY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Update all enemies.
 * @param state Pointer to GameLogicState structure
 */
void logic_update_enemies(GameLogicState *state);

/**
 * @brief Execute enemy AI logic.
 * @param enemy Pointer to Enemy structure
 * @param player Pointer to Player structure
 */
void logic_enemy_ai(Enemy *enemy, Player *player);

/**
 * @brief Move enemy towards player.
 * @param enemy Pointer to Enemy structure
 * @param player Pointer to Player structure
 */
void logic_enemy_move_towards_player(Enemy *enemy, Player *player);

/**
 * @brief Handle enemy ghost mode movement.
 * @param enemy Pointer to Enemy structure
 * @param player Pointer to Player structure (Task 4: needed for pathfinding)
 */
void logic_enemy_ghost_mode(Enemy *enemy, Player *player);

/**
 * @brief Try to move enemy in a direction.
 * @param enemy Pointer to Enemy structure
 * @param dir Direction to move
 * @return 1 if move successful, 0 otherwise
 */
int logic_try_enemy_move(Enemy *enemy, Direction dir);

/**
 * @brief Inflate an enemy (from pump attack).
 * @param enemy Pointer to Enemy structure
 */
void logic_enemy_inflate(Enemy *enemy);

/**
 * @brief Deflate an enemy (when pump is released).
 * @param enemy Pointer to Enemy structure
 */
void logic_enemy_deflate(Enemy *enemy);

/**
 * @brief Check if enemy can see player (for ghost mode decision).
 * @param enemy Pointer to Enemy structure
 * @param player Pointer to Player structure
 * @return 1 if can see, 0 otherwise
 */
int logic_can_see_player(Enemy *enemy, Player *player);

/* ============================================================================
 *                          ROCK FUNCTIONS
 * ============================================================================ */

/**
 * @brief Update all rocks.
 * @param state Pointer to GameLogicState structure
 */
void logic_update_rocks(GameLogicState *state);

/**
 * @brief Check if rock should start falling.
 * @param rock Pointer to Rock structure
 */
void logic_rock_check_fall(Rock *rock);

/**
 * @brief Handle rock falling physics.
 * @param rock Pointer to Rock structure
 * @param state Pointer to GameLogicState structure
 */
void logic_rock_fall(Rock *rock, GameLogicState *state);

/* ============================================================================
 *                       COLLISION DETECTION
 * ============================================================================ */

/**
 * @brief Check collision between player and enemies.
 * @param player Pointer to Player structure
 * @param enemies Array of Enemy structures
 * @param count Number of enemies
 * @return Index of colliding enemy, -1 if none
 */
int logic_check_player_enemy_collision(Player *player, Enemy *enemies, int count);

/**
 * @brief Check collision between player and rocks.
 * @param player Pointer to Player structure
 * @param rocks Array of Rock structures
 * @param count Number of rocks
 * @return Index of colliding rock, -1 if none
 */
int logic_check_player_rock_collision(Player *player, Rock *rocks, int count);

/**
 * @brief Check if pump attack hits an enemy.
 * @param player Pointer to Player structure
 * @param enemies Array of Enemy structures
 * @param count Number of enemies
 * @return Index of hit enemy, -1 if none
 */
int logic_check_pump_hit(Player *player, Enemy *enemies, int count);

/**
 * @brief Check if falling rock crushes anything.
 * @param rock Pointer to Rock structure
 * @param state Pointer to GameLogicState structure
 * @return 1 if something was crushed, 0 otherwise
 */
int logic_check_rock_crush(Rock *rock, GameLogicState *state);

/* ============================================================================
 *                            SCORING
 * ============================================================================ */

/**
 * @brief Add points to score.
 * @param state Pointer to GameLogicState structure
 * @param points Points to add
 */
void logic_add_score(GameLogicState *state, int points);

/**
 * @brief Calculate points for killing enemy at given Y position.
 * @param y Y coordinate of enemy
 * @return Points value based on layer
 */
int logic_calculate_enemy_points(int y);

/**
 * @brief Get layer number (1-4) for a given Y position.
 * @param y Y coordinate
 * @return Layer number (1-4), or 0 if not in ground area
 */
int logic_get_layer(int y);

/* ============================================================================
 *                       GAME STATE MANAGEMENT
 * ============================================================================ */

/**
 * @brief Check if round is complete (all enemies dead).
 * @param state Pointer to GameLogicState structure
 */
void logic_check_round_complete(GameLogicState *state);

/**
 * @brief Check if game is over (no lives left).
 * @param state Pointer to GameLogicState structure
 */
void logic_check_game_over(GameLogicState *state);

/**
 * @brief Transition to next round.
 * @param state Pointer to GameLogicState structure
 */
void logic_transition_to_next_round(GameLogicState *state);

/* ============================================================================
 *                          FYGAR SPECIFIC
 * ============================================================================ */

/**
 * @brief Handle Fygar fire breathing attack.
 * @param fygar Pointer to Fygar enemy
 * @param state Pointer to GameLogicState structure
 */
void logic_fygar_fire(Enemy *fygar, GameLogicState *state);

/**
 * @brief Check if Fygar's fire hits player.
 * @param fygar Pointer to Fygar enemy
 * @param player Pointer to Player structure
 * @return 1 if fire hits player, 0 otherwise
 */
int logic_check_fire_collision(Enemy *fygar, Player *player);

/* ============================================================================
 *                          UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize a Player structure.
 * @param player Pointer to Player structure
 * @param x Initial X position
 * @param y Initial Y position
 */
void logic_player_init(Player *player, int x, int y);

/**
 * @brief Initialize an Enemy structure.
 * @param enemy Pointer to Enemy structure
 * @param x Initial X position
 * @param y Initial Y position
 * @param type Entity type (ENTITY_POOKA or ENTITY_FYGAR)
 */
void logic_enemy_init(Enemy *enemy, int x, int y, EntityType type);

/**
 * @brief Initialize a Rock structure.
 * @param rock Pointer to Rock structure
 * @param x Initial X position
 * @param y Initial Y position
 */
void logic_rock_init(Rock *rock, int x, int y);

/**
 * @brief Calculate absolute value.
 * @param x Value
 * @return Absolute value of x
 */
int logic_abs(int x);

#endif /* __GAME_LOGIC_H__ */
