/**
 * @file screen_samples.c
 * @brief Implementation of screen pattern generators and scene rendering.
 *
 * This file provides functions to generate different visual patterns
 * and animated scenes for testing the screen buffer functionality.
 */

#include <io.h>
#include <libc.h>
#include <screen_samples.h>

void scene_init(SceneState *state) {
    state->num_balls = 0;
    state->num_stars = 0;
    state->frame_count = 0;

    /* Initialize stars for starfield scene */
    for (int i = 0; i < SCENE_MAX_STARS; i++) {
        state->stars[i].x = (i * 17 + 7) % NUM_COLUMNS;
        state->stars[i].y = 1 + ((i * 13 + 3) % (NUM_ROWS - 1));
        state->stars[i].ch = (i % 3 == 0) ? '*' : ((i % 3 == 1) ? '+' : '.');
        state->stars[i].color = 0x07 + (i % 9); /* Cycle through light colors */
        state->stars[i].twinkle = i % 20;
    }
    state->num_stars = SCENE_MAX_STARS;
}

void scene_fill_black(char *buffer) {
    /* Fill all rows - draw_time_and_fps() will repaint row 0 after write */
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLUMNS; col++) {
            int idx = (row * NUM_COLUMNS + col) * 2;
            buffer[idx] = ' ';
            buffer[idx + 1] = 0x00; /* Black on black */
        }
    }
}

void scene_draw_char(char *buffer, int x, int y, char ch, char color) {
    /* Allow drawing on row 0 - draw_time_and_fps() will repaint after write */
    if (x < 0 || x >= NUM_COLUMNS || y < 0 || y >= NUM_ROWS) return;

    int idx = (y * NUM_COLUMNS + x) * 2;
    buffer[idx] = ch;
    buffer[idx + 1] = color;
}

void scene_draw_string(char *buffer, int x, int y, const char *str, char color) {
    for (int i = 0; str[i] && (x + i) < NUM_COLUMNS; i++) {
        scene_draw_char(buffer, x + i, y, str[i], color);
    }
}

void scene_draw_nav_message(char *buffer, int scene_num, int total_scenes) {
    /*
     * Clear row 0 EXCEPT:
     * - First 6 columns (reserved for time display "SS:MMM")
     * - Last 8 columns (reserved for FPS display "XXXX FPS")
     * This allows the kernel's show_time_and_fps() to be visible
     */
    for (int col = 7; col < NUM_COLUMNS - 9; col++) {
        buffer[col * 2] = ' ';
        buffer[col * 2 + 1] = 0x00;
    }

    /* Build message: "Scene X/Y  N:Next B:Back Esc:Exit" */
    char msg[40];
    msg[0] = 'S';
    msg[1] = 'c';
    msg[2] = 'e';
    msg[3] = 'n';
    msg[4] = 'e';
    msg[5] = ' ';
    msg[6] = '0' + scene_num;
    msg[7] = '/';
    msg[8] = '0' + total_scenes;
    msg[9] = ' ';
    msg[10] = ' ';
    msg[11] = 'N';
    msg[12] = ':';
    msg[13] = 'N';
    msg[14] = 'e';
    msg[15] = 'x';
    msg[16] = 't';
    msg[17] = ' ';
    msg[18] = 'B';
    msg[19] = ':';
    msg[20] = 'B';
    msg[21] = 'a';
    msg[22] = 'c';
    msg[23] = 'k';
    msg[24] = ' ';
    msg[25] = 'E';
    msg[26] = 's';
    msg[27] = 'c';
    msg[28] = ':';
    msg[29] = 'E';
    msg[30] = 'x';
    msg[31] = 'i';
    msg[32] = 't';
    msg[33] = '\0';

    /* Center the message in the available space (between time and FPS) */
    int msg_len = 33;
    int available_start = 7;
    int available_end = NUM_COLUMNS - 9;
    int available_width = available_end - available_start;
    int start_col = available_start + (available_width - msg_len) / 2;

    for (int i = 0; msg[i]; i++) {
        int idx = (start_col + i) * 2;
        buffer[idx] = msg[i];
        buffer[idx + 1] = 0x1F; /* White on blue background for visibility */
    }
}

