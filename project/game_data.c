/**
 * @file game_data.c
 * @brief Level data and loading functions implementation.
 *
 * Milestone M5.10 - GameData
 * Contains predefined level layouts and functions to load them.
 */

#include <game_config.h>
#include <game_data.h>
#include <game_logic.h>
#include <game_map.h>
#include <game_types.h>

/* ============================================================================
 *                          LEVEL DATA DEFINITIONS
 * ============================================================================ */

/* Number of predefined levels */
#define NUM_LEVELS_DEFINED 5

/* Static level data array */
static const LevelData g_levels[NUM_LEVELS_DEFINED] = {
    /* ===== ROUND 1 ===== */
    {
        .round_number = 1,
        .player_start_x = 10,
        .player_start_y = 3, /* Start in sky layer (row 3) */

        .enemies =
            {
                {60, 8, ENTITY_POOKA}, /* Only 1 enemy for round 1 */
            },
        .enemy_count = 1,

        .rocks =
            {
                {30, 10, ENTITY_ROCK},
            },
        .rock_count = 1,

        .tunnels =
            {
                {5, 3, 25, 3},   /* Horizontal tunnel in sky for player start */
                {10, 3, 10, 12}, /* Vertical tunnel from sky down */
                {55, 8, 70, 8},  /* Horizontal tunnel where enemy is */
            },
        .tunnel_count = 3,

        .enemy_speed = 8, /* Slower for first round */
        .ghost_threshold = 400,
    },

    /* ===== ROUND 2 ===== */
    {
        .round_number = 2,
        .player_start_x = 10,
        .player_start_y = 5,

        .enemies =
            {
                {60, 5, ENTITY_POOKA},
                {30, 11, ENTITY_POOKA},
                {50, 16, ENTITY_FYGAR},
                {70, 20, ENTITY_POOKA},
            },
        .enemy_count = 4,

        .rocks =
            {
                {25, 7, ENTITY_ROCK},
                {45, 13, ENTITY_ROCK},
                {65, 18, ENTITY_ROCK},
            },
        .rock_count = 3,

        .tunnels =
            {
                {5, 5, 30, 5},
                {10, 5, 10, 20},
                {55, 5, 75, 5},
                {60, 5, 60, 15},
                {30, 11, 50, 11},
                {40, 16, 70, 16},
            },
        .tunnel_count = 6,

        .enemy_speed = 5,
        .ghost_threshold = 250,
    },

    /* ===== ROUND 3 ===== */
    {
        .round_number = 3,
        .player_start_x = 10,
        .player_start_y = 5,

        .enemies =
            {
                {50, 5, ENTITY_POOKA},
                {25, 10, ENTITY_FYGAR},
                {60, 14, ENTITY_POOKA},
                {35, 19, ENTITY_FYGAR},
                {70, 21, ENTITY_POOKA},
            },
        .enemy_count = 5,

        .rocks =
            {
                {20, 8, ENTITY_ROCK},
                {40, 12, ENTITY_ROCK},
                {55, 17, ENTITY_ROCK},
                {30, 22, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {5, 5, 20, 5},
                {45, 5, 70, 5},
                {10, 5, 10, 22},
                {50, 5, 50, 10},
                {20, 10, 35, 10},
                {55, 14, 75, 14},
                {30, 19, 45, 19},
            },
        .tunnel_count = 7,

        .enemy_speed = 5,
        .ghost_threshold = 200,
    },

    /* ===== ROUND 4 ===== */
    {
        .round_number = 4,
        .player_start_x = 40,
        .player_start_y = 5,

        .enemies =
            {
                {10, 8, ENTITY_POOKA},
                {70, 8, ENTITY_POOKA},
                {20, 15, ENTITY_FYGAR},
                {60, 15, ENTITY_FYGAR},
                {40, 20, ENTITY_POOKA},
                {30, 22, ENTITY_POOKA},
            },
        .enemy_count = 6,

        .rocks =
            {
                {15, 10, ENTITY_ROCK},
                {65, 10, ENTITY_ROCK},
                {35, 17, ENTITY_ROCK},
                {55, 21, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {35, 5, 45, 5},
                {40, 5, 40, 10},
                {5, 8, 25, 8},
                {55, 8, 75, 8},
                {10, 8, 10, 20},
                {70, 8, 70, 20},
                {15, 15, 30, 15},
                {50, 15, 65, 15},
                {25, 20, 55, 20},
            },
        .tunnel_count = 9,

        .enemy_speed = 4,
        .ghost_threshold = 180,
    },

    /* ===== ROUND 5 ===== */
    {
        .round_number = 5,
        .player_start_x = 40,
        .player_start_y = 3,

        .enemies =
            {
                {10, 10, ENTITY_FYGAR},
                {70, 10, ENTITY_FYGAR},
                {20, 16, ENTITY_POOKA},
                {60, 16, ENTITY_POOKA},
                {30, 21, ENTITY_FYGAR},
                {50, 21, ENTITY_FYGAR},
            },
        .enemy_count = 6,

        .rocks =
            {
                {25, 8, ENTITY_ROCK},
                {55, 8, ENTITY_ROCK},
                {15, 14, ENTITY_ROCK},
                {65, 14, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {35, 3, 45, 3},
                {40, 3, 40, 8},
                {5, 10, 30, 10},
                {50, 10, 75, 10},
                {10, 10, 10, 22},
                {70, 10, 70, 22},
                {15, 16, 35, 16},
                {45, 16, 65, 16},
                {20, 21, 60, 21},
            },
        .tunnel_count = 9,

        .enemy_speed = 4,
        .ghost_threshold = 150,
    },
};

/* ============================================================================
 *                          LEVEL ACCESS FUNCTIONS
 * ============================================================================ */

const LevelData *data_get_level(int round) {
    if (round < 1) {
        return &g_levels[0];
    }

    int index = round - 1;

    /* If beyond defined levels, use last level as template */
    if (index >= NUM_LEVELS_DEFINED) {
        index = NUM_LEVELS_DEFINED - 1;
    }

    return &g_levels[index];
}

int data_get_num_levels(void) {
    return NUM_LEVELS_DEFINED;
}

/* ============================================================================
 *                          TUNNEL CREATION
 * ============================================================================ */

void data_dig_tunnel(int x1, int y1, int x2, int y2) {
    /* Determine direction */
    int dx = (x2 > x1) ? 1 : ((x2 < x1) ? -1 : 0);
    int dy = (y2 > y1) ? 1 : ((y2 < y1) ? -1 : 0);

    int x = x1;
    int y = y1;

    /* Dig until destination reached */
    while (1) {
        /* Dig current cell */
        if (map_is_valid_position(x, y)) {
            map_dig(x, y);
        }

        /* Check if destination reached */
        if (x == x2 && y == y2) {
            break;
        }

        /* Advance (horizontal first, then vertical) */
        if (x != x2) {
            x += dx;
        } else if (y != y2) {
            y += dy;
        }
    }
}

void data_create_tunnels(const LevelData *level) {
    if (!level) return;

    for (int i = 0; i < level->tunnel_count && i < MAX_TUNNELS; i++) {
        const TunnelDef *tunnel = &level->tunnels[i];
        data_dig_tunnel(tunnel->x1, tunnel->y1, tunnel->x2, tunnel->y2);
    }
}

/* ============================================================================
 *                          ENTITY SPAWNING
 * ============================================================================ */

void data_spawn_enemies(GameLogicState *state, const LevelData *level) {
    if (!state || !level) return;

    for (int i = 0; i < level->enemy_count && i < MAX_ENEMIES; i++) {
        const EntitySpawn *spawn = &level->enemies[i];
        Enemy *enemy = &state->enemies[i];

        /* Initialize enemy using logic function */
        logic_enemy_init(enemy, spawn->x, spawn->y, spawn->type);

        /* Apply level-specific speed */
        enemy->base.speed_limit = level->enemy_speed;
    }

    /* Deactivate unused slots */
    for (int i = level->enemy_count; i < MAX_ENEMIES; i++) {
        state->enemies[i].base.active = 0;
    }
}

void data_spawn_rocks(GameLogicState *state, const LevelData *level) {
    if (!state || !level) return;

    for (int i = 0; i < level->rock_count && i < MAX_ROCKS; i++) {
        const EntitySpawn *spawn = &level->rocks[i];
        Rock *rock = &state->rocks[i];

        /* Initialize rock using logic function */
        logic_rock_init(rock, spawn->x, spawn->y);
    }

    /* Deactivate unused slots */
    for (int i = level->rock_count; i < MAX_ROCKS; i++) {
        state->rocks[i].base.active = 0;
    }
}

/* ============================================================================
 *                          LEVEL LOADING
 * ============================================================================ */

void data_load_level(int round, GameLogicState *state) {
    if (!state) return;

    const LevelData *level = data_get_level(round);
    if (!level) return;

    /* 1. Initialize map */
    map_init(round);

    /* 2. Create predefined tunnels */
    data_create_tunnels(level);

    /* 3. Initialize player at starting position */
    logic_player_init(&state->player, level->player_start_x, level->player_start_y);

    /* 4. Spawn enemies */
    data_spawn_enemies(state, level);

    /* 5. Spawn rocks */
    data_spawn_rocks(state, level);

    /* 6. Update state counters */
    state->enemy_count = level->enemy_count;
    state->enemies_remaining = level->enemy_count;
    state->rock_count = level->rock_count;
    state->round = round;

    /* 7. Apply difficulty modifiers for rounds beyond defined levels */
    if (round > NUM_LEVELS_DEFINED) {
        int difficulty_bonus = round - NUM_LEVELS_DEFINED;

        /* Enemies get faster */
        for (int i = 0; i < state->enemy_count; i++) {
            state->enemies[i].base.speed_limit -= difficulty_bonus;
            if (state->enemies[i].base.speed_limit < 2) {
                state->enemies[i].base.speed_limit = 2;
            }
        }
    }
}
