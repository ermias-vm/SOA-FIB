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
    /* Movement is now one-shot per key press, not continuous */
    /* Direction is set directly in keyboard_handler and consumed by game logic */
}

void input_keyboard_handler(char key, int pressed) {
    /* Update last key pressed */
    if (pressed) {
        g_input.last_key = key;
        g_input.any_key_pressed = 1;
    }

    /* Determine if this is a movement key */
    Direction key_dir = DIR_NONE;
    switch (key) {
    case KEY_W:
    case KEY_ARROW_UP:
        key_dir = DIR_UP;
        break;
    case KEY_S:
    case KEY_ARROW_DOWN:
        key_dir = DIR_DOWN;
        break;
    case KEY_A:
    case KEY_ARROW_LEFT:
        key_dir = DIR_LEFT;
        break;
    case KEY_D:
    case KEY_ARROW_RIGHT:
        key_dir = DIR_RIGHT;
        break;
    default:
        break;
    }

    /* Handle movement key press/release */
    if (key_dir != DIR_NONE) {
        if (pressed) {
            /* Key pressed: update both held direction and immediate direction */
            g_input.held_dir = key_dir;
            g_input.direction = key_dir;
            g_input.move_processed = 0; /* Allow movement on this press */
        } else {
            /* Key released: clear held direction if it matches */
            if (g_input.held_dir == key_dir) {
                g_input.held_dir = DIR_NONE;
            }
        }
        return;
    }

    /* Handle space key (attack) specially - it's held, not just pressed */
    if (key == KEY_SPACE) {
        if (pressed) {
            g_input.attack_held = 1;
            g_input.attack_pressed = 1;
            g_input.action_pressed = 1; /* Also trigger action for menus */
        } else {
            g_input.attack_held = 0;
        }
        return;
    }

    /* Handle non-movement keys (only on press) */
    if (!pressed) {
        return;
    }

    switch (key) {
    case KEY_ENTER:
        g_input.action_pressed = 1;
        break;

    case KEY_P:
        g_input.pause_pressed = 1;
        break;

    case KEY_ESC:
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
    g_input.move_processed = 1;   /* Mark that we processed a move this frame */
    return dir;
}

int input_is_action_pressed(void) {
    int pressed = g_input.action_pressed;
    g_input.action_pressed = 0; /* Consume the input */
    return pressed;
}

int input_is_attack_pressed(void) {
    int pressed = g_input.attack_pressed;
    g_input.attack_pressed = 0; /* Consume the input */
    return pressed;
}

int input_is_attack_held(void) {
    return g_input.attack_held; /* Don't consume - held state */
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
    g_input.held_dir = DIR_NONE;
    g_input.action_pressed = 0;
    g_input.attack_pressed = 0;
    g_input.attack_held = 0;
    g_input.pause_pressed = 0;
    g_input.any_key_pressed = 0;
    g_input.move_processed = 0;
    /* Don't clear quit_pressed or last_key */
}

void input_clear_quit(void) {
    g_input.quit_pressed = 0;
}

void input_reset(void) {
    g_input.direction = DIR_NONE;
    g_input.held_dir = DIR_NONE;
    g_input.action_pressed = 0;
    g_input.attack_pressed = 0;
    g_input.attack_held = 0;
    g_input.pause_pressed = 0;
    g_input.quit_pressed = 0;
    g_input.last_key = 0;
    g_input.any_key_pressed = 0;
    g_input.move_processed = 0;
}

void input_new_frame(void) {
    /* Reset move_processed to allow one new move this frame */
    g_input.move_processed = 0;

    /* If a key is being held, prepare the direction for this frame */
    if (g_input.held_dir != DIR_NONE) {
        g_input.direction = g_input.held_dir;
    }
}

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

int input_is_key_held(unsigned char scancode) {
    (void)scancode; /* Unused - keys_held removed for simplicity */
    return 0;
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
