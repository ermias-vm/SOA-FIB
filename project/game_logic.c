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
    enemy->fire_cooldown = 0;
    enemy->fire_active = 0;
    enemy->fire_duration = 0;
    enemy->paralyzed_timer = 0;
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
}

void logic_init(GameLogicState *state) {
    if (!state) return;

    state->scene = SCENE_MENU;
    state->score = 0;
    state->round = 1;
    state->lives = INITIAL_LIVES;
    state->time_elapsed = 0;
    state->round_start_timer = 0;
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
    state->time_elapsed = 0;
    state->scene = SCENE_ROUND_START;
    state->round_start_timer = ROUND_START_DELAY;

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

    /* Place rocks using configured spawn positions */
    state->rock_count = 1 + (round / 3);
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

    /* Prevent going into sky area (rows 0-2, player can only be at row 3 minimum) */
    if (new_y < ROW_SKY_END) {
        return;
    }

    /* If there's dirt, dig it */
    if (map_is_diggable(new_x, new_y)) {
        map_dig(new_x, new_y);
        player->state = PLAYER_DIGGING;
    } else if (map_is_walkable(new_x, new_y)) {
        player->state = PLAYER_MOVING;
    } else {
        /* Blocked by wall or other solid tile */
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
                /* Hit! Paralyze the enemy */
                enemy->state = ENEMY_PARALYZED;
                enemy->paralyzed_timer = ENEMY_PARALYSIS_TIME;
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

void logic_update_enemies(GameLogicState *state) {
    if (!state) return;

    for (int i = 0; i < state->enemy_count; i++) {
        Enemy *enemy = &state->enemies[i];

        if (!enemy->base.active || enemy->state == ENEMY_DEAD) {
            continue;
        }

        /* Handle paralyzed enemies - countdown and then kill */
        if (enemy->state == ENEMY_PARALYZED) {
            if (enemy->paralyzed_timer > 0) {
                enemy->paralyzed_timer--;
            } else {
                /* Paralysis expired - enemy dies and gives score */
                enemy->state = ENEMY_DEAD;
                enemy->base.active = 0;
                state->enemies_remaining--;
                /* Add base score (200 points) */
                state->score += 200;
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
            }
        }

        /* Fygar fire handling */
        if (enemy->base.type == ENTITY_FYGAR) {
            logic_fygar_fire(enemy, state);
        }
    }
}

void logic_enemy_ai(Enemy *enemy, Player *player) {
    if (!enemy || !player) return;

    if (enemy->state == ENEMY_GHOST) {
        logic_enemy_ghost_mode(enemy);
    } else {
        logic_enemy_move_towards_player(enemy, player);
    }
}

void logic_enemy_move_towards_player(Enemy *enemy, Player *player) {
    if (!enemy || !player) return;

    int ex = enemy->base.pos.x;
    int ey = enemy->base.pos.y;
    int px = player->base.pos.x;
    int py = player->base.pos.y;

    /* Calculate distance */
    int dx = px - ex;
    int dy = py - ey;

    /* Determine primary and secondary directions */
    Direction primary, secondary;

    if (logic_abs(dx) > logic_abs(dy)) {
        /* Prioritize horizontal movement */
        primary = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
        secondary = (dy > 0) ? DIR_DOWN : DIR_UP;
    } else {
        /* Prioritize vertical movement */
        primary = (dy > 0) ? DIR_DOWN : DIR_UP;
        secondary = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
    }

    /* Try primary direction */
    if (logic_try_enemy_move(enemy, primary)) {
        enemy->ghost_timer = 0;
        return;
    }

    /* Try secondary direction */
    if (logic_try_enemy_move(enemy, secondary)) {
        return;
    }

    /* Try other directions */
    Direction dirs[] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
    for (int i = 0; i < 4; i++) {
        if (dirs[i] != primary && dirs[i] != secondary) {
            if (logic_try_enemy_move(enemy, dirs[i])) {
                return;
            }
        }
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
    if (map_is_valid_position(new_x, new_y) && map_is_walkable(new_x, new_y)) {
        enemy->base.pos.x = new_x;
        enemy->base.pos.y = new_y;
        enemy->base.dir = dir;
        return 1;
    }

    return 0;
}

void logic_enemy_ghost_mode(Enemy *enemy) {
    if (!enemy) return;

    int new_x = enemy->base.pos.x;
    int new_y = enemy->base.pos.y;
    Direction current_dir = enemy->base.dir;

    /* If no direction set, pick one */
    if (current_dir == DIR_NONE) {
        current_dir = DIR_DOWN;
        enemy->base.dir = current_dir;
    }

    /* Move in current direction (can pass through walls) */
    switch (current_dir) {
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
        new_y++;
        break;
    }

    /* Check if we can move to new position */
    if (map_is_valid_position(new_x, new_y)) {
        enemy->base.pos.x = new_x;
        enemy->base.pos.y = new_y;

        /* Return to normal mode if in a tunnel */
        if (map_is_walkable(new_x, new_y)) {
            enemy->state = ENEMY_NORMAL;
            enemy->ghost_timer = 0;
        }
    } else {
        /* Hit boundary - change direction */
        /* Try perpendicular directions */
        Direction new_dirs[4] = {DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT};
        for (int i = 0; i < 4; i++) {
            if (new_dirs[i] == current_dir) continue;

            int test_x = enemy->base.pos.x;
            int test_y = enemy->base.pos.y;
            switch (new_dirs[i]) {
            case DIR_UP:
                test_y--;
                break;
            case DIR_DOWN:
                test_y++;
                break;
            case DIR_LEFT:
                test_x--;
                break;
            case DIR_RIGHT:
                test_x++;
                break;
            default:
                break;
            }

            if (map_is_valid_position(test_x, test_y)) {
                enemy->base.dir = new_dirs[i];
                break;
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

        if (!rock->base.active || rock->state == ROCK_LANDED) {
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

    /* Check if hit ground */
    if (!map_is_valid_position(rock->base.pos.x, new_y) || map_is_solid(rock->base.pos.x, new_y)) {
        rock->state = ROCK_LANDED;
        rock->base.active = 0;
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
    int ry = rock->base.pos.y + 1; /* Position below */

    /* Check player */
    if (state->player.base.pos.x == rx && state->player.base.pos.y == ry) {
        logic_player_die(state);
        rock->has_crushed = 1;
        return 1;
    }

    /* Check enemies */
    for (int i = 0; i < state->enemy_count; i++) {
        Enemy *enemy = &state->enemies[i];
        if (enemy->base.active && enemy->state != ENEMY_DEAD && enemy->base.pos.x == rx &&
            enemy->base.pos.y == ry) {
            enemy->state = ENEMY_DEAD;
            enemy->base.active = 0;
            state->enemies_remaining--;

            /* Bonus points for rock kill */
            int points = logic_calculate_enemy_points(ry) * POINTS_ROCK_BONUS;
            logic_add_score(state, points);
            rock->has_crushed = 1;
            return 1;
        }
    }

    return 0;
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
        state->scene = SCENE_ROUND_CLEAR;
        state->round_start_timer = LEVEL_CLEAR_DELAY;
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

    /* Handle cooldown */
    if (fygar->fire_cooldown > 0) {
        fygar->fire_cooldown--;
        return;
    }

    /* Handle active fire */
    if (fygar->fire_active) {
        fygar->fire_duration--;
        if (fygar->fire_duration <= 0) {
            fygar->fire_active = 0;
            fygar->fire_cooldown = FYGAR_FIRE_COOLDOWN;
        }

        /* Check if fire hits player */
        if (logic_check_fire_collision(fygar, &state->player)) {
            logic_player_die(state);
        }
        return;
    }

    /* Decide whether to breathe fire (simple heuristic) */
    /* Fire if player is in horizontal line and within range */
    if (fygar->base.pos.y == state->player.base.pos.y) {
        int dx = state->player.base.pos.x - fygar->base.pos.x;
        if (logic_abs(dx) <= FYGAR_FIRE_RANGE) {
            /* Check direction */
            if ((dx > 0 && fygar->base.dir == DIR_RIGHT) ||
                (dx < 0 && fygar->base.dir == DIR_LEFT)) {
                fygar->fire_active = 1;
                fygar->fire_duration = FYGAR_FIRE_DURATION;
            }
        }
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
