/**
 * @file game_data.h
 * @brief Level data structures and loading functions for Dig Dug clone.
 *
 * Milestone M5.10 - GameData
 * Defines level layouts, enemy/rock spawn positions, and tunnel configurations.
 */

#ifndef __GAME_DATA_H__
#define __GAME_DATA_H__

#include <game_config.h>
#include <game_logic.h>
#include <game_types.h>

/* ============================================================================
 *                              CONSTANTS
 * ============================================================================ */

#define MAX_LEVELS 10  /* Maximum number of predefined levels */
#define MAX_TUNNELS 16 /* Maximum tunnels per level */
#define DATA_PLAYER_START_X 10
#define DATA_PLAYER_START_Y 5

/* ============================================================================
 *                              STRUCTURES
 * ============================================================================ */

/**
 * @brief Spawn point definition for entities.
 */
typedef struct {
    int x;           /* X position */
    int y;           /* Y position */
    EntityType type; /* Entity type (ENTITY_POOKA, ENTITY_FYGAR, ENTITY_ROCK) */
} EntitySpawn;

/**
 * @brief Tunnel definition (line from point A to point B).
 */
typedef struct {
    int x1, y1; /* Start point */
    int x2, y2; /* End point */
} TunnelDef;

/**
 * @brief Complete level data structure.
 */
typedef struct {
    int round_number; /* Level/round number */

    /* Player start position */
    int player_start_x;
    int player_start_y;

    /* Enemies */
    EntitySpawn enemies[MAX_ENEMIES];
    int enemy_count;

    /* Rocks */
    EntitySpawn rocks[MAX_ROCKS];
    int rock_count;

    /* Predefined tunnels */
    TunnelDef tunnels[MAX_TUNNELS];
    int tunnel_count;

    /* Difficulty settings */
    int ghost_threshold; /* Ticks before ghost mode activates */
} LevelData;

/* ============================================================================
 *                          LEVEL ACCESS FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get level data for a specific round.
 * @param round Round number (1-based)
 * @return Pointer to LevelData structure, or NULL if invalid
 */
const LevelData *data_get_level(int round);

/**
 * @brief Get the number of predefined levels.
 * @return Number of levels defined
 */
int data_get_num_levels(void);

/* ============================================================================
 *                          LEVEL LOADING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Load a complete level into game state.
 * @param round Round number to load
 * @param state Game logic state to populate
 */
void data_load_level(int round, GameLogicState *state);

/**
 * @brief Spawn enemies based on level data.
 * @param state Game logic state
 * @param level Level data with enemy spawn points
 */
void data_spawn_enemies(GameLogicState *state, const LevelData *level);

/**
 * @brief Spawn rocks based on level data.
 * @param state Game logic state
 * @param level Level data with rock spawn points
 */
void data_spawn_rocks(GameLogicState *state, const LevelData *level);

/**
 * @brief Create predefined tunnels in the map.
 * @param level Level data with tunnel definitions
 */
void data_create_tunnels(const LevelData *level);

/**
 * @brief Dig a tunnel from one point to another.
 * @param x1 Start X coordinate
 * @param y1 Start Y coordinate
 * @param x2 End X coordinate
 * @param y2 End Y coordinate
 */
void data_dig_tunnel(int x1, int y1, int x2, int y2);

#endif /* __GAME_DATA_H__ */