void scene_add_ball(SceneState *state, int x, int y) {
    if (state->num_balls >= SCENE_MAX_BALLS) return;

    SceneBall *ball = &state->balls[state->num_balls];
    ball->x = x;
    ball->y = y;
    ball->dx = (x % 2) ? 1 : -1;
    ball->dy = (y % 2) ? 1 : -1;
    ball->ch = 'O';
    /* Cycle through bright colors: red, green, yellow, cyan, magenta */
    char colors[] = {0x0C, 0x0A, 0x0E, 0x0B, 0x0D};
    ball->color = colors[state->num_balls % 5];
    state->num_balls++;
}

void scene_update_balls(SceneState *state) {
    /* Only update every BALL_SPEED_DIVISOR frames for slower movement */
    if ((state->frame_count % BALL_SPEED_DIVISOR) != 0) return;

    for (int i = 0; i < state->num_balls; i++) {
        SceneBall *ball = &state->balls[i];

        ball->x += ball->dx;
        ball->y += ball->dy;

        /* Bounce off horizontal walls */
        if (ball->x <= 0 || ball->x >= NUM_COLUMNS - 1) {
            ball->dx = -ball->dx;
            ball->x += ball->dx;
        }

        /* Bounce off vertical walls (skip row 0) */
        if (ball->y <= 1 || ball->y >= NUM_ROWS - 1) {
            ball->dy = -ball->dy;
            ball->y += ball->dy;
        }
    }
}

void render_scene_starfield(char *buffer, SceneState *state) {
    scene_fill_black(buffer);

    int frame = state->frame_count;

    /* Draw twinkling stars */
    for (int i = 0; i < state->num_stars; i++) {
        SceneStar *star = &state->stars[i];

        /* Twinkle effect: some stars blink */
        if ((frame + star->twinkle) % 30 < 25) {
            char color = star->color;
            /* Brightness variation */
            if ((frame + star->twinkle) % 15 < 5) {
                color = 0x08; /* Dark gray */
            }
            scene_draw_char(buffer, star->x, star->y, star->ch, color);
        }
    }

    /* Draw a slow-moving horizontal bar (nebula) - moves left-right and back */
    int nebula_y = 12;                          /* Fixed vertical position */
    int nebula_width = 20;                      /* Width of the nebula bar */
    int max_x = NUM_COLUMNS - nebula_width - 5; /* Max starting position */
    int cycle = (frame / 3) % (max_x * 2);      /* Full cycle: left-right-left */
    int nebula_start = (cycle < max_x) ? cycle : (max_x * 2 - cycle); /* Bounce back */

    for (int x = nebula_start; x < nebula_start + nebula_width; x++) {
        if ((x + frame / 10) % 3 == 0) {
            scene_draw_char(buffer, x, nebula_y, '~', 0x09);     /* Light blue */
            scene_draw_char(buffer, x, nebula_y + 1, '*', 0x01); /* Dark blue trail */
        }
    }

    /* Draw planet in corner */
    scene_draw_char(buffer, 70, 5, '(', 0x06);
    scene_draw_char(buffer, 71, 5, ')', 0x06);
    scene_draw_char(buffer, 69, 4, '/', 0x08);
    scene_draw_char(buffer, 72, 4, '\\', 0x08);

    /* Draw scene title */
    scene_draw_string(buffer, 2, 23, "SCENE 3: Starfield", 0x0F);

    state->frame_count++;
}

