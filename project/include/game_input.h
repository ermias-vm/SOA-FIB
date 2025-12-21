#ifndef __GAME_INPUT_H__
#define __GAME_INPUT_H__

#include <game_types.h>

/**
 * @file game_input.h
 * @brief Input system for ZeOS Miner game with complete documentation
 */

/* ============================================================================
 *                              KEY CONSTANTS
 * ============================================================================ */

/* Keyboard scancodes for movement keys */
#define KEY_W 0x11 /* W key scancode */
#define KEY_A 0x1E /* A key scancode */
#define KEY_S 0x1F /* S key scancode */
#define KEY_D 0x20 /* D key scancode */

/* Control keys */
#define KEY_ESC 0x01   /* Escape key */
#define KEY_Q 0x10     /* Q key */
#define KEY_ENTER 0x1C /* Enter key */
#define KEY_SPACE 0x39 /* Spacebar */
#define KEY_P 0x19     /* P key */
#define KEY_K 0x25     /* K key (developer: kill enemy) */

/* Arrow keys (extended scancodes) */
#define KEY_ARROW_UP 0x48    /* Up arrow */
#define KEY_ARROW_DOWN 0x50  /* Down arrow */
#define KEY_ARROW_LEFT 0x4B  /* Left arrow */
#define KEY_ARROW_RIGHT 0x4D /* Right arrow */

/* ============================================================================
 *                            INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize the input system.
 *
 * Registers keyboard event handler and initializes input state.
 * Must be called before using any other input functions.
 */
void input_init(void);

/**
 * @brief Cleanup the input system.
 *
 * Unregisters keyboard event handler and cleans up resources.
 */
void input_cleanup(void);

/* ============================================================================
 *                            INPUT PROCESSING
 * ============================================================================ */

/**
 * @brief Update input state (if using polling mode).
 *
 * In interrupt-driven mode (KeyboardEvent), this function does nothing
 * as input is processed automatically by the keyboard handler.
 * In polling mode, this would read from keyboard device.
 */
void input_update(void);

/**
 * @brief Internal keyboard event handler function.
 * @param key Scancode of the key
 * @param pressed 1 if key was pressed, 0 if released
 *
 * This function is called automatically by the keyboard interrupt system.
 * Do not call this function directly.
 */
void input_keyboard_handler(char key, int pressed);

/* ============================================================================
 *                            INPUT QUERIES
 * ============================================================================ */

/**
 * @brief Get current movement direction and clear it.
 * @return Direction enum value (DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE)
 *
 * This function "consumes" the direction input, meaning subsequent calls
 * will return DIR_NONE until a new direction key is pressed.
 */
Direction input_get_direction(void);

/**
 * @brief Check if action button was pressed and clear the flag.
 * @return 1 if action button (enter) was pressed, 0 otherwise
 *
 * This function "consumes" the action input.
 */
int input_is_action_pressed(void);

/**
 * @brief Check if attack button was pressed and clear the flag.
 * @return 1 if attack button (space) was pressed, 0 otherwise
 *
 * This function "consumes" the attack input.
 */
int input_is_attack_pressed(void);

/**
 * @brief Check if attack button is currently being held.
 * @return 1 if attack button (space) is held, 0 otherwise
 *
 * This function does NOT consume the input - returns held state.
 */
int input_is_attack_held(void);

/**
 * @brief Check if pause button was pressed and clear the flag.
 * @return 1 if pause button (P or ESC) was pressed, 0 otherwise
 *
 * This function "consumes" the pause input.
 */
int input_is_pause_pressed(void);

/**
 * @brief Check if quit button was pressed.
 * @return 1 if quit button (Q) was pressed, 0 otherwise
 *
 * This function does NOT consume the quit input - it persists until cleared.
 */
int input_is_quit_pressed(void);

/**
 * @brief Check if developer kill button was pressed and clear the flag.
 * @return 1 if K key was pressed, 0 otherwise
 *
 * Developer feature: kills one enemy per press during gameplay.
 */
int input_is_dev_kill_pressed(void);

/**
 * @brief Check if any key was pressed and clear the flag.
 * @return 1 if any key was pressed, 0 otherwise
 *
 * Useful for menu navigation and "press any key to continue" screens.
 */
int input_any_key_pressed(void);

/**
 * @brief Get the last raw key pressed.
 * @return Scancode of last key pressed
 *
 * Useful for debugging input issues.
 */
char input_get_last_key(void);

/* ============================================================================
 *                            STATE MANAGEMENT
 * ============================================================================ */

/**
 * @brief Clear all input state flags.
 *
 * Clears direction, action, pause, and any_key flags.
 * Does NOT clear quit flag (use input_clear_quit for that).
 */
void input_clear(void);

/**
 * @brief Clear the quit flag.
 *
 * Use this when you've processed the quit request.
 */
void input_clear_quit(void);

/**
 * @brief Reset input system to initial state.
 *
 * Clears all flags and resets all state to initial values.
 */
void input_reset(void);

/**
 * @brief Reset the move_processed flag for a new frame.
 *
 * Call this at the start of each game logic frame to allow
 * a new movement to be processed.
 */
void input_new_frame(void);

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Check if a specific key is currently held down.
 * @param scancode Keyboard scancode to check
 * @return 1 if key is held, 0 otherwise
 */
int input_is_key_held(unsigned char scancode);

/**
 * @brief Convert scancode to printable character (for debugging).
 * @param scancode Keyboard scancode
 * @return Printable character or '?' if not printable
 */
char input_scancode_to_char(char scancode);

#endif /* __GAME_INPUT_H__ */
