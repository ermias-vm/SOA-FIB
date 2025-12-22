/**
 * @file game_input.c
 * @brief Input system implementation for ZeOS Miner game
 */

#include <game_input.h>
#include <game_types.h>
#include <libc.h>
#include <stddef.h>
#include <times.h>

/* Global input state */
volatile InputState g_input;

/* Ticks to hold before continuous movement starts */
#define HOLD_THRESHOLD (EIGHTH_SECOND)

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
    /* Increment hold time if a direction key is held */
    if (g_input.held_dir != DIR_NONE) {
        g_input.hold_time++;
    }
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
            /* Key pressed: set held direction and mark as just pressed */
            g_input.held_dir = key_dir;
            g_input.move_just_pressed = 1;
            g_input.hold_time = 0;
            g_input.move_processed = 0;
        } else {
            /* Key released: clear held direction only if it matches */
            if (g_input.held_dir == key_dir) {
                g_input.held_dir = DIR_NONE;
                g_input.hold_time = 0;
                g_input.move_just_pressed = 0;
            }
        }
        return;
    }

    /* Handle space key (attack) specially - it's held, not just pressed */
    if (key == KEY_SPACE) {
        if (pressed) {
            g_input.attack_held = 1;
            g_input.attack_pressed = 1;
            /* SPACE is only for attack, not for menu actions */
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

    case KEY_K:
        g_input.dev_kill_pressed = 1;
        break;
    }
}

/* ============================================================================
 *                            INPUT QUERIES
 * ============================================================================ */

Direction input_get_direction(void) {
    /* No movement key held */
    if (g_input.held_dir == DIR_NONE) {
        return DIR_NONE;
    }

    /* First press: allow one immediate move */
    if (g_input.move_just_pressed && !g_input.move_processed) {
        g_input.move_processed = 1;
        g_input.move_just_pressed = 0; /* Consume the just-pressed flag */
        return g_input.held_dir;
    }

    /* Continuous movement: only after holding for HOLD_THRESHOLD ticks */
    if (g_input.hold_time >= HOLD_THRESHOLD) {
        return g_input.held_dir;
    }

    /* Key is held but not long enough for continuous movement */
    return DIR_NONE;
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

int input_is_dev_kill_pressed(void) {
    int pressed = g_input.dev_kill_pressed;
    g_input.dev_kill_pressed = 0; /* Consume the input */
    return pressed;
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
    g_input.move_just_pressed = 0;
    g_input.hold_time = 0;
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
    g_input.move_just_pressed = 0;
    g_input.hold_time = 0;
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
    /* Reset move_processed for the new frame */
    /* Note: move_just_pressed is consumed by input_get_direction */
}
