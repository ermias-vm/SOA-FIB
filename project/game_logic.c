/**
 * @file game_logic.c
 * @brief Game logic implementation for Dig Dug clone.
 *
 * Milestone M5.8 - GameLogic
 * Implements all game logic including:
 * - Player movement and actions
 * - Enemy AI and behavior
 * - Rock physics
 * - Collision detection
 * - Scoring system
 * - Game state management
 */

#include <game_config.h>
#include <game_input.h>
#include <game_logic.h>
#include <game_map.h>
#include <game_types.h>
#include <libc.h>

/* ============================================================================
 *                          GLOBAL STATE
 * ============================================================================ */

/* Global pointer to current game state (for rock collision checks) */
GameLogicState *g_current_logic_state = 0;

/* ============================================================================
 *                          UTILITY FUNCTIONS
 * ============================================================================ */

int logic_abs(int x) {
    return (x < 0) ? -x : x;
}

/* ============================================================================
 *                           INITIALIZATION
 * ============================================================================ */

void logic_player_init(Player *player, int x, int y) {
    if (!player) return;

    player->base.pos.x = x;
    player->base.pos.y = y;
    player->base.dir = DIR_NONE;
    player->base.type = ENTITY_PLAYER;
    player->base.active = 1;
    player->base.speed_counter = 0;
    player->base.speed_limit = PLAYER_SPEED;

    player->state = PLAYER_IDLE;
    player->facing_dir = DIR_RIGHT; /* Start facing right */
    player->is_pumping = 0;
    player->pump_length = 0;
    player->pump_dir = DIR_NONE;
    player->is_attacking = 0;
    player->attack_timer = 0;
}

void logic_enemy_init(Enemy *enemy, int x, int y, EntityType type) {
    if (!enemy) return;

    enemy->base.pos.x = x;
    enemy->base.pos.y = y;
    enemy->base.dir = DIR_NONE;
    enemy->base.type = type;
    enemy->base.active = 1;
    enemy->base.speed_counter = 0;

    /* Set speed based on enemy type */
    if (type == ENTITY_FYGAR) {
        enemy->base.speed_limit = FYGAR_BASE_SPEED;
    } else {
        enemy->base.speed_limit = POOKA_BASE_SPEED;
    }

    enemy->state = ENEMY_NORMAL;
    enemy->inflate_level = 0;
    enemy->ghost_timer = 0;
    enemy->fire_start_time = 0;
    enemy->fire_end_time = 0;
    enemy->fire_active = 0;
    enemy->fire_duration = 0;
    enemy->paralyzed_timer = 0;
    enemy->has_left_tunnel = 0;
}

void logic_rock_init(Rock *rock, int x, int y) {
    if (!rock) return;

    rock->base.pos.x = x;
    rock->base.pos.y = y;
    rock->base.dir = DIR_NONE;
    rock->base.type = ENTITY_ROCK;
    rock->base.active = 1;
    rock->base.speed_counter = 0;
    rock->base.speed_limit = 1;

    rock->state = ROCK_STABLE;
    rock->wobble_timer = 0;
    rock->has_crushed = 0;
    rock->blink_timer = 0;
    rock->blink_count = 0;
}

void logic_init(GameLogicState *state) {
    if (!state) return;

    state->scene = SCENE_MENU;
    state->score = 0;
    state->round = 1;
    state->lives = INITIAL_LIVES;
    state->time_elapsed = 0;
    state->round_start_timer = 0;
    state->enemies_cleared_time = 0;
    state->running = 1;
    state->enemies_remaining = 0;
    state->enemy_count = 0;
    state->rock_count = 0;

    /* Initialize player */
    logic_player_init(&state->player, PLAYER_START_X, PLAYER_START_Y);

    /* Clear enemies and rocks arrays */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        state->enemies[i].base.active = 0;
    }
    for (int i = 0; i < MAX_ROCKS; i++) {
        state->rocks[i].base.active = 0;
    }
}

void logic_start_round(GameLogicState *state, int round) {
    if (!state) return;

    state->round = round;
    /* Don't reset time_elapsed - keep accumulating across rounds */
    state->scene = SCENE_ROUND_START;
    state->round_start_timer = ROUND_START_DELAY;
    state->enemies_cleared_time = 0; /* Reset round clear delay tracker */

    /* Initialize player at starting position */
    logic_player_init(&state->player, PLAYER_START_X, PLAYER_START_Y);

    /* Initialize map for this level */
    map_init(round);

    /* Calculate number of enemies based on round */
    int num_enemies = 2 + (round / 2); /* More enemies as rounds progress */
    if (num_enemies > MAX_ENEMIES) num_enemies = MAX_ENEMIES;

    state->enemy_count = num_enemies;
    state->enemies_remaining = num_enemies;

    /* Place enemies using configured spawn positions */
    for (int i = 0; i < num_enemies; i++) {
        int ex = ENEMY_SPAWN_BASE_X + (i * ENEMY_SPAWN_OFFSET_X) % ENEMY_SPAWN_AREA_X;
        int ey = ENEMY_SPAWN_BASE_Y + (i * ENEMY_SPAWN_OFFSET_Y) % ENEMY_SPAWN_AREA_Y;

        /* Alternate between Pooka and Fygar */
        EntityType type = (i % 2 == 0) ? ENTITY_POOKA : ENTITY_FYGAR;
        logic_enemy_init(&state->enemies[i], ex, ey, type);
    }

    /* Place rocks using configured spawn positions - 4 per map */
    state->rock_count = 4;
    if (state->rock_count > MAX_ROCKS) state->rock_count = MAX_ROCKS;

    for (int i = 0; i < state->rock_count; i++) {
        int rx = ROCK_SPAWN_BASE_X + (i * ROCK_SPAWN_OFFSET_X);
        int ry = ROCK_SPAWN_BASE_Y + (i * ROCK_SPAWN_OFFSET_Y);
        logic_rock_init(&state->rocks[i], rx, ry);
    }
}

