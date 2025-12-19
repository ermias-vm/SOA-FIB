/**
 * @file times.h
 * @brief Centralized time constants and configuration for ZeOS.
 *
 * This header provides a unified system for time management throughout ZeOS.
 * All time-related constants are defined here to facilitate easy adjustment
 * of timing behavior across the entire system.
 *
 * The TIME_ADJUSTMENT factor compensates for differences between expected
 * and actual timer behavior in the emulation environment.
 */

#ifndef __TIMES_H__
#define __TIMES_H__

/*============================================================================*
 *                    BASE TIME CONFIGURATION                                 *
 *============================================================================*/

/**
 * @brief Time adjustment factor for emulation environment.
 *
 * This factor compensates for the difference between expected timer frequency
 * and actual timer behavior. Adjust this value based on calibration tests.
 *
 * - 1.0 = No adjustment (theoretical 1000 Hz timer)
 * - 0.8 = Timer runs 20% faster than expected
 * - 1.2 = Timer runs 20% slower than expected
 */
#define TIME_ADJUSTMENT 0.32

/**
 * @brief Base ticks per second (theoretical value before adjustment).
 *
 * The PIT is configured to generate approximately 1000 interrupts per second.
 * This is the base value used for all time calculations.
 */
#define BASE_TICKS_PER_SECOND 1000

/**
 * @brief Effective ticks per second after adjustment.
 *
 * This is the actual number of ticks that represent one real second,
 * taking into account the TIME_ADJUSTMENT factor.
 */
#define TICKS_PER_SECOND ((int)(BASE_TICKS_PER_SECOND * TIME_ADJUSTMENT))

/**
 * @brief One second in ticks (adjusted).
 *
 * Use this constant when you need to wait or measure one second.
 */
#define ONE_SECOND TICKS_PER_SECOND

/**
 * @brief Half second in ticks (adjusted).
 */
#define HALF_SECOND (TICKS_PER_SECOND / 2)

/**
 * @brief Quarter second in ticks (adjusted).
 */
#define QUARTER_SECOND (TICKS_PER_SECOND / 4)

/*============================================================================*
 *                    MILLISECOND CONVERSIONS                                 *
 *============================================================================*/

/**
 * @brief Convert milliseconds to ticks.
 * @param ms Milliseconds to convert.
 * @return Equivalent ticks after adjustment.
 */
#define MS_TO_TICKS(ms) ((int)((ms) * TIME_ADJUSTMENT))

/**
 * @brief Convert seconds to ticks.
 * @param s Seconds to convert.
 * @return Equivalent ticks after adjustment.
 */
#define SECONDS_TO_TICKS(s) ((s) * TICKS_PER_SECOND)

/*============================================================================*
 *                    STANDARD TIME INTERVALS                                 *
 *============================================================================*/

/** @brief Minimal delay */
#define TIME_MINIMAL MS_TO_TICKS(50)

/** @brief Short delay */
#define TIME_SHORT MS_TO_TICKS(100)

/** @brief Default delay */
#define TIME_DEFAULT MS_TO_TICKS(200)

/** @brief Medium delay */
#define TIME_MEDIUM MS_TO_TICKS(300)

/** @brief Long delay */
#define TIME_LONG MS_TO_TICKS(500)

/** @brief Visual display time */
#define TIME_VISUAL_DISPLAY MS_TO_TICKS(700)

/** @brief Visual pause time */
#define TIME_VISUAL_PAUSE MS_TO_TICKS(400)

/*============================================================================*
 *                    TEST-SPECIFIC TIMES                                     *
 *============================================================================*/

/** @brief Keyboard test wait time for first key event test (5 seconds) */
#define TIME_KBD_WAIT_FIRST SECONDS_TO_TICKS(5)

/** @brief Keyboard test wait time for subsequent tests (3 seconds) */
#define TIME_KBD_WAIT SECONDS_TO_TICKS(3)

/** @brief Keyboard test pause time */
#define TIME_KBD_PAUSE ONE_SECOND

/** @brief FPS test duration */
#define TIME_FPS_TEST_DURATION SECONDS_TO_TICKS(60)

/** @brief Tick calibration test duration in seconds */
#define TIME_CALIBRATION_SECONDS 10

/** @brief Tick calibration expected ticks */
#define TIME_CALIBRATION_TICKS SECONDS_TO_TICKS(TIME_CALIBRATION_SECONDS)

/** @brief Countdown before calibration starts */
#define TIME_CALIBRATION_COUNTDOWN SECONDS_TO_TICKS(3)

/*============================================================================*
 *                    FPS DISPLAY CONFIGURATION                               *
 *============================================================================*/

/**
 * @brief FPS display update interval.
 *
 * FPS is calculated as: frames_written_since_last_update
 * We use BASE_TICKS_PER_SECOND (1000) to get real FPS regardless of
 * TIME_ADJUSTMENT. This ensures the displayed FPS matches the actual
 * frames rendered per real second.
 */
#define FPS_UPDATE_INTERVAL BASE_TICKS_PER_SECOND

/*============================================================================*
 *                    GAME FPS LIMITING                                       *
 *============================================================================*/

/**
 * @brief Target frames per second for the game.
 */
#define TARGET_FPS 60

/**
 * @brief Ticks per frame at target FPS.
 *
 * This is the minimum number of ticks that should pass between frames.
 * We use BASE_TICKS_PER_SECOND (1000) to get real 60 FPS timing regardless
 * of TIME_ADJUSTMENT. At 1000 Hz with TARGET_FPS=60, we wait ~16-17 ticks.
 */
#define TICKS_PER_FRAME (BASE_TICKS_PER_SECOND / TARGET_FPS)

/**
 * @brief Minimum ticks per frame (fallback if TICKS_PER_FRAME is 0).
 */
#define MIN_TICKS_PER_FRAME 1

/*============================================================================*
 *                    GAME ATTACK/PARALYSIS TIMES                             *
 *============================================================================*/

/**
 * @brief Time an enemy stays paralyzed before dying (1 second).
 */
#define ENEMY_PARALYSIS_TIME SECONDS_TO_TICKS(1)

/**
 * @brief Attack display duration in frames.
 */
#define ATTACK_DISPLAY_FRAMES 10

/**
 * @brief Delay before transitioning to next round (1 second at 60 FPS).
 * NOTE: This decrements once per frame, not per tick!
 */
#define ROUND_CLEAR_DELAY TARGET_FPS /* 60 frames = 1 second */

/*============================================================================*
 *                    SYSTEM TIMES                                            *
 *============================================================================*/

/** @brief Boot splash display time */
#define TIME_BOOT_SPLASH SECONDS_TO_TICKS(3)

#endif /* __TIMES_H__ */
