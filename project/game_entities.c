/**
 * @file game_entities.c
 * @brief Entity system implementation for ZeOS Miner game
 */

#include <game_config.h>
#include <game_entities.h>
#include <game_map.h>
#include <game_types.h>
#include <libc.h>

int calculate_distance_squared(Position a, Position b);
Direction get_best_direction_to_target(Position current, Position target);
int is_direction_blocked(Position pos, Direction dir);

void entity_init(Entity *e, int x, int y, EntityType type) {
    if (!e) return;

    e->pos.x = x;
    e->pos.y = y;
    e->dir = DIR_NONE;
    e->type = type;
    e->active = 1;
    e->speed_counter = 0;

    /* Set default speed based on entity type */
    switch (type) {
    case ENTITY_PLAYER:
        e->speed_limit = PLAYER_SPEED;
        break;
    case ENTITY_ENEMY:
    case ENTITY_POOKA:
        e->speed_limit = POOKA_BASE_SPEED;
        break;
    case ENTITY_FYGAR:
        e->speed_limit = FYGAR_BASE_SPEED;
        break;
    default:
        e->speed_limit = 1;
        break;
    }
}

void entity_move(Entity *e, Direction dir) {
    if (!e || !e->active) return;

    Position next_pos = entity_next_pos(e, dir);

    if (entity_can_move(e, dir)) {
        e->pos = next_pos;
        e->dir = dir;
    }
}

int entity_can_move(Entity *e, Direction dir) {
    if (!e || !e->active) return 0;

    Position next_pos = entity_next_pos(e, dir);

    /* Check map boundaries and walkability */
    if (!map_is_valid_position(next_pos.x, next_pos.y)) {
        return 0;
    }

    /* For players, they can also move into diggable tiles */
    if (e->type == ENTITY_PLAYER) {
        return (map_is_walkable(next_pos.x, next_pos.y) || map_is_diggable(next_pos.x, next_pos.y));
    }

    /* For enemies, only walkable tiles */
    return map_is_walkable(next_pos.x, next_pos.y);
}

Position entity_next_pos(Entity *e, Direction dir) {
    Position next_pos = e->pos;

    switch (dir) {
    case DIR_UP:
        next_pos.y--;
        break;
    case DIR_DOWN:
        next_pos.y++;
        break;
    case DIR_LEFT:
        next_pos.x--;
        break;
    case DIR_RIGHT:
        next_pos.x++;
        break;
    case DIR_NONE:
    default:
        /* No movement */
        break;
    }

    return next_pos;
}

void entity_set_position(Entity *e, int x, int y) {
    if (!e) return;

    e->pos.x = x;
    e->pos.y = y;
}

void player_init(Entity *player, int x, int y) {
    entity_init(player, x, y, ENTITY_PLAYER);
    player->speed_limit = PLAYER_SPEED;
}

void player_update(Entity *player, Direction input_dir) {
    if (!player || !player->active) return;

    /* Player moves immediately on input */
    if (input_dir != DIR_NONE) {
        /* Check if player is trying to dig */
        if (player_can_dig(player, input_dir)) {
            player_dig(player, input_dir);
        }

        /* Try to move */
        entity_move(player, input_dir);

        /* Check for gem collection */
        if (map_has_gem(player->pos.x, player->pos.y)) {
            player_collect_gem(player, player->pos.x, player->pos.y);
        }
    }
}

void player_collect_gem(Entity *player, int x, int y) {
    if (!player || !player->active) return;

    if (map_has_gem(x, y)) {
        map_remove_gem(x, y);
        /* Score increment would be handled by game logic */
    }
}

void player_reset_position(Entity *player) {
    if (!player) return;

    player->pos.x = 1;
    player->pos.y = 1;
    player->dir = DIR_NONE;
    player->active = 1;
}

int player_can_dig(Entity *player, Direction dir) {
    if (!player || !player->active) return 0;

    Position next_pos = entity_next_pos(player, dir);
    return map_is_diggable(next_pos.x, next_pos.y);
}

void player_dig(Entity *player, Direction dir) {
    if (!player || !player->active) return;

    Position dig_pos = entity_next_pos(player, dir);

    if (map_is_diggable(dig_pos.x, dig_pos.y)) {
        map_dig(dig_pos.x, dig_pos.y);
    }
}

void enemy_init(Entity *enemy, int x, int y) {
    entity_init(enemy, x, y, ENTITY_ENEMY);
    enemy->speed_limit = POOKA_BASE_SPEED; /* Use POOKA speed by default */
}