/* ============================================================================
 *                          MAIN UPDATE LOOP
 * ============================================================================ */

void logic_update(GameLogicState *state) {
    if (!state) return;

    /* Update global state pointer for rock collision checks */
    g_current_logic_state = state;

    /* Handle different scenes */
    switch (state->scene) {
    case SCENE_PLAYING:
        /* Main gameplay - continue below */
        break;

    case SCENE_ROUND_CLEAR:
        /* Waiting for timer before next round */
        if (state->round_start_timer > 0) {
            state->round_start_timer--;
            if (state->round_start_timer == 0) {
                logic_transition_to_next_round(state);
            }
        }
        return;

    case SCENE_ROUND_START:
        /* Short delay at round start */
        if (state->round_start_timer > 0) {
            state->round_start_timer--;
            if (state->round_start_timer == 0) {
                state->scene = SCENE_PLAYING;
            }
        }
        return;

    default:
        /* Not in playable state (menu, game over, paused) */
        return;
    }

    /* Handle respawn timer during gameplay */
    if (state->round_start_timer > 0) {
        state->round_start_timer--;
        if (state->round_start_timer == 0) {
            if (state->player.state == PLAYER_DEAD && state->lives > 0) {
                logic_player_respawn(state);
            }
        }
        return; /* Don't update game during timer */
    }

    /* Increment time */
    state->time_elapsed++;

    /* Update player */
    logic_update_player(state);

    /* Update enemies */
    logic_update_enemies(state);

    /* Update rocks */
    logic_update_rocks(state);

    /* Check win/lose conditions */
    logic_check_round_complete(state);
    logic_check_game_over(state);
}

/* ============================================================================
 *                          PLAYER FUNCTIONS
 * ============================================================================ */

void logic_update_player(GameLogicState *state) {
    if (!state) return;

    Player *player = &state->player;

    /* Don't update if dead */
    if (player->state == PLAYER_DEAD) {
        return;
    }

    /* Get direction from player entity (set by game.c) */
    Direction dir = player->base.dir;
    int action = player->is_pumping;

    /* Process movement */
    if (dir != DIR_NONE) {
        /* Check speed counter (movement cooldown) */
        if (player->base.speed_counter <= 0) {
            logic_player_move(player, dir);
            player->base.speed_counter = PLAYER_MOVE_DELAY;

            /* Check for bonus collection after moving (100 points) */
            if (map_get_tile(player->base.pos.x, player->base.pos.y) == TILE_BONUS) {
                map_set_tile(player->base.pos.x, player->base.pos.y, TILE_EMPTY);
                state->score += 100;
                if (state->score > MAX_SCORE) {
                    state->score = MAX_SCORE;
                }
            }
        }
        /* Clear direction after processing */
        player->base.dir = DIR_NONE;
    }

    /* Decrement speed counter */
    if (player->base.speed_counter > 0) {
        player->base.speed_counter--;
    }

    /* Update attack timer */
    if (player->is_attacking) {
        if (player->attack_timer > 0) {
            player->attack_timer--;
        } else {
            player->is_attacking = 0;
            if (player->state == PLAYER_ATTACKING) {
                player->state = PLAYER_IDLE;
            }
        }
    }

    /* Process pump action */
    if (action) {
        logic_player_pump(player, state);
    } else if (player->is_pumping) {
        /* Stop pumping when button released */
        player->is_pumping = 0;
        player->pump_length = 0;
        player->state = PLAYER_IDLE;
    }

    /* Check collision with enemies */
    int enemy_idx = logic_check_player_enemy_collision(player, state->enemies, state->enemy_count);
    if (enemy_idx >= 0) {
        /* Only die if enemy is not being inflated or paralyzed */
        EnemyState es = state->enemies[enemy_idx].state;
        if (es != ENEMY_INFLATING && es != ENEMY_PARALYZED) {
            logic_player_die(state);
        }
    }

    /* Check collision with falling rocks */
    int rock_idx = logic_check_player_rock_collision(player, state->rocks, state->rock_count);
    if (rock_idx >= 0 && state->rocks[rock_idx].state == ROCK_FALLING) {
        logic_player_die(state);
    }
}

/**
 * @brief Check if there's a rock at the given position.
 */
static int has_rock_at(Rock *rocks, int count, int x, int y) {
    for (int i = 0; i < count; i++) {
        if (rocks[i].base.active && rocks[i].base.pos.x == x && rocks[i].base.pos.y == y &&
            (rocks[i].state == ROCK_STABLE || rocks[i].state == ROCK_WOBBLING)) {
            return 1;
        }
    }
    return 0;
}

