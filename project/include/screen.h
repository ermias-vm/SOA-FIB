/**
 * @file screen.h
 * @brief Screen frame buffer support for ZeOS.
 *
 * This header provides direct screen buffer writing capability
 * through file descriptor 10, enabling efficient full-screen updates
 * for animations, games, and graphical text interfaces.
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <io.h>

#define SCREEN_METHOD_WORD_LOOP 0xA      /**< Word-by-word C loop (slowest) */
#define SCREEN_METHOD_COPY_FROM_USER 0xB /**< copy_from_user() (medium) */
#define SCREEN_METHOD_REP_MOVSL 0xC      /**< REP MOVSL assembly (fastest) */

/** Selected copy method for screen buffer to video memory */
#define SCREEN_COPY_METHOD SCREEN_METHOD_REP_MOVSL

/** Global frame counter - incremented each time a full screen is written */
extern int frame_count;

/**
 * @brief Write frame buffer directly to screen memory.
 *
 * This function copies a user-space frame buffer directly to VGA video
 * memory at 0xb8000. Unlike sys_write_console(), this function:
 *   - Does NOT update cursor position (x, y)
 *   - Does NOT handle newlines or special characters
 *   - Does NOT perform scrolling
 *   - Writes raw character+color pairs directly
 *
 * The buffer format is 80×25 characters, 2 bytes per character:
 *   - Byte 0: ASCII character code
 *   - Byte 1: Color attribute (bits 7-4: background, bits 3-0: foreground)
 *
 * @param buffer Pointer to user-space frame buffer.
 * @param size Size of buffer in bytes (max 4000 bytes).
 * @return Number of bytes written, or negative error code.
 */
int sys_write_screen(char *buffer, int size);

#endif /* __SCREEN_H__ */
