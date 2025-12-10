/**
 * @file game_map.c
 * @brief Map system implementation for ZeOS Miner game
 */

#include <game_map.h>
#include <game_types.h>
#include <game_config.h>
#include <libc.h>

/* Private map data */
static TileType g_map[MAP_HEIGHT][MAP_WIDTH];
static int g_gem_positions[MAX_GEMS][2]; /* Store gem positions for easier management */
static int g_current_gem_count = 0;

/* Private helper functions */
static int random_int(int max);
static void place_random_dirt(int density);
static void create_initial_tunnels(void);

/**
 * Initialize map for a specific level
 */
void map_init(int level) {
    map_clear();
    map_generate_level(level);
}

/**
 * Clear the entire map (fill with empty tiles)
 */
void map_clear(void) {
    int x, y;
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            g_map[y][x] = TILE_EMPTY;
        }
    }
    g_current_gem_count = 0;
}

/**
 * Get tile type at specific position
 */
TileType map_get_tile(int x, int y) {
    if (!map_is_valid_position(x, y)) {
        return TILE_WALL; /* Treat out-of-bounds as walls */
    }
    return g_map[y][x];
}

/**
 * Set tile type at specific position
 */
void map_set_tile(int x, int y, TileType type) {
    if (!map_is_valid_position(x, y)) {
        return;
    }
    g_map[y][x] = type;
}

/**
 * Check if position is within map bounds
 */
int map_is_valid_position(int x, int y) {
    return (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT);
}

/**
 * Check if position is walkable (player/enemies can move here)
 */
int map_is_walkable(int x, int y) {
    TileType tile = map_get_tile(x, y);
    return (tile == TILE_EMPTY || tile == TILE_GEM);
}

/**
 * Check if position can be dug (dirt can be removed)
 */
int map_is_diggable(int x, int y) {
    TileType tile = map_get_tile(x, y);
    return (tile == TILE_DIRT);
}

/**
 * Dig at position (convert dirt to empty space)
 */
void map_dig(int x, int y) {
    if (map_is_diggable(x, y)) {
        map_set_tile(x, y, TILE_EMPTY);
    }
}

/**
 * Place a tile at specific position
 */
void map_place_tile(int x, int y, TileType type) {
    map_set_tile(x, y, type);
}

/**
 * Count remaining gems on the map
 */
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

/**
 * Place gems randomly on the map
 */