void logic_player_move(Player *player, Direction dir) {
    if (!player || !player->base.active) return;

    int new_x = player->base.pos.x;
    int new_y = player->base.pos.y;

    switch (dir) {
    case DIR_UP:
        new_y--;
        break;
    case DIR_DOWN:
        new_y++;
        break;
    case DIR_LEFT:
        new_x--;
        break;
    case DIR_RIGHT:
        new_x++;
        break;
    default:
        return;
    }

    /* Update facing direction (for rendering) */
    player->facing_dir = dir;

    /* Check bounds */
    if (!map_is_valid_position(new_x, new_y)) {
        return;
    }

    /* Prevent going above the sky area (player can only be at row ROW_SKY_END minimum) */
    if (new_y < ROW_SKY_END) {
        return;
    }

    /* Prevent going to the border row (max depth is ROW_GROUND_END - 1) */
    if (new_y >= ROW_BORDER) {
        return;
    }

    /* Check for rocks - they block movement */
    extern GameLogicState *g_current_logic_state;
    if (g_current_logic_state && has_rock_at(g_current_logic_state->rocks,
                                             g_current_logic_state->rock_count, new_x, new_y)) {
        return; /* Can't move through rocks */
    }

    /* If there's dirt, dig it */
    if (map_is_diggable(new_x, new_y)) {
        map_dig(new_x, new_y);
        player->state = PLAYER_DIGGING;
    } else if (map_is_walkable(new_x, new_y)) {
        player->state = PLAYER_MOVING;
    } else {
        /* Blocked by wall, rock, or other solid tile */
        return;
    }

    /* Move player */
    player->base.pos.x = new_x;
    player->base.pos.y = new_y;

    /* Check for gem collection */
    if (map_has_gem(new_x, new_y)) {
        map_remove_gem(new_x, new_y);
        /* Could add gem score here */
    }
    /* Bonus collection is handled in logic_update_player with state access */
}

/**
 * @brief Check if attack path is clear of solid blocks.
 *
 * @param x Starting X position
 * @param y Starting Y position
 * @param dir Direction of attack
 * @param range Number of cells to check
 * @return Number of cells that are clear (up to range)
 */
static int attack_path_clear(int x, int y, Direction dir, int range) {
    int dx = 0, dy = 0;

    switch (dir) {
    case DIR_UP:
        dy = -1;
        break;
    case DIR_DOWN:
        dy = 1;
        break;
    case DIR_LEFT:
        dx = -1;
        break;
    case DIR_RIGHT:
        dx = 1;
        break;
    default:
        return 0;
    }

    int clear_count = 0;
    for (int i = 1; i <= range; i++) {
        int check_x = x + dx * i;
        int check_y = y + dy * i;

        /* Check if position is valid and not solid */
        if (!map_is_valid_position(check_x, check_y)) {
            break;
        }
        if (map_is_solid(check_x, check_y)) {
            break;
        }
        clear_count++;
    }

    return clear_count;
}

/**
 * @brief Perform player attack.
 *
 * Attacks in the direction the player is facing.
 * Vertical (up/down): 2 blocks with '|'
 * Horizontal (left/right): 3 blocks with '-'
 *
 * @param player Pointer to player
 * @param state Pointer to game logic state
 * @return 1 if attack was performed, 0 otherwise
 */
int logic_player_attack(Player *player, GameLogicState *state) {
    if (!player || !state) return 0;
    if (player->state == PLAYER_DEAD) return 0;

    Direction dir = player->facing_dir;
    int range;
    int dx = 0, dy = 0;

    /* Determine attack range based on direction */
    switch (dir) {
    case DIR_UP:
        range = ATTACK_RANGE_V;
        dy = -1;
        break;
    case DIR_DOWN:
        range = ATTACK_RANGE_V;
        dy = 1;
        break;
    case DIR_LEFT:
        range = ATTACK_RANGE_H;
        dx = -1;
        break;
    case DIR_RIGHT:
        range = ATTACK_RANGE_H;
        dx = 1;
        break;
    default:
        return 0;
    }

    /* Check how many cells are clear */
    int clear = attack_path_clear(player->base.pos.x, player->base.pos.y, dir, range);
    if (clear == 0) {
        /* No clear path for attack */
        return 0;
    }

    /* Set player attacking state */
    player->is_attacking = 1;
    player->attack_timer = ATTACK_DISPLAY_FRAMES;
    player->state = PLAYER_ATTACKING;

    /* Check for enemy hits in attack range */
    int px = player->base.pos.x;
    int py = player->base.pos.y;

    for (int i = 1; i <= clear; i++) {
        int check_x = px + dx * i;
        int check_y = py + dy * i;

        /* Check each enemy */
        for (int e = 0; e < state->enemy_count; e++) {
            Enemy *enemy = &state->enemies[e];
            if (!enemy->base.active) continue;
            if (enemy->state == ENEMY_PARALYZED) continue; /* Already paralyzed */

            if (enemy->base.pos.x == check_x && enemy->base.pos.y == check_y) {
                /* Hit! Paralyze the enemy - 10 blinks then dies */
                enemy->state = ENEMY_PARALYZED;
                enemy->blink_count = 10;    /* 10 blinks before death */
                enemy->paralyzed_timer = 5; /* Ticks per blink cycle */
            }
        }
    }

    return 1;
}

void logic_player_pump(Player *player, GameLogicState *state) {
    if (!player || !state) return;

    player->is_pumping = 1;
    player->state = PLAYER_PUMPING;
    player->pump_dir = player->base.dir;

    /* Extend pump */
    if (player->pump_length < MAX_PUMP_LENGTH) {
        player->pump_length++;
    }

    /* Check if pump hits an enemy */
    int hit_idx = logic_check_pump_hit(player, state->enemies, state->enemy_count);
    if (hit_idx >= 0) {
        logic_enemy_inflate(&state->enemies[hit_idx]);
    }
}