void enemy_update(Entity *enemy, Position player_pos) {
    if (!enemy || !enemy->active) return;

    /* Speed control - enemy moves every N ticks */
    enemy->speed_counter++;
    if (enemy->speed_counter < enemy->speed_limit) {
        return; /* Not time to move yet */
    }
    enemy->speed_counter = 0;

    /* Get direction towards player */
    Direction target_dir = enemy_ai_direction(enemy, player_pos);

    /* Try to move in that direction */
    if (target_dir != DIR_NONE && entity_can_move(enemy, target_dir)) {
        entity_move(enemy, target_dir);
    } else {
        /* If blocked, try alternative directions */
        Direction alternatives[] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
        for (int i = 0; i < 4; i++) {
            if (alternatives[i] != target_dir && entity_can_move(enemy, alternatives[i])) {
                entity_move(enemy, alternatives[i]);
                break;
            }
        }
    }
}

Direction enemy_ai_direction(Entity *enemy, Position target) {
    if (!enemy || !enemy->active) return DIR_NONE;

    int dx = target.x - enemy->pos.x;
    int dy = target.y - enemy->pos.y;

    /* Choose direction based on largest distance component */
    if (dx == 0 && dy == 0) {
        return DIR_NONE; /* Already at target */
    }

    /* Prioritize horizontal or vertical movement */
    if (abs(dx) > abs(dy)) {
        return (dx > 0) ? DIR_RIGHT : DIR_LEFT;
    } else {
        return (dy > 0) ? DIR_DOWN : DIR_UP;
    }
}

void enemy_reset_position(Entity *enemy, int enemy_index) {
    if (!enemy) return;

    Position spawn_pos;

    /* Try to get a safe spawn position */
    if (map_get_safe_spawn_position(&spawn_pos, 5)) {
        enemy->pos = spawn_pos;
    } else {
        /* Fallback positions based on enemy index */
        switch (enemy_index % 4) {
        case 0:
            enemy->pos.x = MAP_WIDTH - 3;
            enemy->pos.y = 3;
            break;
        case 1:
            enemy->pos.x = MAP_WIDTH - 3;
            enemy->pos.y = MAP_HEIGHT - 3;
            break;
        case 2:
            enemy->pos.x = 3;
            enemy->pos.y = MAP_HEIGHT - 3;
            break;
        case 3:
            enemy->pos.x = MAP_WIDTH / 2;
            enemy->pos.y = MAP_HEIGHT / 2;
            break;
        }
    }

    enemy->active = 1;
    enemy->dir = DIR_NONE;
    enemy->speed_counter = 0;
}

void enemies_init_all(Entity enemies[], int count) {
    if (!enemies) return;

    for (int i = 0; i < count; i++) {
        enemy_init(&enemies[i], 0, 0); /* Position will be set by reset */
        enemy_reset_position(&enemies[i], i);
    }
}

void enemies_update_all(Entity enemies[], int count, Position player_pos) {
    if (!enemies) return;

    for (int i = 0; i < count; i++) {
        if (enemies[i].active) {
            enemy_update(&enemies[i], player_pos);
        }
    }
}

int check_collision(Entity *a, Entity *b) {
    if (!a || !b || !a->active || !b->active) {
        return 0;
    }

    return (a->pos.x == b->pos.x && a->pos.y == b->pos.y);
}

int check_player_enemy_collision(Entity *player, Entity enemies[], int count) {
    if (!player || !enemies || !player->active) {
        return 0;
    }

    for (int i = 0; i < count; i++) {
        if (enemies[i].active && check_collision(player, &enemies[i])) {
            return 1; /* Collision detected */
        }
    }

    return 0; /* No collision */
}

int check_position_collision(Position pos_a, Position pos_b) {
    return (pos_a.x == pos_b.x && pos_a.y == pos_b.y);
}

int entity_is_active(Entity *e) {
    return (e && e->active);
}

void entity_deactivate(Entity *e) {
    if (e) {
        e->active = 0;
    }
}

void entity_activate(Entity *e) {
    if (e) {
        e->active = 1;
    }
}

Direction entity_get_direction_to(Entity *from, Position to) {
    if (!from) return DIR_NONE;

    return enemy_ai_direction(from, to);
}

int entity_distance_to(Entity *from, Position to) {
    if (!from) return 9999;

    return calculate_distance_squared(from->pos, to);
}

int calculate_distance_squared(Position a, Position b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return (dx * dx + dy * dy);
}

Direction get_best_direction_to_target(Position current, Position target) {
    int dx = target.x - current.x;
    int dy = target.y - current.y;

    if (abs(dx) > abs(dy)) {
        return (dx > 0) ? DIR_RIGHT : DIR_LEFT;
    } else if (dy != 0) {
        return (dy > 0) ? DIR_DOWN : DIR_UP;
    } else {
        return DIR_NONE;
    }
}

int is_direction_blocked(Position pos, Direction dir) {
    Position next_pos = pos;

    switch (dir) {
    case DIR_UP:
        next_pos.y--;
        break;
    case DIR_DOWN:
        next_pos.y++;
        break;
    case DIR_LEFT:
        next_pos.x--;
        break;
    case DIR_RIGHT:
        next_pos.x++;
        break;
    default:
        return 1;
    }

    return !map_is_walkable(next_pos.x, next_pos.y);
}
