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
    /* ===== ROUND 1: 1 enemy (1 Pooka) ===== */
    {
        .round_number = 1,
        .player_start_x = 10,
        .player_start_y = 2, /* Start in sky layer (row 2) */

        .enemies =
            {
                {60, 8, ENTITY_POOKA}, /* 1 Pooka */
            },
        .enemy_count = 1,

        .rocks =
            {
                {30, 10, ENTITY_ROCK},
                {50, 8, ENTITY_ROCK},
                {20, 15, ENTITY_ROCK},
                {65, 12, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {5, 2, 15, 2},  /* Horizontal tunnel in sky for player start */
                {58, 7, 62, 7}, /* Horizontal tunnel for enemy (5 cells) */
                {60, 7, 60, 9}, /* Vertical tunnel for enemy (3 cells) */
            },
        .tunnel_count = 3,

        .ghost_threshold = 400,
    },

    /* ===== ROUND 2: 2 enemies (1 Pooka, 1 Fygar) ===== */
    {
        .round_number = 2,
        .player_start_x = 10,
        .player_start_y = 2,

        .enemies =
            {
                {60, 6, ENTITY_POOKA},  /* 1 Pooka */
                {30, 14, ENTITY_FYGAR}, /* 1 Fygar */
            },
        .enemy_count = 2,

        .rocks =
            {
                {25, 7, ENTITY_ROCK},
                {45, 13, ENTITY_ROCK},
                {15, 18, ENTITY_ROCK},
                {70, 10, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {5, 2, 15, 2},    /* Player sky tunnel */
                {58, 5, 62, 5},   /* Pooka horizontal (5 cells) */
                {60, 5, 60, 7},   /* Pooka vertical (3 cells) */
                {28, 13, 32, 13}, /* Fygar horizontal (5 cells) */
                {30, 13, 30, 15}, /* Fygar vertical (3 cells) */
            },
        .tunnel_count = 5,

        .ghost_threshold = 300,
    },

    /* ===== ROUND 3: 3 enemies (2 Pooka, 1 Fygar) ===== */
    {
        .round_number = 3,
        .player_start_x = 10,
        .player_start_y = 2,

        .enemies =
            {
                {50, 6, ENTITY_POOKA},  /* Pooka 1 */
                {70, 12, ENTITY_POOKA}, /* Pooka 2 */
                {35, 18, ENTITY_FYGAR}, /* Fygar */
            },
        .enemy_count = 3,

        .rocks =
            {
                {20, 8, ENTITY_ROCK},
                {55, 15, ENTITY_ROCK},
                {40, 10, ENTITY_ROCK},
                {65, 20, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {5, 2, 20, 2},    /* Player sky */
                {48, 5, 52, 5},   /* Pooka 1 horizontal */
                {50, 5, 50, 7},   /* Pooka 1 vertical */
                {68, 11, 72, 11}, /* Pooka 2 horizontal */
                {70, 11, 70, 13}, /* Pooka 2 vertical */
                {33, 17, 37, 17}, /* Fygar horizontal */
                {35, 17, 35, 19}, /* Fygar vertical */
            },
        .tunnel_count = 7,

        .ghost_threshold = 250,
    },

    /* ===== ROUND 4: 4 enemies (2 Pooka, 2 Fygar) ===== */
    {
        .round_number = 4,
        .player_start_x = 40,
        .player_start_y = 2,

        .enemies =
            {
                {15, 8, ENTITY_POOKA},  /* Pooka 1 */
                {65, 8, ENTITY_POOKA},  /* Pooka 2 */
                {25, 16, ENTITY_FYGAR}, /* Fygar 1 */
                {55, 16, ENTITY_FYGAR}, /* Fygar 2 */
            },
        .enemy_count = 4,

        .rocks =
            {
                {20, 10, ENTITY_ROCK},
                {60, 10, ENTITY_ROCK},
                {40, 18, ENTITY_ROCK},
                {30, 14, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {35, 2, 45, 2},   /* Player sky */
                {13, 7, 17, 7},   /* Pooka 1 horizontal */
                {15, 7, 15, 9},   /* Pooka 1 vertical */
                {63, 7, 67, 7},   /* Pooka 2 horizontal */
                {65, 7, 65, 9},   /* Pooka 2 vertical */
                {23, 15, 27, 15}, /* Fygar 1 horizontal */
                {25, 15, 25, 17}, /* Fygar 1 vertical */
                {53, 15, 57, 15}, /* Fygar 2 horizontal */
                {55, 15, 55, 17}, /* Fygar 2 vertical */
            },
        .tunnel_count = 9,

        .ghost_threshold = 200,
    },

    /* ===== ROUND 5: 5 enemies (3 Pooka, 2 Fygar) ===== */
    {
        .round_number = 5,
        .player_start_x = 40,
        .player_start_y = 2,

        .enemies =
            {
                {10, 8, ENTITY_POOKA},  /* Pooka 1 */
                {70, 8, ENTITY_POOKA},  /* Pooka 2 */
                {40, 14, ENTITY_POOKA}, /* Pooka 3 */
                {20, 20, ENTITY_FYGAR}, /* Fygar 1 */
                {60, 20, ENTITY_FYGAR}, /* Fygar 2 */
            },
        .enemy_count = 5,

        .rocks =
            {
                {25, 10, ENTITY_ROCK},
                {55, 10, ENTITY_ROCK},
                {35, 17, ENTITY_ROCK},
                {45, 17, ENTITY_ROCK},
            },
        .rock_count = 4,

        .tunnels =
            {
                {35, 2, 45, 2},   /* Player sky */
                {8, 7, 12, 7},    /* Pooka 1 horizontal */
                {10, 7, 10, 9},   /* Pooka 1 vertical */
                {68, 7, 72, 7},   /* Pooka 2 horizontal */
                {70, 7, 70, 9},   /* Pooka 2 vertical */
                {38, 13, 42, 13}, /* Pooka 3 horizontal */
                {40, 13, 40, 15}, /* Pooka 3 vertical */
                {18, 19, 22, 19}, /* Fygar 1 horizontal */
                {20, 19, 20, 21}, /* Fygar 1 vertical */
                {58, 19, 62, 19}, /* Fygar 2 horizontal */
                {60, 19, 60, 21}, /* Fygar 2 vertical */
            },
        .tunnel_count = 11,

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
        /* Speed is set by logic_enemy_init based on enemy type */
        logic_enemy_init(enemy, spawn->x, spawn->y, spawn->type);
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

/**
 * @brief Place bonus items on the map (3 per level).
 *
 * Bonuses are placed at fixed positions based on round number.
 * They give 100 points when collected.
 */
static void data_place_bonuses(int round) {
    /* Fixed bonus positions for each level - spread across the map */
    /* Bonus 1: left side, Bonus 2: center, Bonus 3: right side */
    int bonus_positions[3][2];

    switch (round) {
    case 1:
        bonus_positions[0][0] = 20;
        bonus_positions[0][1] = 10;
        bonus_positions[1][0] = 40;
        bonus_positions[1][1] = 15;
        bonus_positions[2][0] = 65;
        bonus_positions[2][1] = 12;
        break;
    case 2:
        bonus_positions[0][0] = 15;
        bonus_positions[0][1] = 8;
        bonus_positions[1][0] = 45;
        bonus_positions[1][1] = 11;
        bonus_positions[2][0] = 70;
        bonus_positions[2][1] = 16;
        break;
    case 3:
        bonus_positions[0][0] = 25;
        bonus_positions[0][1] = 9;
        bonus_positions[1][0] = 40;
        bonus_positions[1][1] = 14;
        bonus_positions[2][0] = 60;
        bonus_positions[2][1] = 19;
        break;
    case 4:
        bonus_positions[0][0] = 20;
        bonus_positions[0][1] = 11;
        bonus_positions[1][0] = 45;
        bonus_positions[1][1] = 13;
        bonus_positions[2][0] = 55;
        bonus_positions[2][1] = 18;
        break;
    case 5:
    default:
        bonus_positions[0][0] = 25;
        bonus_positions[0][1] = 10;
        bonus_positions[1][0] = 50;
        bonus_positions[1][1] = 16;
        bonus_positions[2][0] = 65;
        bonus_positions[2][1] = 18;
        break;
    }

    /* Place the 3 bonuses */
    for (int i = 0; i < 3; i++) {
        map_set_tile(bonus_positions[i][0], bonus_positions[i][1], TILE_BONUS);
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

    /* 6. Place bonus items (3 per level, 100 points each) */
    data_place_bonuses(round);

    /* 7. Update state counters */
    state->enemy_count = level->enemy_count;
    state->enemies_remaining = level->enemy_count;
    state->rock_count = level->rock_count;
    state->round = round;

    /* 8. Apply difficulty modifiers for rounds beyond defined levels */
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
