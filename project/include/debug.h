/**
 * @file debug.h
 * @brief Debug configuration flags for ZeOS kernel debugging.
 *
 * This header defines debug flags to enable/disable debug output
 * for different subsystems of the ZeOS kernel. Enable specific
 * debug options by setting them to 1, disable by setting to 0.
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

/* ============================================================================
 *                        KERNEL DEBUG FLAGS
 * ============================================================================ */
// clang-format off
#define DEBUG_INFO_TASK_SWITCH      0
#define DEBUG_INFO_FORK             0
#define DEBUG_INFO_EXIT             0
#define DEBUG_INFO_BLOCK            0
#define DEBUG_INFO_UNBLOCK          0
#define DEBUG_INFO_THREAD_CREATE    0
#define DEBUG_INFO_THREAD_EXIT      0
// clang-format on

/* ============================================================================
 *                         GAME DEBUG FLAGS
 * ============================================================================ */
// clang-format off
#define DEBUG_GAME_ENABLED          1   /** Master switch: Enable/disable ALL game debug output */

#define DEBUG_GAME_INPUT            1   /** Debug player input (direction, action pressed) */
#define DEBUG_GAME_PLAYER           1   /** Debug player state and position */
#define DEBUG_GAME_ENEMIES          1   /** Debug enemy AI and movement */
#define DEBUG_GAME_COLLISION        1   /** Debug collision detection */
#define DEBUG_GAME_STATE            1   /** Debug game state/scene transitions */
#define DEBUG_GAME_RENDER           1   /** Debug rendering (frame timing, buffer swaps) */
#define DEBUG_GAME_MAP              1   /** Debug map generation and tiles */
// clang-format on

#endif /* __DEBUG_H__ */
