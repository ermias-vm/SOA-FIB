#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include <game_config.h>
#include <game_types.h>

/**
 * @file game_map.h
 * @brief Map system prototypes and functions with complete documentation
 */

/* ============================================================================
 *                         MAP INITIALIZATION & CLEANUP
 * ============================================================================ */

/**
 * @brief Initialize map for a specific level/round.
 * @param level The level number (1-based)
 */
void map_init(int level);

/**
 * @brief Clear the entire map (fill with empty tiles).
 */
void map_clear(void);

/* ============================================================================
 *                            MAP ACCESS FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get tile type at specific position.
 * @param x Column position (0-79)
 * @param y Row position (0-24)
 * @return TileType at position, TILE_WALL if out of bounds
 */
TileType map_get_tile(int x, int y);

/**
 * @brief Set tile type at specific position.
 * @param x Column position
 * @param y Row position
 * @param type New tile type to set
 */
void map_set_tile(int x, int y, TileType type);

/* ============================================================================
 *                            MAP VALIDATION
 * ============================================================================ */

/**
 * @brief Check if position is within map bounds.
 * @param x Column position
 * @param y Row position
 * @return 1 if valid position, 0 otherwise
 */
int map_is_valid_position(int x, int y);

/**
 * @brief Check if position is walkable (player/enemies can move here).
 * @param x Column position
 * @param y Row position
 * @return 1 if walkable (TILE_EMPTY, TILE_GEM), 0 otherwise
 */
int map_is_walkable(int x, int y);

/**
 * @brief Check if a tile is solid (blocks movement).
 * @param x Column position
 * @param y Row position
 * @return 1 if solid (TILE_DIRT, TILE_WALL), 0 otherwise
 */
int map_is_solid(int x, int y);

/**
 * @brief Check if position can be dug (dirt can be removed).
 * @param x Column position
 * @param y Row position
 * @return 1 if diggable (TILE_DIRT), 0 otherwise
 */
int map_is_diggable(int x, int y);

/* ============================================================================
 *                           MAP MODIFICATION
 * ============================================================================ */

/**
 * @brief Dig at position (convert dirt to empty space).
 * @param x Column position
 * @param y Row position
 */
void map_dig(int x, int y);

/**
 * @brief Place a tile at specific position.
 * @param x Column position
 * @param y Row position
 * @param type Tile type to place
 */
void map_place_tile(int x, int y, TileType type);

/* ============================================================================
 *                            GEM MANAGEMENT
 * ============================================================================ */

/**
 * @brief Count remaining gems on the map.
 * @return Number of TILE_GEM tiles on map
 */
int map_count_gems(void);

/**
 * @brief Place gems randomly on the map.
 * @param count Number of gems to place
 */
void map_place_gems(int count);

/**
 * @brief Remove gem at specific position.
 * @param x Column position
 * @param y Row position
 */
void map_remove_gem(int x, int y);

/**
 * @brief Check if there's a gem at specific position.
 * @param x Column position
 * @param y Row position
 * @return 1 if gem present, 0 otherwise
 */
int map_has_gem(int x, int y);

/* ============================================================================
 *                           LEVEL GENERATION
 * ============================================================================ */

/**
 * @brief Generate level layout based on level number.
 * @param level Level number (affects difficulty and layout)
 */
void map_generate_level(int level);

/**
 * @brief Create border walls around the map.
 */
void map_create_borders(void);

/**
 * @brief Create dirt pattern for specified level.
 * @param level Level number (affects dirt density)
 */
void map_create_dirt_pattern(int level);

/**
 * @brief Create initial tunnels for gameplay.
 * @param level Level number (affects tunnel complexity)
 */
void map_create_tunnels(int level);

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Fill rectangular area with specific tile type.
 * @param x1 Left coordinate
 * @param y1 Top coordinate
 * @param x2 Right coordinate
 * @param y2 Bottom coordinate
 * @param type Tile type to fill with
 */
void map_fill_area(int x1, int y1, int x2, int y2, TileType type);

/**
 * @brief Draw a line of tiles between two points.
 * @param x1 Start X coordinate
 * @param y1 Start Y coordinate
 * @param x2 End X coordinate
 * @param y2 End Y coordinate
 * @param type Tile type for the line
 */
void map_draw_line(int x1, int y1, int x2, int y2, TileType type);

/**
 * @brief Get a random empty position on the map.
 * @param pos Pointer to Position structure to fill
 * @return 1 if successful, 0 if failed to find empty position
 */
int map_get_random_empty_position(Position *pos);

/**
 * @brief Get a safe spawn position away from player.
 * @param pos Pointer to Position structure to fill
 * @param min_distance_from_player Minimum distance from player
 * @return 1 if successful, 0 if failed
 */
int map_get_safe_spawn_position(Position *pos, int min_distance_from_player);

/* ============================================================================
 *                           RANDOM GENERATION
 * ============================================================================ */

/**
 * @brief Generate random integer from 0 to max-1.
 * @param max Upper bound (exclusive)
 * @return Random integer in range [0, max)
 */
int random_int(int max);

/**
 * @brief Place dirt randomly based on density percentage.
 * @param density Percentage of tiles to fill with dirt (0-100)
 */
void place_random_dirt(int density);

/**
 * @brief Create some initial tunnels for gameplay.
 */
void create_initial_tunnels(void);

/* ============================================================================
 *                            DEBUG FUNCTIONS
 * ============================================================================ */

/**
 * @brief Debug function to print map to console (for testing).
 */
void map_print_debug(void);

#endif /* __GAME_MAP_H__ */
