#ifndef __GAME_ENTITIES_H__
#define __GAME_ENTITIES_H__

#include <game_types.h>

/**
 * @file game_entities.h
 * @brief Entity management system prototypes and functions
 */

/* Generic entity functions */
void entity_init(Entity *e, int x, int y, EntityType type);
void entity_move(Entity *e, Direction dir);
int entity_can_move(Entity *e, Direction dir);
Position entity_next_pos(Entity *e, Direction dir);
void entity_set_position(Entity *e, int x, int y);

/* Player-specific functions */
void player_init(Entity *player, int x, int y);
void player_update(Entity *player, Direction input_dir);
void player_collect_gem(Entity *player, int x, int y);
void player_reset_position(Entity *player);
int player_can_dig(Entity *player, Direction dir);
void player_dig(Entity *player, Direction dir);

/* Enemy-specific functions */
void enemy_init(Entity *enemy, int x, int y);
void enemy_update(Entity *enemy, Position player_pos);
Direction enemy_ai_direction(Entity *enemy, Position target);
void enemy_reset_position(Entity *enemy, int enemy_index);
void enemies_init_all(Entity enemies[], int count, int level);
void enemies_update_all(Entity enemies[], int count, Position player_pos);

/* Collision detection */
int check_collision(Entity *a, Entity *b);
int check_player_enemy_collision(Entity *player, Entity enemies[], int count);
int check_position_collision(Position pos_a, Position pos_b);

/* Entity utilities */
int entity_is_active(Entity *e);
void entity_deactivate(Entity *e);
void entity_activate(Entity *e);
Direction entity_get_direction_to(Entity *from, Position to);
int entity_distance_to(Entity *from, Position to);

#endif /* __GAME_ENTITIES_H__ */