/**
 * @file screen_samples.c
 * @brief Implementation of screen pattern generators for testing.
 *
 * This file provides functions to generate different visual patterns
 * for testing the screen buffer functionality.
 */

#include <io.h>
#include <screen_samples.h>

void generate_checkerboard_pattern(char *buffer) {
    for (int y = 0; y < NUM_ROWS; y++) {
        for (int x = 0; x < NUM_COLUMNS; x++) {
            int pos = (y * NUM_COLUMNS + x) * 2;
            /* Checkerboard pattern: alternate A/B both horizontally and vertically */
            int is_alternate = (x + y) % 2;

            buffer[pos] = is_alternate ? 'B' : 'A';       /* Character */
            buffer[pos + 1] = is_alternate ? 0x4F : 0x1F; /* Red on white : Blue on white */
        }
    }
}

void generate_rainbow_pattern(char *buffer) {
    for (int y = 0; y < NUM_ROWS; y++) {
        for (int x = 0; x < NUM_COLUMNS; x++) {
            int pos = (y * NUM_COLUMNS + x) * 2;

            /* Character: cycle through numbers 0-9 */
            buffer[pos] = '0' + ((x + y) % 10);

            /* Color: create rainbow effect by cycling background colors */
            int color_index = (x + y * 3) % 16; /* 16 possible background colors */
            buffer[pos + 1] =
                (color_index << 4) | 0x0F; /* Bright white text on colored background */
        }
    }
}

void generate_border_pattern(char *buffer) {
    for (int y = 0; y < NUM_ROWS; y++) {
        for (int x = 0; x < NUM_COLUMNS; x++) {
            int pos = (y * NUM_COLUMNS + x) * 2;

            if (y == 0 || y == NUM_ROWS - 1) {
                /* Top and bottom border */
                if (x == 0 || x == NUM_COLUMNS - 1) {
                    buffer[pos] = '+'; /* Corners */
                } else {
                    buffer[pos] = '-'; /* Horizontal border */
                }
                buffer[pos + 1] = 0x0F; /* White on black */
            } else if (x == 0 || x == NUM_COLUMNS - 1) {
                /* Left and right border */
                buffer[pos] = '|';      /* Vertical border */
                buffer[pos + 1] = 0x0F; /* White on black */
            } else {
                /* Interior content */
                if (y == NUM_ROWS / 2 && x >= 10 && x < 70) {
                    /* Center text line */
                    const char *text = "SCREEN BUFFER TEST PATTERN - BORDER STYLE";
                    int text_len = 42; /* Length of text above */
                    int text_start = 10;

                    if (x - text_start < text_len) {
                        buffer[pos] = text[x - text_start];
                        buffer[pos + 1] = 0x2F; /* Green on bright white */
                    } else {
                        buffer[pos] = ' ';
                        buffer[pos + 1] = 0x07; /* Light gray on black */
                    }
                } else if (y == NUM_ROWS / 2 + 2 && x >= 20 && x < 60) {
                    /* Second text line */
                    const char *text = "80x25 Characters, 4000 Bytes Total";
                    int text_len = 35;
                    int text_start = 20;

                    if (x - text_start < text_len) {
                        buffer[pos] = text[x - text_start];
                        buffer[pos + 1] = 0x1E; /* Yellow on blue */
                    } else {
                        buffer[pos] = ' ';
                        buffer[pos + 1] = 0x07; /* Light gray on black */
                    }
                } else {
                    /* Empty space */
                    buffer[pos] = ' ';
                    buffer[pos + 1] = 0x07; /* Light gray on black */
                }
            }
        }
    }
}