void logic_player_die(GameLogicState *state) {
    if (!state) return;

    state->player.state = PLAYER_DEAD;
    state->player.is_pumping = 0;
    state->player.pump_length = 0;
    state->lives--;

    if (state->lives > 0) {
        /* Schedule respawn */
        state->round_start_timer = RESPAWN_DELAY;
    }
}

void logic_player_respawn(GameLogicState *state) {
    if (!state) return;

    logic_player_init(&state->player, PLAYER_START_X, PLAYER_START_Y);
    state->player.state = PLAYER_IDLE;
}

/* ============================================================================
 *                          ENEMY FUNCTIONS
 * ============================================================================ */

/* Task 1: Dijkstra Pathfinding structures and functions */
#define MAX_PRIORITY_QUEUE_SIZE 256

typedef struct {
    Position pos;
    Direction first_dir;
    int cost;
} DijkstraNode;

typedef struct {
    DijkstraNode nodes[MAX_PRIORITY_QUEUE_SIZE];
    int size;
} PriorityQueue;

void pq_init(PriorityQueue *pq) {
    pq->size = 0;
}

int pq_empty(PriorityQueue *pq) {
    return pq->size == 0;
}

int pq_full(PriorityQueue *pq) {
    return pq->size >= MAX_PRIORITY_QUEUE_SIZE;
}

void pq_push(PriorityQueue *pq, Position pos, Direction first_dir, int cost) {
    if (pq_full(pq)) return;

    /* Insert at end */
    pq->nodes[pq->size].pos = pos;
    pq->nodes[pq->size].first_dir = first_dir;
    pq->nodes[pq->size].cost = cost;

    /* Bubble up to maintain min-heap property */
    int i = pq->size;
    pq->size++;

    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->nodes[i].cost >= pq->nodes[parent].cost) break;

        /* Swap with parent */
        DijkstraNode temp = pq->nodes[i];
        pq->nodes[i] = pq->nodes[parent];
        pq->nodes[parent] = temp;
        i = parent;
    }
}

DijkstraNode pq_pop(PriorityQueue *pq) {
    DijkstraNode result = {{-1, -1}, DIR_NONE, 9999};
    if (pq_empty(pq)) return result;

    result = pq->nodes[0];
    pq->size--;

    if (pq->size > 0) {
        pq->nodes[0] = pq->nodes[pq->size];

        /* Bubble down */
        int i = 0;
        while (1) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;

            if (left < pq->size && pq->nodes[left].cost < pq->nodes[smallest].cost) {
                smallest = left;
            }
            if (right < pq->size && pq->nodes[right].cost < pq->nodes[smallest].cost) {
                smallest = right;
            }

            if (smallest == i) break;

            /* Swap */
            DijkstraNode temp = pq->nodes[i];
            pq->nodes[i] = pq->nodes[smallest];
            pq->nodes[smallest] = temp;
            i = smallest;
        }
    }

    return result;
}

/**
 * @brief Task 1: Find shortest path using Dijkstra with weighted costs
 * @param can_pass_walls If 1, can move through solid tiles (ghost mode)
 * @return Direction to move, or DIR_NONE if no path
 */
Direction logic_find_path_bfs(Position start, Position target, int can_pass_walls) {
    static PriorityQueue pq;
    static int cost[MAP_HEIGHT][MAP_WIDTH];
    static Direction first_dir[MAP_HEIGHT][MAP_WIDTH];

    /* Initialize costs to infinity */
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            cost[y][x] = 9999;
            first_dir[y][x] = DIR_NONE;
        }
    }

    pq_init(&pq);
    pq_push(&pq, start, DIR_NONE, 0);
    cost[start.y][start.x] = 0;

    Direction dirs[] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    while (!pq_empty(&pq)) {
        DijkstraNode current = pq_pop(&pq);

        /* Check if we reached target */
        if (current.pos.x == target.x && current.pos.y == target.y) {
            return first_dir[target.y][target.x];
        }

        /* Skip if we already found a better path */
        if (current.cost > cost[current.pos.y][current.pos.x]) {
            continue;
        }

        /* Explore neighbors */
        for (int i = 0; i < 4; i++) {
            int nx = current.pos.x + dx[i];
            int ny = current.pos.y + dy[i];

            /* Check bounds */
            if (!map_is_valid_position(nx, ny)) continue;

            /* Calculate movement cost */
            int move_cost = 1; /* Default cost for tunnels */

            if (can_pass_walls) {
                /* In ghost mode: tunnels cost 1, walls cost 10 */
                if (!map_is_walkable(nx, ny)) {
                    move_cost = 10; /* Penalty for passing through walls */
                }
            } else {
                /* Normal mode: can't pass walls */
                if (!map_is_walkable(nx, ny)) {
                    continue;
                }
            }

            int new_cost = current.cost + move_cost;

            /* Update if we found a better path */
            if (new_cost < cost[ny][nx]) {
                cost[ny][nx] = new_cost;

                /* Track first direction from start */
                Direction dir_to_use =
                    (current.first_dir == DIR_NONE) ? dirs[i] : current.first_dir;
                first_dir[ny][nx] = dir_to_use;

                Position next_pos = {nx, ny};
                pq_push(&pq, next_pos, dir_to_use, new_cost);
            }
        }
    }

    /* No path found */
    return DIR_NONE;
}

/**
 * @brief Task 1: Get random movement direction that is valid
 */
