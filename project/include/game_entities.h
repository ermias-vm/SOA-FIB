#ifndef __GAME_ENTITIES_H__
#define __GAME_ENTITIES_H__

#include <game_types.h>

/**
 * @file game_entities.h
 * @brief Entity management system prototypes with complete documentation
 */

/* ============================================================================
 *                         GENERIC ENTITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize a generic entity.
 * @param e Pointer to entity structure
 * @param x Initial X position
 * @param y Initial Y position
 * @param type Entity type (ENTITY_PLAYER, ENTITY_POOKA, etc.)
 */
void entity_init(Entity *e, int x, int y, EntityType type);

/**
 * @brief Move entity in specified direction.
 * @param e Pointer to entity
 * @param dir Direction to move
 */
void entity_move(Entity *e, Direction dir);

/**
 * @brief Check if entity can move in specified direction.
 * @param e Pointer to entity
 * @param dir Direction to check
 * @return 1 if can move, 0 if blocked
 */
int entity_can_move(Entity *e, Direction dir);

/**
 * @brief Get next position if entity moves in direction.
 * @param e Pointer to entity
 * @param dir Direction to move
 * @return Position structure with next coordinates
 */
Position entity_next_pos(Entity *e, Direction dir);

/**
 * @brief Set entity position directly.
 * @param e Pointer to entity
 * @param x New X position
 * @param y New Y position
 */
void entity_set_position(Entity *e, int x, int y);

/* ============================================================================
 *                          PLAYER-SPECIFIC FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize player entity.
 * @param player Pointer to player entity
 * @param x Initial X position
 * @param y Initial Y position
 */
void player_init(Entity *player, int x, int y);

/**
 * @brief Update player based on input.
 * @param player Pointer to player entity
 * @param input_dir Direction from input
 */
void player_update(Entity *player, Direction input_dir);

/**
 * @brief Player collects gem at position.
 * @param player Pointer to player entity
 * @param x X position of gem
 * @param y Y position of gem
 */
void player_collect_gem(Entity *player, int x, int y);

/**
 * @brief Reset player to starting position.
 * @param player Pointer to player entity
 */
void player_reset_position(Entity *player);

/**
 * @brief Check if player can dig in direction.
 * @param player Pointer to player entity
 * @param dir Direction to dig
 * @return 1 if can dig, 0 otherwise
 */
int player_can_dig(Entity *player, Direction dir);

/**
 * @brief Dig in specified direction.
 * @param player Pointer to player entity
 * @param dir Direction to dig
 */
void player_dig(Entity *player, Direction dir);

/* ============================================================================
 *                          ENEMY-SPECIFIC FUNCTIONS
 * ============================================================================ */

/**
 * @brief Initialize enemy entity.
 * @param enemy Pointer to enemy entity
 * @param x Initial X position
 * @param y Initial Y position
 */
void enemy_init(Entity *enemy, int x, int y);

/**
 * @brief Update enemy AI and movement.
 * @param enemy Pointer to enemy entity
 * @param player_pos Player's current position
 */
void enemy_update(Entity *enemy, Position player_pos);

/**
 * @brief Get AI direction towards target.
 * @param enemy Pointer to enemy entity
 * @param target Target position to move towards
 * @return Direction to move (DIR_NONE if no valid move)
 */
Direction enemy_ai_direction(Entity *enemy, Position target);

/**
 * @brief Reset enemy to starting position.
 * @param enemy Pointer to enemy entity
 * @param enemy_index Index of enemy (for spawn position calculation)
 */
void enemy_reset_position(Entity *enemy, int enemy_index);

/**
 * @brief Initialize all enemies for a level.
 * @param enemies Array of enemy entities
 * @param count Number of enemies
 * @param level Current level (affects enemy behavior)
 */
void enemies_init_all(Entity enemies[], int count, int level);

/**
 * @brief Update all active enemies.
 * @param enemies Array of enemy entities
 * @param count Number of enemies
 * @param player_pos Player's current position
 */
void enemies_update_all(Entity enemies[], int count, Position player_pos);

/* ============================================================================
 *                           COLLISION DETECTION
 * ============================================================================ */

/**
 * @brief Check collision between two entities.
 * @param a First entity
 * @param b Second entity
 * @return 1 if collision detected, 0 otherwise
 */
int check_collision(Entity *a, Entity *b);

/**
 * @brief Check if player collides with any enemy.
 * @param player Pointer to player entity
 * @param enemies Array of enemy entities
 * @param count Number of enemies
 * @return 1 if collision detected, 0 otherwise
 */
int check_player_enemy_collision(Entity *player, Entity enemies[], int count);

/**
 * @brief Check collision between two positions.
 * @param pos_a First position
 * @param pos_b Second position
 * @return 1 if positions are same, 0 otherwise
 */
int check_position_collision(Position pos_a, Position pos_b);

/* ============================================================================
 *                            ENTITY UTILITIES
 * ============================================================================ */

/**
 * @brief Check if entity is active.
 * @param e Pointer to entity
 * @return 1 if active, 0 if inactive or NULL
 */
int entity_is_active(Entity *e);

/**
 * @brief Deactivate entity.
 * @param e Pointer to entity
 */
void entity_deactivate(Entity *e);

/**
 * @brief Activate entity.
 * @param e Pointer to entity
 */
void entity_activate(Entity *e);

/**
 * @brief Get direction from entity to target position.
 * @param from Pointer to source entity
 * @param to Target position
 * @return Direction to move towards target
 */
Direction entity_get_direction_to(Entity *from, Position to);

/**
 * @brief Calculate squared distance between entity and position.
 * @param from Pointer to source entity
 * @param to Target position
 * @return Squared distance (dx*dx + dy*dy)
 */
int entity_distance_to(Entity *from, Position to);

#endif /* __GAME_ENTITIES_H__ */
