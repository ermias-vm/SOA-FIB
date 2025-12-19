/**
 * @file game_config.h
 * @brief Game configuration constants for Dig Dug clone.
 *
 * This header defines all configurable constants for the game including
 * screen dimensions, layer definitions, scoring, speeds, and visual elements.
 */

#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include <times.h>

/* ============================================================================
 *                           SCREEN DIMENSIONS
 * ============================================================================ */

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT * 2) /* 4000 bytes */

/* ============================================================================
 *                           ROW DEFINITIONS
 * ============================================================================ */

/* Status bars (painted black) */
#define ROW_STATUS_TOP 0     /* Row 0: TIME + FPS */
#define ROW_STATUS_BOTTOM 24 /* Row 24: LIVES + SCORE + ROUND */

/* Sky area (3 rows) */
#define ROW_SKY_START 1                            /* Sky starts here */
#define ROW_SKY_END 3                              /* Sky ends here (rows 1, 2, 3) */
#define SKY_ROWS (ROW_SKY_END - ROW_SKY_START + 1) /* 3 rows */

/* Ground area (20 rows, divided in 4 layers of 5 rows each) */
#define ROW_GROUND_START 4                                  /* Ground starts here */
#define ROW_GROUND_END 23                                   /* Ground ends here */
#define GROUND_ROWS (ROW_GROUND_END - ROW_GROUND_START + 1) /* 20 rows */

/* ============================================================================
 *                           LAYER DEFINITIONS
 * ============================================================================ */

/* Each layer has 5 rows */
#define ROWS_PER_LAYER 5

/* Layer 1: rows 4-8 */
#define LAYER_1_START 4
#define LAYER_1_END 8

/* Layer 2: rows 9-13 */
#define LAYER_2_START 9
#define LAYER_2_END 13

/* Layer 3: rows 14-18 */
#define LAYER_3_START 14
#define LAYER_3_END 18

/* Layer 4: rows 19-23 */
#define LAYER_4_START 19
#define LAYER_4_END 23

/* ============================================================================
 *                           SCORING SYSTEM
 * ============================================================================ */

/* Base points per enemy depending on layer */
#define SCORE_LAYER_1 200
#define SCORE_LAYER_2 300
#define SCORE_LAYER_3 400
#define SCORE_LAYER_4 500

/* Fygar gives double points */
#define FYGAR_SCORE_MULT 2

/* ============================================================================
 *                           MAP DIMENSIONS
 * ============================================================================ */

#define MAP_WIDTH SCREEN_WIDTH                          /* 80 columns */
#define MAP_HEIGHT (ROW_GROUND_END - ROW_SKY_START + 1) /* 23 rows (sky + ground) */
#define MAP_OFFSET_Y ROW_SKY_START                      /* Maps starts at row 1 */

/* Playable ground area */
#define GROUND_WIDTH MAP_WIDTH
#define GROUND_HEIGHT GROUND_ROWS        /* 20 rows */
#define GROUND_OFFSET_Y ROW_GROUND_START /* Ground starts at row 4 */

/* ============================================================================
 *                           GAME LIMITS
 * ============================================================================ */

#define MAX_ENEMIES 8
#define INITIAL_LIVES 3
#define MAX_LIVES 5
#define MAX_ROUNDS 99
#define MAX_SCORE 99999
#define MAX_GEMS 20
#define MAX_ROCKS 10

/* ============================================================================
 *                           SPEEDS (ticks between movements)
 * ============================================================================ */

#define PLAYER_SPEED 2              /* Player moves every 2 ticks */
#define POOKA_BASE_SPEED 4          /* Pooka moves every 4 ticks */
#define FYGAR_BASE_SPEED 5          /* Fygar is slightly slower */
#define GHOST_SPEED 6               /* Ghost mode is slow */
#define ENEMY_BASE_SPEED 3          /* Base enemy speed */
#define SPEED_INCREMENT_PER_LEVEL 1 /* Speed increase per level */

/* Speed increases per round (enemies get faster) */
#define SPEED_DECREASE_PER_ROUND 1 /* Decrease tick delay = faster */
#define MIN_ENEMY_SPEED 2          /* Minimum ticks between moves */

/*============================================================================*
 *                    GAME-SPECIFIC TIMES                                     *
 *============================================================================*/