Direction logic_get_random_direction(Enemy *enemy) {
    Direction dirs[] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};

    /* Simple pseudo-random: use position and time as seed */
    int seed = enemy->base.pos.x + enemy->base.pos.y * 79 + enemy->ghost_timer;
    int start_idx = seed % 4;

    /* Try starting from random direction and find first valid one */
    for (int i = 0; i < 4; i++) {
        int idx = (start_idx + i) % 4;
        Direction dir = dirs[idx];

        /* Check if can move in this direction without actually moving */
        int new_x = enemy->base.pos.x;
        int new_y = enemy->base.pos.y;

        switch (dir) {
        case DIR_UP:
            new_y--;
            break;
        case DIR_DOWN:
            new_y++;
            break;
        case DIR_LEFT:
            new_x--;
            break;
        case DIR_RIGHT:
            new_x++;
            break;
        default:
            continue;
        }

        /* Check if valid move */
        if (map_is_valid_position(new_x, new_y) && map_is_walkable(new_x, new_y) &&
            new_y < ROW_BORDER) {
            /* Check for rocks */
            int blocked = 0;
            if (g_current_logic_state) {
                for (int r = 0; r < g_current_logic_state->rock_count; r++) {
                    Rock *rock = &g_current_logic_state->rocks[r];
                    if (rock->base.active && rock->base.pos.x == new_x &&
                        rock->base.pos.y == new_y) {
                        blocked = 1;
                        break;
                    }
                }
            }
            if (!blocked) {
                return dir;
            }
        }
    }

    return DIR_NONE;
}

void logic_update_enemies(GameLogicState *state) {
    if (!state) return;

    for (int i = 0; i < state->enemy_count; i++) {
        Enemy *enemy = &state->enemies[i];

        if (!enemy->base.active || enemy->state == ENEMY_DEAD) {
            continue;
        }

        /* Handle paralyzed enemies - blink 10 times then die */
        if (enemy->state == ENEMY_PARALYZED) {
            if (enemy->paralyzed_timer > 0) {
                enemy->paralyzed_timer--;
            } else {
                /* Reset timer and decrement blink count */
                enemy->blink_count--;
                if (enemy->blink_count <= 0) {
                    /* All blinks done - enemy dies and gives score */
                    enemy->state = ENEMY_DEAD;
                    enemy->base.active = 0;
                    state->enemies_remaining--;
                    /* Add score based on enemy depth */
                    int points = logic_calculate_enemy_points(enemy->base.pos.y);
                    logic_add_score(state, points);
                } else {
                    /* Reset timer for next blink (5 ticks per blink cycle) */
                    enemy->paralyzed_timer = 5;
                }
            }
            continue;
        }

        /* If being inflated, handle deflation when player stops pumping */
        if (enemy->state == ENEMY_INFLATING) {
            if (!state->player.is_pumping) {
                logic_enemy_deflate(enemy);
            }
            continue;
        }

        /* Speed counter check */
        if (enemy->base.speed_counter > 0) {
            enemy->base.speed_counter--;
            continue;
        }

        /* Fygar fire handling - check before movement */
        if (enemy->base.type == ENTITY_FYGAR) {
            /* If fire is active, Fygar stops moving */
            if (enemy->fire_active) {
                logic_fygar_fire(enemy, state);
                continue; /* Skip movement while attacking */
            }
            logic_fygar_fire(enemy, state);
            /* If fire just started, skip movement */
            if (enemy->fire_active) {
                continue;
            }
        }

        /* Execute AI */
        logic_enemy_ai(enemy, &state->player);

        /* Reset speed counter */
        enemy->base.speed_counter = enemy->base.speed_limit;

        /* Update ghost timer */
        enemy->ghost_timer++;
        if (enemy->ghost_timer > GHOST_MODE_THRESHOLD && enemy->state == ENEMY_NORMAL) {
            /* Activate ghost mode if stuck */
            if (!logic_can_see_player(enemy, &state->player)) {
                enemy->state = ENEMY_GHOST;
                enemy->has_left_tunnel = 0; /* Initialize ghost mode flag */
            }
        }
    }
}

void logic_enemy_ai(Enemy *enemy, Player *player) {
    if (!enemy || !player) return;

    if (enemy->state == ENEMY_GHOST) {
        logic_enemy_ghost_mode(enemy, player);
    } else {
        logic_enemy_move_towards_player(enemy, player);
    }
}

void logic_enemy_move_towards_player(Enemy *enemy, Player *player) {
    if (!enemy || !player) return;

    Position enemy_pos = {enemy->base.pos.x, enemy->base.pos.y};
    Position player_pos = {player->base.pos.x, player->base.pos.y};

    /* Task 1: Use BFS pathfinding to find actual shortest path */
    Direction best_dir = logic_find_path_bfs(enemy_pos, player_pos, 0);

    if (best_dir != DIR_NONE && logic_try_enemy_move(enemy, best_dir)) {
        enemy->ghost_timer = 0; /* Reset ghost timer on successful move */
        return;
    }

    /* Task 1: No path found - use random movement */
    Direction random_dir = logic_get_random_direction(enemy);
    if (random_dir != DIR_NONE) {
        logic_try_enemy_move(enemy, random_dir);
    }
}

