#ifndef __GAME_MAP_H__
#define __GAME_MAP_H__

#include <game_types.h>
#include <game_config.h>

/**
 * @file game_map.h
 * @brief Map system prototypes and functions
 */

/* Map initialization and cleanup */
void map_init(int level);
void map_clear(void);

/* Map access functions */
TileType map_get_tile(int x, int y);
void map_set_tile(int x, int y, TileType type);

/* Map validation */
int map_is_valid_position(int x, int y);
int map_is_walkable(int x, int y);
int map_is_diggable(int x, int y);

/* Map modification */
void map_dig(int x, int y);
void map_place_tile(int x, int y, TileType type);

/* Gem management */
int map_count_gems(void);
void map_place_gems(int count);
void map_remove_gem(int x, int y);
int map_has_gem(int x, int y);

/* Level generation */
void map_generate_level(int level);
void map_create_borders(void);
void map_create_dirt_pattern(int level);
void map_create_tunnels(int level);

/* Utility functions */
void map_fill_area(int x1, int y1, int x2, int y2, TileType type);
void map_draw_line(int x1, int y1, int x2, int y2, TileType type);
int map_get_random_empty_position(Position *pos);
int map_get_safe_spawn_position(Position *pos, int min_distance_from_player);

/* Debug functions */
void map_print_debug(void);

#endif /* __GAME_MAP_H__ */