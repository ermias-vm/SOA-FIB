/**
 * @file game_map.c
 * @brief Map system implementation for ZeOS Miner game
 */

#include <game_config.h>
#include <game_map.h>
#include <game_types.h>
#include <libc.h>

/* Private map data */
static TileType g_map[MAP_HEIGHT][MAP_WIDTH];
static int g_gem_positions[MAX_GEMS][2];
static int g_current_gem_count = 0;

/* ============================================================================
 *                         MAP INITIALIZATION & CLEANUP
 * ============================================================================ */

void map_init(int level) {
    map_clear();
    map_generate_level(level);
}

void map_clear(void) {
    int x, y;
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            g_map[y][x] = TILE_EMPTY;
        }
    }
    g_current_gem_count = 0;
}

/* ============================================================================
 *                            MAP ACCESS FUNCTIONS
 * ============================================================================ */

TileType map_get_tile(int x, int y) {
    if (!map_is_valid_position(x, y)) {
        return TILE_WALL;
    }
    return g_map[y][x];
}

void map_set_tile(int x, int y, TileType type) {
    if (!map_is_valid_position(x, y)) {
        return;
    }
    g_map[y][x] = type;
}

/* ============================================================================
 *                            MAP VALIDATION
 * ============================================================================ */

int map_is_valid_position(int x, int y) {
    return (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT);
}

int map_is_walkable(int x, int y) {
    TileType tile = map_get_tile(x, y);
    return (tile == TILE_EMPTY || tile == TILE_GEM || tile == TILE_BONUS || tile == TILE_SKY ||
            tile == TILE_BORDER);
}

int map_is_solid(int x, int y) {
    TileType tile = map_get_tile(x, y);
    return (tile == TILE_DIRT || tile == TILE_WALL);
}

int map_is_diggable(int x, int y) {
    TileType tile = map_get_tile(x, y);
    return (tile == TILE_DIRT);
}

/* ============================================================================
 *                           MAP MODIFICATION
 * ============================================================================ */

void map_dig(int x, int y) {
    if (map_is_diggable(x, y)) {
        map_set_tile(x, y, TILE_EMPTY);
    }
}

void map_place_tile(int x, int y, TileType type) {
    map_set_tile(x, y, type);
}

/* ============================================================================
 *                            GEM MANAGEMENT
 * ============================================================================ */

int map_count_gems(void) {
    int count = 0;
    int x, y;

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (g_map[y][x] == TILE_GEM) {
                count++;
            }
        }
    }
    return count;
}

void map_place_gems(int count) {
    int placed = 0;
    int attempts = 0;
    const int max_attempts = count * 10;

    g_current_gem_count = 0;

    while (placed < count && attempts < max_attempts) {
        int x = random_int(MAP_WIDTH);
        int y = random_int(MAP_HEIGHT);

        if (map_get_tile(x, y) == TILE_EMPTY) {
            map_set_tile(x, y, TILE_GEM);

            if (g_current_gem_count < MAX_GEMS) {
                g_gem_positions[g_current_gem_count][0] = x;
                g_gem_positions[g_current_gem_count][1] = y;
                g_current_gem_count++;
            }

            placed++;
        }
        attempts++;
    }
}

void map_remove_gem(int x, int y) {
    if (map_get_tile(x, y) == TILE_GEM) {
        map_set_tile(x, y, TILE_EMPTY);

        for (int i = 0; i < g_current_gem_count; i++) {
            if (g_gem_positions[i][0] == x && g_gem_positions[i][1] == y) {
                for (int j = i; j < g_current_gem_count - 1; j++) {
                    g_gem_positions[j][0] = g_gem_positions[j + 1][0];
                    g_gem_positions[j][1] = g_gem_positions[j + 1][1];
                }
                g_current_gem_count--;
                break;
            }
        }
    }
}

int map_has_gem(int x, int y) {
    return (map_get_tile(x, y) == TILE_GEM);
}

/* ============================================================================
 *                           LEVEL GENERATION
 * ============================================================================ */

void map_generate_level(int level) {
    map_create_borders();

    /* Fill the map completely with dirt (100% density) */
    int dirt_density = 100;

    place_random_dirt(dirt_density);

    /* Player spawn area in sky layer - keep as TILE_SKY */
    map_fill_area(1, ROW_SKY_START, 5, ROW_SKY_END, TILE_SKY);

    /* Note: Gems are optional, we can skip them or reduce count */
    int gem_count = 1 + (level / 2);
    if (gem_count > MAX_GEMS) gem_count = MAX_GEMS;

    /* Don't place gems automatically - tunnels will be created by data_load_level */
}