int logic_try_enemy_move(Enemy *enemy, Direction dir) {
    if (!enemy) return 0;

    int new_x = enemy->base.pos.x;
    int new_y = enemy->base.pos.y;

    switch (dir) {
    case DIR_UP:
        new_y--;
        break;
    case DIR_DOWN:
        new_y++;
        break;
    case DIR_LEFT:
        new_x--;
        break;
    case DIR_RIGHT:
        new_x++;
        break;
    default:
        return 0;
    }

    /* Check if can move */
    if (!map_is_valid_position(new_x, new_y)) {
        return 0;
    }

    /* Check for rocks - enemies can't move through rocks */
    if (g_current_logic_state && has_rock_at(g_current_logic_state->rocks,
                                             g_current_logic_state->rock_count, new_x, new_y)) {
        return 0; /* Can't move through rocks */
    }

    /* Limit depth - enemies can't go to border row */
    if (new_y >= ROW_BORDER) {
        return 0;
    }

    if (map_is_walkable(new_x, new_y)) {
        enemy->base.pos.x = new_x;
        enemy->base.pos.y = new_y;
        enemy->base.dir = dir;
        return 1;
    }

    return 0;
}

void logic_enemy_ghost_mode(Enemy *enemy, Player *player) {
    if (!enemy) return;

    Position enemy_pos = {enemy->base.pos.x, enemy->base.pos.y};
    Position player_pos = {player->base.pos.x, player->base.pos.y};

    /* Check if currently on a tunnel */
    int on_tunnel = map_is_walkable(enemy->base.pos.x, enemy->base.pos.y);

    /* Task 4: Return to normal only if: on tunnel AND has previously left a tunnel */
    if (on_tunnel && enemy->has_left_tunnel) {
        enemy->state = ENEMY_NORMAL;
        enemy->ghost_timer = 0;
        enemy->has_left_tunnel = 0;
        return;
    }

    /* Track if we've moved through dirt (left tunnel) */
    if (!on_tunnel && !enemy->has_left_tunnel) {
        enemy->has_left_tunnel = 1;
    }

    /* Task 4: Use pathfinding with wall-passing enabled to find shortest path */
    Direction best_dir = logic_find_path_bfs(enemy_pos, player_pos, 1);

    if (best_dir != DIR_NONE) {
        /* Apply the movement (ghosts can pass through walls) */
        int new_x = enemy->base.pos.x;
        int new_y = enemy->base.pos.y;

        switch (best_dir) {
        case DIR_UP:
            new_y--;
            break;
        case DIR_DOWN:
            new_y++;
            break;
        case DIR_LEFT:
            new_x--;
            break;
        case DIR_RIGHT:
            new_x++;
            break;
        default:
            break;
        }

        if (map_is_valid_position(new_x, new_y)) {
            enemy->base.pos.x = new_x;
            enemy->base.pos.y = new_y;
            enemy->base.dir = best_dir;

            /* Check if reached a tunnel after leaving one */
            if (map_is_walkable(new_x, new_y) && enemy->has_left_tunnel) {
                enemy->state = ENEMY_NORMAL;
                enemy->ghost_timer = 0;
                enemy->has_left_tunnel = 0;
            }
        }
    }
}

int logic_can_see_player(Enemy *enemy, Player *player) {
    if (!enemy || !player) return 0;

    /* Simple line-of-sight check: same row or column with clear path */
    int ex = enemy->base.pos.x;
    int ey = enemy->base.pos.y;
    int px = player->base.pos.x;
    int py = player->base.pos.y;

    /* Check horizontal line */
    if (ey == py) {
        int start = (ex < px) ? ex : px;
        int end = (ex < px) ? px : ex;
        for (int x = start; x <= end; x++) {
            if (!map_is_walkable(x, ey)) {
                return 0;
            }
        }
        return 1;
    }

    /* Check vertical line */
    if (ex == px) {
        int start = (ey < py) ? ey : py;
        int end = (ey < py) ? py : ey;
        for (int y = start; y <= end; y++) {
            if (!map_is_walkable(ex, y)) {
                return 0;
            }
        }
        return 1;
    }

    return 0;
}

void logic_enemy_inflate(Enemy *enemy) {
    if (!enemy || enemy->state == ENEMY_DEAD) return;

    enemy->state = ENEMY_INFLATING;
    enemy->inflate_level++;

    /* Explode if max inflation reached */
    if (enemy->inflate_level >= INFLATE_LEVELS) {
        enemy->state = ENEMY_DEAD;
        enemy->base.active = 0;

        /* Use global state to update enemies_remaining and score */
        if (g_current_logic_state) {
            g_current_logic_state->enemies_remaining--;
            /* Add score based on enemy depth */
            int points = logic_calculate_enemy_points(enemy->base.pos.y);
            logic_add_score(g_current_logic_state, points);
        }
    }
}

void logic_enemy_deflate(Enemy *enemy) {
    if (!enemy || enemy->state != ENEMY_INFLATING) return;

    enemy->inflate_level--;

    if (enemy->inflate_level <= 0) {
        enemy->state = ENEMY_NORMAL;
        enemy->inflate_level = 0;
    }
}

/* ============================================================================
 *                          ROCK FUNCTIONS
 * ============================================================================ */

void logic_update_rocks(GameLogicState *state) {
    if (!state) return;

    for (int i = 0; i < state->rock_count; i++) {
        Rock *rock = &state->rocks[i];

        if (!rock->base.active) {
            continue;
        }

        switch (rock->state) {
        case ROCK_STABLE:
            logic_rock_check_fall(rock);
            break;

        case ROCK_WOBBLING:
            rock->wobble_timer--;
            if (rock->wobble_timer <= 0) {
                rock->state = ROCK_FALLING;
            }
            break;

        case ROCK_FALLING:
            logic_rock_fall(rock, state);
            break;

        case ROCK_BLINKING:
            /* Task 5: Handle blink animation */
            rock->blink_timer--;
            if (rock->blink_timer <= 0) {
                rock->blink_count--;
                if (rock->blink_count <= 0) {
                    /* Done blinking, make rock inactive */
                    rock->state = ROCK_LANDED;
                    rock->base.active = 0;
                } else {
                    /* Reset timer for next blink cycle */
                    rock->blink_timer = ROCK_BLINK_DURATION;
                }
            }
            break;

        case ROCK_LANDED:
            /* Rock is done - could be cleaned up */
            rock->base.active = 0;
            break;

        default:
            break;
        }
    }
}

