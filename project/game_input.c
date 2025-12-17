/**
 * @file game_input.c
 * @brief Input system implementation for ZeOS Miner game
 */

#include <game_input.h>
#include <game_types.h>
#include <libc.h>
#include <stddef.h>

/* Global input state */
volatile InputState g_input;

/* ============================================================================
 *                            INITIALIZATION
 * ============================================================================ */

void input_init(void) {
    /* Clear all input state */
    input_reset();

    /* Register keyboard event handler */
    int result = KeyboardEvent(input_keyboard_handler);
    if (result < 0) {
        /* Handle error if KeyboardEvent fails */
    }
}

void input_cleanup(void) {
    /* Unregister keyboard event handler by passing NULL */
    KeyboardEvent(NULL);

    /* Clear all state */
    input_reset();
}

/* ============================================================================
 *                            INPUT PROCESSING
 * ============================================================================ */

void input_update(void) {
    /* In interrupt-driven mode with KeyboardEvent, this function does nothing */
    /* All input processing is handled by input_keyboard_handler */
}

void input_keyboard_handler(char key, int pressed) {
    /* Update last key pressed */
    if (pressed) {
        g_input.last_key = key;
        g_input.any_key_pressed = 1;
    }

    /* Update key held state (key is unsigned char so always < 256) */
    g_input.keys_held[(unsigned char)key] = pressed;

    /* Only process key press events, not releases */
    if (!pressed) {
        return;
    }

    /* Process movement keys */
    switch (key) {
    case KEY_W:
    case KEY_ARROW_UP:
        g_input.direction = DIR_UP;
        break;

    case KEY_S:
    case KEY_ARROW_DOWN:
        g_input.direction = DIR_DOWN;
        break;

    case KEY_A:
    case KEY_ARROW_LEFT:
        g_input.direction = DIR_LEFT;
        break;

    case KEY_D:
    case KEY_ARROW_RIGHT:
        g_input.direction = DIR_RIGHT;
        break;

    case KEY_SPACE:
    case KEY_ENTER:
        g_input.action_pressed = 1;
        break;

    case KEY_P:
    case KEY_ESC:
        g_input.pause_pressed = 1;
        break;

    case KEY_Q:
        g_input.quit_pressed = 1;
        break;
    }
}

/* ============================================================================
 *                            INPUT QUERIES
 * ============================================================================ */

Direction input_get_direction(void) {
    Direction dir = g_input.direction;
    g_input.direction = DIR_NONE; /* Consume the input */
    return dir;
}

int input_is_action_pressed(void) {
    int pressed = g_input.action_pressed;
    g_input.action_pressed = 0; /* Consume the input */
    return pressed;
}

int input_is_pause_pressed(void) {
    int pressed = g_input.pause_pressed;
    g_input.pause_pressed = 0; /* Consume the input */
    return pressed;
}

int input_is_quit_pressed(void) {
    return g_input.quit_pressed; /* Don't consume - let caller decide */
}

int input_any_key_pressed(void) {
    int pressed = g_input.any_key_pressed;
    g_input.any_key_pressed = 0; /* Consume the input */
    return pressed;
}

char input_get_last_key(void) {
    return g_input.last_key;
}

/* ============================================================================
 *                            STATE MANAGEMENT
 * ============================================================================ */

void input_clear(void) {
    g_input.direction = DIR_NONE;
    g_input.action_pressed = 0;
    g_input.pause_pressed = 0;
    g_input.any_key_pressed = 0;
    /* Don't clear quit_pressed or last_key */
}

void input_clear_quit(void) {
    g_input.quit_pressed = 0;
}

void input_reset(void) {
    g_input.direction = DIR_NONE;
    g_input.action_pressed = 0;
    g_input.pause_pressed = 0;
    g_input.quit_pressed = 0;
    g_input.last_key = 0;
    g_input.any_key_pressed = 0;

    /* Clear all key held states */
    for (int i = 0; i < 256; i++) {
        g_input.keys_held[i] = 0;
    }
}

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

int input_is_key_held(unsigned char scancode) {
    return g_input.keys_held[scancode];
}

char input_scancode_to_char(char scancode) {
    switch (scancode) {
    case KEY_W:
        return 'W';
    case KEY_A:
        return 'A';
    case KEY_S:
        return 'S';
    case KEY_D:
        return 'D';
    case KEY_P:
        return 'P';
    case KEY_Q:
        return 'Q';
    case KEY_ESC:
        return 27;
    case KEY_SPACE:
        return ' ';
    case KEY_ENTER:
        return '\n';
    case KEY_ARROW_UP:
        return '^';
    case KEY_ARROW_DOWN:
        return 'v';
    case KEY_ARROW_LEFT:
        return '<';
    case KEY_ARROW_RIGHT:
        return '>';
    default:
        return '?';
    }
}