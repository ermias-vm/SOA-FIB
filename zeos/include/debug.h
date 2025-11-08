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

// clang-format off
#define DEBUG_INFO_TASK_SWITCH          0
#define DEBUG_INFO_FORK                 0
#define DEBUG_INFO_EXIT                 0
#define DEBUG_INFO_BLOCK                0
#define DEBUG_INFO_UNBLOCK              0
// clang-format on

#endif /* __DEBUG_H__ */