void logic_rock_check_fall(Rock *rock) {
    if (!rock) return;

    int below_x = rock->base.pos.x;
    int below_y = rock->base.pos.y + 1;

    /* Check if there's empty space below */
    if (map_is_valid_position(below_x, below_y) && !map_is_solid(below_x, below_y)) {
        rock->state = ROCK_WOBBLING;
        rock->wobble_timer = ROCK_WOBBLE_TICKS;
    }
}

void logic_rock_fall(Rock *rock, GameLogicState *state) {
    if (!rock || !state) return;

    int new_y = rock->base.pos.y + 1;

    /* Task 5: Check if hit solid earth block */
    if (!map_is_valid_position(rock->base.pos.x, new_y) || map_is_solid(rock->base.pos.x, new_y)) {
        /* Rock hit earth - start blinking animation */
        rock->state = ROCK_BLINKING;
        rock->blink_count = ROCK_BLINK_COUNT;
        rock->blink_timer = ROCK_BLINK_DURATION;
        return;
    }

    /* Check if crushes something */
    logic_check_rock_crush(rock, state);

    /* Move rock down */
    rock->base.pos.y = new_y;
}

/* ============================================================================
 *                       COLLISION DETECTION
 * ============================================================================ */

int logic_check_player_enemy_collision(Player *player, Enemy *enemies, int count) {
    if (!player || !enemies || !player->base.active) return -1;

    for (int i = 0; i < count; i++) {
        if (enemies[i].base.active && enemies[i].state != ENEMY_DEAD) {
            if (player->base.pos.x == enemies[i].base.pos.x &&
                player->base.pos.y == enemies[i].base.pos.y) {
                return i;
            }
        }
    }

    return -1;
}

int logic_check_player_rock_collision(Player *player, Rock *rocks, int count) {
    if (!player || !rocks || !player->base.active) return -1;

    for (int i = 0; i < count; i++) {
        if (rocks[i].base.active) {
            if (player->base.pos.x == rocks[i].base.pos.x &&
                player->base.pos.y == rocks[i].base.pos.y) {
                return i;
            }
        }
    }

    return -1;
}

int logic_check_pump_hit(Player *player, Enemy *enemies, int count) {
    if (!player || !enemies || !player->is_pumping) return -1;

    /* Calculate pump endpoint */
    int pump_x = player->base.pos.x;
    int pump_y = player->base.pos.y;

    for (int i = 1; i <= player->pump_length; i++) {
        switch (player->pump_dir) {
        case DIR_UP:
            pump_y--;
            break;
        case DIR_DOWN:
            pump_y++;
            break;
        case DIR_LEFT:
            pump_x--;
            break;
        case DIR_RIGHT:
            pump_x++;
            break;
        default:
            break;
        }

        /* Check for enemy at this position */
        for (int j = 0; j < count; j++) {
            if (enemies[j].base.active && enemies[j].state != ENEMY_DEAD &&
                enemies[j].base.pos.x == pump_x && enemies[j].base.pos.y == pump_y) {
                return j;
            }
        }

        /* Stop if hit a wall */
        if (!map_is_walkable(pump_x, pump_y)) {
            break;
        }
    }

    return -1;
}

int logic_check_rock_crush(Rock *rock, GameLogicState *state) {
    if (!rock || !state) return 0;

    int rx = rock->base.pos.x;
    int ry_current = rock->base.pos.y;   /* Current position */
    int ry_below = rock->base.pos.y + 1; /* Position below (where rock is falling to) */

    int crushed = 0;

    /* Check player at current rock position (rock is falling through) */
    if (state->player.base.pos.x == rx && state->player.base.pos.y == ry_current) {
        logic_player_die(state);
        rock->has_crushed = 1;
        crushed = 1;
    }

    /* Check player at position below (where rock is falling to) */
    if (state->player.base.pos.x == rx && state->player.base.pos.y == ry_below) {
        logic_player_die(state);
        rock->has_crushed = 1;
        crushed = 1;
    }

    /* Check enemies at current rock position (rock is falling through) */
    for (int i = 0; i < state->enemy_count; i++) {
        Enemy *enemy = &state->enemies[i];
        if (enemy->base.active && enemy->state != ENEMY_DEAD) {
            if (enemy->base.pos.x == rx &&
                (enemy->base.pos.y == ry_current || enemy->base.pos.y == ry_below)) {
                enemy->state = ENEMY_DEAD;
                enemy->base.active = 0;
                state->enemies_remaining--;

                /* Bonus points for rock kill with multiplier */
                int points = logic_calculate_enemy_points(enemy->base.pos.y) * ROCK_KILL_MULTIPLIER;
                logic_add_score(state, points);
                rock->has_crushed = 1;
                crushed = 1;
            }
        }
    }

    return crushed;
}

/* ============================================================================
 *                            SCORING
 * ============================================================================ */

void logic_add_score(GameLogicState *state, int points) {
    if (!state) return;

    state->score += points;
    if (state->score > MAX_SCORE) {
        state->score = MAX_SCORE;
    }
}