void map_place_gems(int count) {
    int placed = 0;
    int attempts = 0;
    const int max_attempts = count * 10; /* Prevent infinite loop */
    
    g_current_gem_count = 0;
    
    while (placed < count && attempts < max_attempts) {
        int x = random_int(MAP_WIDTH);
        int y = random_int(MAP_HEIGHT);
        
        /* Place gem only on empty spaces */
        if (map_get_tile(x, y) == TILE_EMPTY) {
            map_set_tile(x, y, TILE_GEM);
            
            /* Store gem position for easy access */
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

/**
 * Remove gem at specific position
 */
void map_remove_gem(int x, int y) {
    if (map_get_tile(x, y) == TILE_GEM) {
        map_set_tile(x, y, TILE_EMPTY);
        
        /* Remove from gem positions array */
        for (int i = 0; i < g_current_gem_count; i++) {
            if (g_gem_positions[i][0] == x && g_gem_positions[i][1] == y) {
                /* Shift remaining gems down */
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

/**
 * Check if there's a gem at specific position
 */
int map_has_gem(int x, int y) {
    return (map_get_tile(x, y) == TILE_GEM);
}

/**
 * Generate level layout based on level number
 */
void map_generate_level(int level) {
    /* Create border walls */
    map_create_borders();
    
    /* Fill with dirt based on level difficulty */
    int dirt_density = 60 + (level * 5); /* Increase dirt density with level */
    if (dirt_density > 85) dirt_density = 85; /* Cap at 85% */
    
    place_random_dirt(dirt_density);
    
    /* Create some initial tunnels */
    create_initial_tunnels();
    
    /* Ensure player spawn area is clear */
    map_fill_area(1, 1, 3, 3, TILE_EMPTY);
    
    /* Place gems based on level */
    int gem_count = 3 + (level * 2);
    if (gem_count > MAX_GEMS) gem_count = MAX_GEMS;
    
    map_place_gems(gem_count);
}

/**
 * Create border walls around the map
 */
void map_create_borders(void) {
    int x, y;
    
    /* Top and bottom borders */
    for (x = 0; x < MAP_WIDTH; x++) {
        map_set_tile(x, 0, TILE_WALL);
        map_set_tile(x, MAP_HEIGHT - 1, TILE_WALL);
    }
    
    /* Left and right borders */
    for (y = 0; y < MAP_HEIGHT; y++) {
        map_set_tile(0, y, TILE_WALL);
        map_set_tile(MAP_WIDTH - 1, y, TILE_WALL);
    }
}

/**
 * Fill area with specific tile type
 */
void map_fill_area(int x1, int y1, int x2, int y2, TileType type) {
    int x, y;
    
    /* Ensure coordinates are in correct order */
    if (x1 > x2) { int temp = x1; x1 = x2; x2 = temp; }
    if (y1 > y2) { int temp = y1; y1 = y2; y2 = temp; }
    
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
            if (map_is_valid_position(x, y)) {
                map_set_tile(x, y, type);
            }
        }
    }
}

/**
 * Draw a line of tiles between two points
 */
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

/**
 * Get a random empty position on the map
 */
int map_get_random_empty_position(Position *pos) {
    int attempts = 0;
    const int max_attempts = 100;
    
    while (attempts < max_attempts) {
        int x = 1 + random_int(MAP_WIDTH - 2); /* Avoid borders */
        int y = 1 + random_int(MAP_HEIGHT - 2);
        
        if (map_is_walkable(x, y)) {
            pos->x = x;
            pos->y = y;
            return 1; /* Success */
        }
        attempts++;
    }
    
    /* Fallback: return a default position */
    pos->x = 1;
    pos->y = 1;
    return 0; /* Failed */
}

/**
 * Get a safe spawn position away from player
 */
int map_get_safe_spawn_position(Position *pos, int min_distance_from_player) {
    int attempts = 0;
    const int max_attempts = 50;
    
    /* Assume player is at (1,1) for now - this could be improved */
    Position player_pos = {1, 1};
    
    while (attempts < max_attempts) {
        Position candidate;
        if (map_get_random_empty_position(&candidate)) {
            /* Calculate distance from player */
            int dx = candidate.x - player_pos.x;
            int dy = candidate.y - player_pos.y;
            int distance = dx * dx + dy * dy; /* Squared distance */
            
            if (distance >= min_distance_from_player * min_distance_from_player) {
                *pos = candidate;
                return 1; /* Success */
            }
        }
        attempts++;
    }
    
    /* Fallback */
    return map_get_random_empty_position(pos);
}

/* Private helper functions */

/**
 * Simple random number generator (very basic)
 * In a real implementation, you might want to use a better PRNG
 */
static int random_int(int max) {
    static unsigned int seed = 12345; /* Simple seed */
    seed = seed * 1103515245 + 12345; /* Linear congruential generator */
    return (seed % max);
}

/**
 * Place dirt randomly based on density percentage
 */
static void place_random_dirt(int density) {
    int x, y;
    
    for (y = 1; y < MAP_HEIGHT - 1; y++) {
        for (x = 1; x < MAP_WIDTH - 1; x++) {
            /* Skip if already a wall */
            if (map_get_tile(x, y) == TILE_WALL) {
                continue;
            }
            
            /* Place dirt based on density */
            if (random_int(100) < density) {
                map_set_tile(x, y, TILE_DIRT);
            } else {
                map_set_tile(x, y, TILE_EMPTY);
            }
        }
    }
}

/**
 * Create some initial tunnels for gameplay
 */
static void create_initial_tunnels(void) {
    /* Create a few horizontal and vertical tunnels */
    
    /* Horizontal tunnel in the middle */
    int mid_y = MAP_HEIGHT / 2;
    map_draw_line(1, mid_y, MAP_WIDTH - 2, mid_y, TILE_EMPTY);
    
    /* Vertical tunnel */
    int mid_x = MAP_WIDTH / 2;
    map_draw_line(mid_x, 1, mid_x, MAP_HEIGHT - 2, TILE_EMPTY);
    
    /* A few random smaller tunnels */
    for (int i = 0; i < 3; i++) {
        int x1 = 1 + random_int(MAP_WIDTH - 2);
        int y1 = 1 + random_int(MAP_HEIGHT - 2);
        int x2 = 1 + random_int(MAP_WIDTH - 2);
        int y2 = 1 + random_int(MAP_HEIGHT - 2);
        
        map_draw_line(x1, y1, x2, y2, TILE_EMPTY);
    }
}

/**
 * Debug function to print map to console (for testing)
 */
void map_print_debug(void) {
    int x, y;
    
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            char c;
            switch (map_get_tile(x, y)) {
                case TILE_EMPTY: c = '.'; break;
                case TILE_DIRT:  c = '#'; break;
                case TILE_WALL:  c = 'X'; break;
                case TILE_GEM:   c = '*'; break;
                default:         c = '?'; break;
            }
            /* In a real implementation, you'd use your print function */
            /* printk("%c", c); */
        }
        /* printk("\n"); */
    }
}