void render_scene_balls(char *buffer, SceneState *state) {
    scene_fill_black(buffer);

    int frame = state->frame_count;

    /* Add a new ball every BALL_SPAWN_INTERVAL frames until max */
    if (frame % BALL_SPAWN_INTERVAL == 0 && state->num_balls < SCENE_MAX_BALLS) {
        scene_add_ball(state, 10 + (state->num_balls * 7) % 60, 5 + (state->num_balls * 3) % 15);
    }

    /* Draw decorative border blocks */
    for (int i = 0; i < NUM_COLUMNS; i += 10) {
        scene_draw_char(buffer, i, 2, '#', 0x08);
        scene_draw_char(buffer, i, 22, '#', 0x08);
    }
    for (int i = 2; i < NUM_ROWS - 2; i += 5) {
        scene_draw_char(buffer, 2, i, '#', 0x08);
        scene_draw_char(buffer, NUM_COLUMNS - 3, i, '#', 0x08);
    }

    /* Draw moving vertical lines */
    int line_x = 20 + (frame / 10) % 40;
    for (int y = 3; y < 22; y += 2) {
        scene_draw_char(buffer, line_x, y, '|', 0x01);
    }

    /* Update and draw all balls */
    scene_update_balls(state);
    for (int i = 0; i < state->num_balls; i++) {
        scene_draw_char(buffer, state->balls[i].x, state->balls[i].y, state->balls[i].ch,
                        state->balls[i].color);
    }

    /* Draw scene title and ball count */
    scene_draw_string(buffer, 2, 23, "SCENE 4: Bouncing Balls", 0x0F);

    /* Draw ball count (supports up to 999) */
    char count_str[15];
    count_str[0] = 'B';
    count_str[1] = 'a';
    count_str[2] = 'l';
    count_str[3] = 'l';
    count_str[4] = 's';
    count_str[5] = ':';
    count_str[6] = ' ';
    count_str[7] = '0' + (state->num_balls / 100);
    count_str[8] = '0' + ((state->num_balls / 10) % 10);
    count_str[9] = '0' + (state->num_balls % 10);
    count_str[10] = '\0';
    scene_draw_string(buffer, 2, 24, count_str, 0x0E);

    state->frame_count++;
}

void generate_checkerboard_pattern(char *buffer) {
    for (int y = 0; y < NUM_ROWS; y++) {
        for (int x = 0; x < NUM_COLUMNS; x++) {
            int pos = (y * NUM_COLUMNS + x) * 2;

            /* Row 0 is black for time/FPS display */
            if (y == 0) {
                buffer[pos] = ' ';
                buffer[pos + 1] = 0x00;
                continue;
            }

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

            /* Row 0 is black for time/FPS display */
            if (y == 0) {
                buffer[pos] = ' ';
                buffer[pos + 1] = 0x00;
                continue;
            }

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
    const char *text1 = "SCREEN BUFFER TEST PATTERN - BORDER STYLE";
    const char *text2 = "80x25 Characters, 4000 Bytes Total";
    int text_len1 = strlen(text1);
    int text_len2 = strlen(text2);

    for (int y = 0; y < NUM_ROWS; y++) {
        for (int x = 0; x < NUM_COLUMNS; x++) {
            int pos = (y * NUM_COLUMNS + x) * 2;

            /* Row 0 is black for time/FPS display */
            if (y == 0) {
                buffer[pos] = ' ';
                buffer[pos + 1] = 0x00;
                continue;
            }

            if (y == 1 || y == NUM_ROWS - 1) {
                /* Top and bottom border */
                if (x == 0 || x == NUM_COLUMNS - 1) {
                    buffer[pos] = '+'; /* Corners */
                } else {
                    buffer[pos] = '-'; /* Horizontal border */
                }
                buffer[pos + 1] = 0x0F;
            } else if (x == 0 || x == NUM_COLUMNS - 1) {
                /* Left and right border */
                buffer[pos] = '|'; /* Vertical border */
                buffer[pos + 1] = 0x0F;
            } else {
                /* Interior content */
                if (y == NUM_ROWS / 2 && x >= 10 && x < 70) {
                    int text_start = 10;
                    if (x - text_start < text_len1) {
                        buffer[pos] = text1[x - text_start];
                        buffer[pos + 1] = 0x2F;
                    } else {
                        buffer[pos] = ' ';
                        buffer[pos + 1] = 0x07;
                    }
                } else if (y == NUM_ROWS / 2 + 2 && x >= 20 && x < 60) {
                    /* Second text line */
                    int text_start = 20;

                    if (x - text_start < text_len2) {
                        buffer[pos] = text2[x - text_start];
                        buffer[pos + 1] = 0x1E;
                    } else {
                        buffer[pos] = ' ';
                        buffer[pos + 1] = 0x07;
                    }
                } else {
                    /* Empty space */
                    buffer[pos] = ' ';
                    buffer[pos + 1] = 0x07;
                }
            }
        }
    }
}