int logic_get_layer(int y) {
    if (y >= LAYER_1_START && y <= LAYER_1_END) return 1;
    if (y >= LAYER_2_START && y <= LAYER_2_END) return 2;
    if (y >= LAYER_3_START && y <= LAYER_3_END) return 3;
    if (y >= LAYER_4_START && y <= LAYER_4_END) return 4;
    return 0;
}

int logic_calculate_enemy_points(int y) {
    int layer = logic_get_layer(y);
    switch (layer) {
    case 1:
        return POINTS_LAYER1;
    case 2:
        return POINTS_LAYER2;
    case 3:
        return POINTS_LAYER3;
    case 4:
        return POINTS_LAYER4;
    default:
        return POINTS_LAYER1;
    }
}

/* ============================================================================
 *                       GAME STATE MANAGEMENT
 * ============================================================================ */

void logic_check_round_complete(GameLogicState *state) {
    if (!state) return;

    if (state->enemies_remaining <= 0) {
        int current_time = gettime();

        /* First time enemies reached 0 - record timestamp */
        if (state->enemies_cleared_time == 0) {
            state->enemies_cleared_time = current_time + 2*ONE_SECOND;
        }

        /* Wait 1 second (ONE_SECOND) before transitioning to round clear screen */
        /* This allows the game to continue running briefly after last enemy dies */
        if (state->enemies_cleared_time - current_time <= 0) {
            state->scene = SCENE_ROUND_CLEAR;
            state->round_start_timer = LEVEL_CLEAR_DELAY;
        }
    }
}

void logic_check_game_over(GameLogicState *state) {
    if (!state) return;

    if (state->lives <= 0) {
        state->scene = SCENE_GAME_OVER;
    }
}

void logic_transition_to_next_round(GameLogicState *state) {
    if (!state) return;

    state->round++;
    if (state->round > MAX_ROUNDS) {
        state->scene = SCENE_GAME_OVER; /* Victory could be a separate state */
    } else {
        logic_start_round(state, state->round);
    }
}

/* ============================================================================
 *                          FYGAR SPECIFIC
 * ============================================================================ */

void logic_fygar_fire(Enemy *fygar, GameLogicState *state) {
    if (!fygar || !state) return;
    if (fygar->base.type != ENTITY_FYGAR) return;

    int current_time = gettime();

    /* Handle active fire - check if duration has passed */
    if (fygar->fire_active) {
        if (current_time - fygar->fire_start_time >= FYGAR_FIRE_DURATION) {
            /* Fire duration ended */
            fygar->fire_active = 0;
            fygar->fire_end_time = current_time; /* Start cooldown */
        } else {
            /* Fire still active - check if hits player */
            if (logic_check_fire_collision(fygar, &state->player)) {
                logic_player_die(state);
            }
        }
        return;
    }

    /* Check cooldown - must wait 2 seconds after last attack */
    if (fygar->fire_end_time > 0 && current_time - fygar->fire_end_time < FYGAR_FIRE_COOLDOWN) {
        return; /* Still in cooldown */
    }

    /* Determine fire direction based on player position */
    int dx = state->player.base.pos.x - fygar->base.pos.x;
    Direction fire_dir = (dx >= 0) ? DIR_RIGHT : DIR_LEFT;

    /* Check if there are exactly 2 empty cells in fire direction */
    int check_dx = (fire_dir == DIR_RIGHT) ? 1 : -1;
    int empty_cells = 0;
    for (int i = 1; i <= FYGAR_FIRE_RANGE; i++) {
        int check_x = fygar->base.pos.x + (check_dx * i);
        if (map_is_valid_position(check_x, fygar->base.pos.y) &&
            map_is_walkable(check_x, fygar->base.pos.y)) {
            empty_cells++;
        } else {
            break;
        }
    }

    /* Fire only if exactly 2 empty cells in front */
    if (empty_cells == FYGAR_FIRE_RANGE) {
        fygar->fire_active = 1;
        fygar->fire_start_time = current_time;
        fygar->base.dir = fire_dir;
        return;
    }

    /* Try the other direction */
    fire_dir = (fire_dir == DIR_RIGHT) ? DIR_LEFT : DIR_RIGHT;
    check_dx = (fire_dir == DIR_RIGHT) ? 1 : -1;
    empty_cells = 0;
    for (int i = 1; i <= FYGAR_FIRE_RANGE; i++) {
        int check_x = fygar->base.pos.x + (check_dx * i);
        if (map_is_valid_position(check_x, fygar->base.pos.y) &&
            map_is_walkable(check_x, fygar->base.pos.y)) {
            empty_cells++;
        } else {
            break;
        }
    }
    /* Fire only if exactly 2 empty cells in front */
    if (empty_cells == FYGAR_FIRE_RANGE) {
        fygar->fire_active = 1;
        fygar->fire_start_time = current_time;
        fygar->base.dir = fire_dir;
    }
}

int logic_check_fire_collision(Enemy *fygar, Player *player) {
    if (!fygar || !player || !fygar->fire_active) return 0;

    int fy = fygar->base.pos.y;
    int py = player->base.pos.y;

    /* Fire only travels horizontally */
    if (fy != py) return 0;

    int fx = fygar->base.pos.x;
    int px = player->base.pos.x;

    /* Check fire direction */
    if (fygar->base.dir == DIR_RIGHT) {
        if (px > fx && px <= fx + FYGAR_FIRE_RANGE) {
            return 1;
        }
    } else if (fygar->base.dir == DIR_LEFT) {
        if (px < fx && px >= fx - FYGAR_FIRE_RANGE) {
            return 1;
        }
    }

    return 0;
}
