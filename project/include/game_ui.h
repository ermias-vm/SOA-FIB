#ifndef __GAME_UI_H__
#define __GAME_UI_H__

#include <game_types.h>
#include <game_render.h>

/**
 * @file game_ui.h
 * @brief User Interface and HUD system for ZeOS Miner game
 */

/* ============================================================================
 *                            UI CONSTANTS
 * ============================================================================ */

/* Character constants for UI elements */
#define CHAR_HEART          0x03    /* ♥ for lives display */
#define CHAR_BORDER_H       '-'     /* Horizontal border */
#define CHAR_BORDER_V       '|'     /* Vertical border */
#define CHAR_CORNER         '+'     /* Corner character */

/* UI Layout constants */
#define HUD_TIME_X          0       /* Time position (left) */
#define HUD_FPS_X           72      /* FPS position (right) */
#define HUD_LIVES_X         0       /* Lives position (left) */
#define HUD_SCORE_X         34      /* Score position (center) */
#define HUD_ROUND_X         72      /* Round position (right) */

/* Message box dimensions */
#define MSG_BOX_WIDTH       40
#define MSG_BOX_HEIGHT      8
#define MSG_BOX_X           ((SCREEN_WIDTH - MSG_BOX_WIDTH) / 2)
#define MSG_BOX_Y           8

/* ============================================================================
 *                            MAIN HUD FUNCTIONS
 * ============================================================================ */

/**
 * @brief Draw the complete HUD (top and bottom bars).
 * @param lives Current player lives
 * @param score Current game score
 * @param round Current round number
 * @param time_seconds Elapsed time in seconds
 * @param fps Current frames per second
 */
void ui_draw_hud(int lives, int score, int round, int time_seconds, int fps);

/**
 * @brief Draw the top status bar (time and FPS).
 * @param time_seconds Game time in seconds
 * @param fps Current FPS
 */
void ui_draw_top_bar(int time_seconds, int fps);

/**
 * @brief Draw the bottom status bar (lives, score, round).
 * @param lives Number of lives remaining
 * @param score Current score
 * @param round Current round number
 */
void ui_draw_bottom_bar(int lives, int score, int round);

/* ============================================================================
 *                         INDIVIDUAL HUD ELEMENTS
 * ============================================================================ */

/**
 * @brief Draw game time in MM:SS format.
 * @param seconds Total seconds elapsed
 */
void ui_draw_time(int seconds);

/**
 * @brief Draw FPS counter.
 * @param fps Frames per second to display
 */
void ui_draw_fps(int fps);

/**
 * @brief Draw player lives as hearts.
 * @param lives Number of lives (0-5)
 */
void ui_draw_lives(int lives);

/**
 * @brief Draw current score.
 * @param score Score to display (0-99999)
 */
void ui_draw_score(int score);

/**
 * @brief Draw current round number.
 * @param round Round number to display
 */
void ui_draw_round(int round);

/* ============================================================================
 *                            SCREEN OVERLAYS
 * ============================================================================ */

/**
 * @brief Draw the main menu screen.
 */
void ui_draw_menu_screen(void);

/**
 * @brief Draw pause overlay (doesn't clear game).
 */
void ui_draw_pause_screen(void);

/**
 * @brief Draw game over screen with final score.
 * @param final_score Final score achieved
 */
void ui_draw_game_over_screen(int final_score);

/**
 * @brief Draw level completion overlay.
 * @param round Round that was completed
 * @param score Score earned in this round
 */
void ui_draw_level_clear_screen(int round, int score);

/**
 * @brief Draw victory screen (all levels completed).
 * @param final_score Final total score
 */
void ui_draw_victory_screen(int final_score);

/* ============================================================================
 *                            TEXT UTILITIES
 * ============================================================================ */

/**
 * @brief Draw text centered horizontally on specified row.
 * @param y Row position
 * @param text Text to draw
 * @param color Color for the text
 */
void ui_draw_centered_text(int y, const char* text, Color color);

/**
 * @brief Draw a message box with title and content.
 * @param title Box title
 * @param message Main message text
 */
void ui_draw_message_box(const char* title, const char* message);

/**
 * @brief Draw a simple rectangular border.
 * @param x Left coordinate
 * @param y Top coordinate  
 * @param w Width
 * @param h Height
 * @param color Border color
 */
void ui_draw_border(int x, int y, int w, int h, Color color);

/* ============================================================================
 *                           UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Calculate string length (simple strlen implementation).
 * @param str String to measure
 * @return Length of string
 */
int ui_strlen(const char* str);

/**
 * @brief Convert integer to string with padding.
 * @param number Number to convert
 * @param buffer Output buffer (must be large enough)
 * @param digits Number of digits to display
 * @param pad_char Character to use for padding
 */
void ui_number_to_string(int number, char* buffer, int digits, char pad_char);

/**
 * @brief Flash effect for score (blink when points gained).
 * @param duration Number of frames to flash
 */
void ui_flash_score(int duration);

/**
 * @brief Animation when life is lost.
 */
void ui_animate_life_lost(void);

/**
 * @brief Clear only the game area (preserve HUD).
 */
void ui_clear_game_area(void);

#endif /* __GAME_UI_H__ */