void map_create_borders(void) {
    int x, y;

    /* Top and bottom walls */
    for (x = 0; x < MAP_WIDTH; x++) {
        map_set_tile(x, 0, TILE_WALL);
        map_set_tile(x, MAP_HEIGHT - 1, TILE_WALL);
    }

    /* Left and right walls */
    for (y = 0; y < MAP_HEIGHT; y++) {
        map_set_tile(0, y, TILE_WALL);
        map_set_tile(MAP_WIDTH - 1, y, TILE_WALL);
    }

    /* Bottom border row (row 23) - gray # characters */
    for (x = 0; x < MAP_WIDTH; x++) {
        map_set_tile(x, ROW_BORDER, TILE_BORDER);
    }
}

void map_create_dirt_pattern(int level) {
    int dirt_density = 60 + (level * 5);
    if (dirt_density > 85) dirt_density = 85;
    place_random_dirt(dirt_density);
}

void map_create_tunnels(int level) {
    (void)level; /* Suppress unused parameter warning */
    create_initial_tunnels();
}

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

void map_fill_area(int x1, int y1, int x2, int y2, TileType type) {
    int x, y;

    if (x1 > x2) {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            if (map_is_valid_position(x, y)) {
                map_set_tile(x, y, type);
            }
        }
    }
}

void map_draw_line(int x1, int y1, int x2, int y2, TileType type) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (dx > dy ? (dx > -dx ? dx : -dx) : (dy > -dy ? dy : -dy));

    if (steps == 0) {
        map_set_tile(x1, y1, type);
        return;
    }

    float x_inc = (float)dx / steps;
    float y_inc = (float)dy / steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++) {
        map_set_tile((int)(x + 0.5), (int)(y + 0.5), type);
        x += x_inc;
        y += y_inc;
    }
}

int map_get_random_empty_position(Position *pos) {
    int attempts = 0;
    const int max_attempts = 100;

    while (attempts < max_attempts) {
        int x = 1 + random_int(MAP_WIDTH - 2);
        int y = 1 + random_int(MAP_HEIGHT - 2);

        if (map_is_walkable(x, y)) {
            pos->x = x;
            pos->y = y;
            return 1;
        }
        attempts++;
    }

    pos->x = 1;
    pos->y = 1;
    return 0;
}

int map_get_safe_spawn_position(Position *pos, int min_distance_from_player) {
    int attempts = 0;
    const int max_attempts = 50;
    Position player_pos = {1, 1};

    while (attempts < max_attempts) {
        Position candidate;
        if (map_get_random_empty_position(&candidate)) {
            int dx = candidate.x - player_pos.x;
            int dy = candidate.y - player_pos.y;
            int distance = dx * dx + dy * dy;

            if (distance >= min_distance_from_player * min_distance_from_player) {
                *pos = candidate;
                return 1;
            }
        }
        attempts++;
    }

    return map_get_random_empty_position(pos);
}

/* ============================================================================
 *                           RANDOM GENERATION
 * ============================================================================ */

int random_int(int max) {
    static unsigned int seed = 12345;
    seed = seed * 1103515245 + 12345;
    return (seed % max);
}

void place_random_dirt(int density) {
    int x, y;

    for (y = 1; y < MAP_HEIGHT - 1; y++) {
        for (x = 1; x < MAP_WIDTH - 1; x++) {
            if (map_get_tile(x, y) == TILE_WALL) {
                continue;
            }

            /* Sky layer - always empty, no dirt */
            if (y <= ROW_SKY_END) {
                map_set_tile(x, y, TILE_SKY);
                continue;
            }

            /* Fill everything with dirt (density 100% means always dirt) */
            if (density >= 100) {
                map_set_tile(x, y, TILE_DIRT);
            } else if (random_int(100) < density) {
                map_set_tile(x, y, TILE_DIRT);
            } else {
                map_set_tile(x, y, TILE_EMPTY);
            }
        }
    }
}

void create_initial_tunnels(void) {
    /* Horizontal tunnel in the middle of the ground */
    int mid_y = (ROW_GROUND_START + MAP_HEIGHT - 1) / 2;
    map_draw_line(1, mid_y, MAP_WIDTH - 2, mid_y, TILE_EMPTY);

    /* Vertical tunnel in the middle, starting from ground */
    int mid_x = MAP_WIDTH / 2;
    map_draw_line(mid_x, ROW_GROUND_START, mid_x, MAP_HEIGHT - 2, TILE_EMPTY);

    /* Random tunnels only in ground area */
    for (int i = 0; i < 3; i++) {
        int x1 = 1 + random_int(MAP_WIDTH - 2);
        int y1 = ROW_GROUND_START + random_int(MAP_HEIGHT - ROW_GROUND_START - 1);
        int x2 = 1 + random_int(MAP_WIDTH - 2);
        int y2 = ROW_GROUND_START + random_int(MAP_HEIGHT - ROW_GROUND_START - 1);

        map_draw_line(x1, y1, x2, y2, TILE_EMPTY);
    }
}

/* ============================================================================
 *                            DEBUG FUNCTIONS
 * ============================================================================ */

void map_print_debug(void) {
    (void)0; /* Debug function - implementation placeholder */
}