#define FYGAR_FIRE_RANGE 2                   /* Fire reaches 2 blocks horizontally */
#define FYGAR_FIRE_COOLDOWN (ONE_SECOND * 2) /* Ticks between fire attacks */
#define FYGAR_FIRE_DURATION HALF_SECOND      /* Ticks fire stays active */

#define INFLATE_LEVELS 4                /* Levels before enemy explodes */
#define INFLATE_DEFLATE_TIME ONE_SECOND /* Ticks to deflate one level */
#define PUMP_RANGE 1                    /* Pump reaches 1 block ahead */

#define ROUND_START_DELAY (ONE_SECOND * 2) /* Delay at round start */
#define DEATH_DELAY ONE_SECOND             /* Delay after death */

/* ============================================================================
 *                           GAME CHARACTERS (ASCII)
 * ============================================================================ */

#define CHAR_PLAYER '@' /* Dig Dug character */
#define CHAR_POOKA 'O'  /* Pooka enemy (round) */
#define CHAR_FYGAR 'F'  /* Fygar enemy (dragon) */
#define CHAR_ROCK 'R'   /* Rock obstacle */
#define CHAR_DIRT '#'   /* Solid dirt block */
#define CHAR_EMPTY ' '  /* Empty tunnel */
#define CHAR_FIRE '*'   /* Fygar fire */
#define CHAR_PUMP '-'   /* Pump attack */
#define CHAR_LIFE 'V'   /* Life symbol (V for vida/life) */
#define CHAR_SKY ' '    /* Sky (empty) */

/* Inflating enemy characters (stages) */
#define CHAR_INFLATE_1 'o' /* Slightly inflated */
#define CHAR_INFLATE_2 '0' /* More inflated */
#define CHAR_INFLATE_3 'O' /* Very inflated */
#define CHAR_INFLATE_4 '@' /* About to pop */

/* ============================================================================
 *                           VGA COLORS
 *                     Format: 0xBT (B=background, T=text)
 * ============================================================================ */

/* Status bar colors */
#define COLOR_STATUS_BG 0x00    /* Black background */
#define COLOR_STATUS_TEXT 0x0F  /* White text on black */
#define COLOR_STATUS_TIME 0x0B  /* Light Cyan for time (matches existing) */
#define COLOR_STATUS_FPS 0x0E   /* Yellow for FPS (matches existing) */
#define COLOR_STATUS_SCORE 0x0E /* Yellow for score */
#define COLOR_STATUS_ROUND 0x0B /* Cyan for round */
#define COLOR_STATUS_LIVES 0x04 /* Red for lives (hearts) */

/* ============================================================================
 *                      STATUS BAR POSITIONS (Row 24)
 * ============================================================================ */

/* Lives display: position 0-2, shows heart symbols */
#define UI_LIVES_X 0
#define UI_LIVES_Y ROW_STATUS_BOTTOM

/* Score display: position 31-43, "SCORE: XXXXX" (centered) */
#define UI_SCORE_X 31
#define UI_SCORE_Y ROW_STATUS_BOTTOM
#define UI_SCORE_LABEL "SCORE: "
#define UI_SCORE_DIGITS 5

/* Round display: position 67-77, "ROUND: XX" (right aligned) */
#define UI_ROUND_X 67
#define UI_ROUND_Y ROW_STATUS_BOTTOM
#define UI_ROUND_LABEL "ROUND: "
#define UI_ROUND_DIGITS 2

/* ============================================================================
 *                           SKY AND LAYER COLORS
 * ============================================================================ */

/* Sky colors */
#define COLOR_SKY 0x9F    /* White on light blue */
#define COLOR_SKY_BG 0x99 /* Light blue background */

/* Layer colors (background represents dirt color) */
#define COLOR_LAYER_1 0x6F /* White on brown (light soil) */
#define COLOR_LAYER_2 0x6E /* Yellow on brown */
#define COLOR_LAYER_3 0x4F /* White on red (deeper soil) */
#define COLOR_LAYER_4 0x40 /* Black on red (deepest) */

/* Tunnel color */
#define COLOR_TUNNEL 0x00 /* Black on black */

/* Entity colors */
#define COLOR_PLAYER 0x0A /* Bright green */
#define COLOR_POOKA 0x0C  /* Bright red */
#define COLOR_FYGAR 0x0E  /* Yellow */
#define COLOR_FIRE 0x04   /* Dark red for fire */

#endif /* __GAME_CONFIG_H__ */
