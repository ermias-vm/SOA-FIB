/**
 * @file game_ui.c
 * @brief User Interface system implementation for ZeOS Miner game
 */

#include <game_render.h>
#include <game_types.h>
#include <game_ui.h>
#include <libc.h>

/* ============================================================================
 *                            STATIC VARIABLES
 * ============================================================================ */

/* Flash effect state */
static int g_score_flash_timer = 0;
static int g_life_lost_timer = 0;

/* ============================================================================
 *                            MAIN HUD FUNCTIONS
 * ============================================================================ */

void ui_draw_hud(int lives, int score, int round, int time_seconds, int fps) {
    ui_draw_top_bar(time_seconds, fps);
    ui_draw_bottom_bar(lives, score, round);
}

void ui_draw_top_bar(int time_seconds, int fps) {
    Color status_color = render_make_color(COLOR_WHITE, COLOR_BLACK);

    /* Clear the top row */
    render_fill_rect(0, STATUS_TOP_ROW, SCREEN_WIDTH, 1, ' ', status_color);

    /* Draw time on the left */
    ui_draw_time(time_seconds);

    /* Draw FPS on the right */
    ui_draw_fps(fps);
}

void ui_draw_bottom_bar(int lives, int score, int round) {
    Color status_color = render_make_color(COLOR_WHITE, COLOR_BLACK);

    /* Clear the bottom row */
    render_fill_rect(0, STATUS_BOTTOM_ROW, SCREEN_WIDTH, 1, ' ', status_color);

    /* Draw lives on the left */
    ui_draw_lives(lives);

    /* Draw score in the center */
    ui_draw_score(score);

    /* Draw round on the right */
    ui_draw_round(round);
}

/* ============================================================================
 *                         INDIVIDUAL HUD ELEMENTS
 * ============================================================================ */

void ui_draw_time(int seconds) {
    Color time_color = render_make_color(COLOR_WHITE, COLOR_BLACK);

    int minutes = seconds / 60;
    int secs = seconds % 60;

    char time_str[6];
    time_str[0] = '0' + (minutes / 10) % 10;
    time_str[1] = '0' + (minutes % 10);
    time_str[2] = ':';
    time_str[3] = '0' + (secs / 10);
    time_str[4] = '0' + (secs % 10);
    time_str[5] = '\0';

    render_put_string_colored(HUD_TIME_X, STATUS_TOP_ROW, time_str, time_color);
}

void ui_draw_fps(int fps) {
    Color fps_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);

    char fps_str[8];
    ui_number_to_string(fps, fps_str, 3, ' ');

    /* Append " FPS" */
    int len = ui_strlen(fps_str);
    fps_str[len] = ' ';
    fps_str[len + 1] = 'F';
    fps_str[len + 2] = 'P';
    fps_str[len + 3] = 'S';
    fps_str[len + 4] = '\0';

    /* Position at right edge */
    int total_len = ui_strlen(fps_str);
    int x = SCREEN_WIDTH - total_len;
    render_put_string_colored(x, STATUS_TOP_ROW, fps_str, fps_color);
}

void ui_draw_lives(int lives) {
    Color label_color = render_make_color(COLOR_WHITE, COLOR_BLACK);
    Color heart_color = render_make_color(COLOR_LIGHT_RED, COLOR_BLACK);

    /* Draw "LIVES: " label */
    render_put_string_colored(HUD_LIVES_X, STATUS_BOTTOM_ROW, "LIVES: ", label_color);

    /* Draw hearts for each life (max 5) */
    for (int i = 0; i < lives && i < 5; i++) {
        render_set_cell(HUD_LIVES_X + 7 + i, STATUS_BOTTOM_ROW, CHAR_HEART, heart_color);
    }
}

void ui_draw_score(int score) {
    Color score_color;

    /* Flash effect when gaining points */
    if (g_score_flash_timer > 0) {
        score_color = render_make_color(COLOR_WHITE, COLOR_RED);
        g_score_flash_timer--;
    } else {
        score_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);
    }

    char score_text[14] = "SCORE: ";
    char score_digits[6];
    ui_number_to_string(score, score_digits, 5, '0');

    /* Concatenate "SCORE: " + digits */
    int i = 7;
    int j = 0;
    while (score_digits[j] && i < 13) {
        score_text[i++] = score_digits[j++];
    }
    score_text[i] = '\0';

    /* Center the text */
    int len = ui_strlen(score_text);
    int x = (SCREEN_WIDTH - len) / 2;
    render_put_string_colored(x, STATUS_BOTTOM_ROW, score_text, score_color);
}

void ui_draw_round(int round) {
    Color round_color = render_make_color(COLOR_CYAN, COLOR_BLACK);

    /* Build "ROUND:  X" string with two spaces between : and number */
    char round_text[12] = "ROUND:  ";
    char round_digits[3];
    ui_number_to_string(round, round_digits, 2, ' ');

    /* Append the number (skip leading spaces for single digit) */
    int i = 8; /* After "ROUND:  " */
    int j = 0;
    /* Skip leading space for single digit numbers */
    if (round_digits[0] == ' ') j = 1;
    while (round_digits[j] && i < 11) {
        round_text[i++] = round_digits[j++];
    }
    round_text[i] = '\0';

    /* Position at right edge */
    int len = ui_strlen(round_text);
    int x = SCREEN_WIDTH - len;
    render_put_string_colored(x, STATUS_BOTTOM_ROW, round_text, round_color);
}

/* ============================================================================
 *                            SCREEN OVERLAYS
 * ============================================================================ */

void ui_draw_menu_screen(void) {
    Color title_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);
    Color text_color = render_make_color(COLOR_WHITE, COLOR_BLACK);
    Color highlight_color = render_make_color(COLOR_LIGHT_GREEN, COLOR_BLACK);

    /* Clear screen */
    render_clear();

    /* Title */
    ui_draw_centered_text(5, "========================", title_color);
    ui_draw_centered_text(6, "         DIG DUG        ", title_color);
    ui_draw_centered_text(7, "      ZeOS Edition      ", title_color);
    ui_draw_centered_text(8, "========================", title_color);

    /* Instructions */
    ui_draw_centered_text(11, "CONTROLS:", text_color);
    ui_draw_centered_text(13, "W/A/S/D or Arrows - Move", text_color);
    ui_draw_centered_text(14, "SPACE - Inflate enemies", text_color);
    ui_draw_centered_text(15, "P - Pause game", text_color);
    ui_draw_centered_text(16, "Q - Quit game", text_color);

    /* Objective */
    ui_draw_centered_text(18, "OBJECTIVE:", text_color);
    ui_draw_centered_text(19, "Eliminate all enemies", text_color);
    ui_draw_centered_text(20, "Deeper = More points", text_color);

    /* Start prompt */
    ui_draw_centered_text(23, "Press SPACE to start", highlight_color);

    /* Credits */
    ui_draw_centered_text(1, "SOA Project 2025-2026",
                          render_make_color(COLOR_DARK_GRAY, COLOR_BLACK));
}

void ui_draw_pause_screen(void) {
    Color box_color = render_make_color(COLOR_WHITE, COLOR_BLUE);
    Color title_color = render_make_color(COLOR_YELLOW, COLOR_BLUE);
    Color text_color = render_make_color(COLOR_WHITE, COLOR_BLACK);

    /* Draw pause box in center */
    int box_x = (SCREEN_WIDTH - 20) / 2;
    int box_y = 10;
    int box_w = 20;
    int box_h = 5;

    /* Fill box background */
    render_fill_rect(box_x, box_y, box_w, box_h, ' ', box_color);

    /* Draw border */
    ui_draw_border(box_x, box_y, box_w, box_h, box_color);

    /* Pause text */
    ui_draw_centered_text(box_y + 2, "PAUSED", title_color);

    /* Instructions below box */
    ui_draw_centered_text(box_y + box_h + 1, "Press P to continue", text_color);
}

void ui_draw_game_over_screen(int final_score) {
    Color title_color = render_make_color(COLOR_LIGHT_RED, COLOR_BLACK);
    Color text_color = render_make_color(COLOR_WHITE, COLOR_BLACK);
    Color score_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);

    /* Clear screen */
    render_clear();

    /* Game Over title */
    ui_draw_centered_text(7, "========================", title_color);
    ui_draw_centered_text(8, "      GAME  OVER        ", title_color);
    ui_draw_centered_text(9, "========================", title_color);

    /* Final score */
    ui_draw_centered_text(12, "FINAL SCORE:", text_color);

    char score_str[6];
    ui_number_to_string(final_score, score_str, 5, '0');
    ui_draw_centered_text(14, score_str, score_color);

    /* Options */
    ui_draw_centered_text(18, "Press SPACE to restart", text_color);
    ui_draw_centered_text(19, "Press Q to quit", text_color);
}

void ui_draw_level_clear_screen(int round, int score) {
    (void)score; /* Score not displayed in current implementation */
    Color box_color = render_make_color(COLOR_BLACK, COLOR_GREEN);
    Color title_color = render_make_color(COLOR_WHITE, COLOR_GREEN);
    Color text_color = render_make_color(COLOR_WHITE, COLOR_BLACK);

    /* Draw completion box */
    int box_x = (SCREEN_WIDTH - 30) / 2;
    int box_y = 9;
    int box_w = 30;
    int box_h = 7;

    /* Fill background */
    render_fill_rect(box_x, box_y, box_w, box_h, ' ', box_color);

    /* Title */
    ui_draw_centered_text(box_y + 1, "ROUND CLEAR!", title_color);

    /* Round completed */
    char round_text[20] = "Round ";
    char round_num[3];
    ui_number_to_string(round, round_num, 2, ' ');

    int i = 6;
    int j = 0;
    while (round_num[j]) {
        if (round_num[j] != ' ' || i > 6) {
            round_text[i++] = round_num[j];
        }
        j++;
    }
    round_text[i++] = ' ';
    round_text[i++] = 'C';
    round_text[i++] = 'l';
    round_text[i++] = 'e';
    round_text[i++] = 'a';
    round_text[i++] = 'r';
    round_text[i++] = 'e';
    round_text[i++] = 'd';
    round_text[i] = '\0';

    ui_draw_centered_text(box_y + 3, round_text, title_color);

    /* Next round message */
    ui_draw_centered_text(box_y + box_h + 1, "Preparing next round...", text_color);
}

void ui_draw_victory_screen(int final_score) {
    Color title_color = render_make_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    Color text_color = render_make_color(COLOR_WHITE, COLOR_BLACK);
    Color score_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);
    Color subtitle_color = render_make_color(COLOR_CYAN, COLOR_BLACK);

    /* Clear screen */
    render_clear();

    /* Victory title */
    ui_draw_centered_text(5, "========================", title_color);
    ui_draw_centered_text(6, "       YOU WIN!         ", title_color);
    ui_draw_centered_text(7, "========================", title_color);

    /* Subtitle */
    ui_draw_centered_text(9, "Finally, Baka Baka is defeated", subtitle_color);

    /* Final score */
    ui_draw_centered_text(12, "TOTAL SCORE:", text_color);

    char score_str[6];
    ui_number_to_string(final_score, score_str, 5, '0');
    ui_draw_centered_text(14, score_str, score_color);

    /* Options */
    ui_draw_centered_text(18, "Press SPACE to play again", text_color);
    ui_draw_centered_text(19, "Press C for credits", text_color);
    ui_draw_centered_text(20, "Press ESC for main menu", text_color);
}

void ui_draw_credits_screen(void) {
    Color title_color = render_make_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    Color text_color = render_make_color(COLOR_WHITE, COLOR_BLACK);
    Color name_color = render_make_color(COLOR_YELLOW, COLOR_BLACK);

    /* Clear screen */
    render_clear();

    /* Credits title */
    ui_draw_centered_text(5, "========================", title_color);
    ui_draw_centered_text(6, "        CREDITS         ", title_color);
    ui_draw_centered_text(7, "========================", title_color);

    /* Developers */
    ui_draw_centered_text(10, "Developed by:", text_color);
    ui_draw_centered_text(12, "ERMIAS VALLS", name_color);
    ui_draw_centered_text(14, "MARC DE RIALP", name_color);

    /* Course info */
    ui_draw_centered_text(17, "SOA - FIB UPC", text_color);
    ui_draw_centered_text(18, "2024-2025", text_color);

    /* Return option */
    ui_draw_centered_text(21, "Press ESC to return", text_color);
}

/* ============================================================================
 *                            TEXT UTILITIES
 * ============================================================================ */

void ui_draw_centered_text(int y, const char *text, Color color) {
    int len = ui_strlen(text);
    int x = (SCREEN_WIDTH - len) / 2;
    if (x < 0) x = 0;

    render_put_string_colored(x, y, text, color);
}

void ui_draw_message_box(const char *title, const char *message) {
    Color box_color = render_make_color(COLOR_BLACK, COLOR_WHITE);
    Color title_color = render_make_color(COLOR_BLUE, COLOR_WHITE);
    Color text_color = render_make_color(COLOR_BLACK, COLOR_WHITE);

    int title_len = ui_strlen(title);
    int msg_len = ui_strlen(message);
    int box_width = (title_len > msg_len ? title_len : msg_len) + 4;
    int box_x = (SCREEN_WIDTH - box_width) / 2;
    int box_y = 10;

    /* Draw box background */
    render_fill_rect(box_x, box_y, box_width, 5, ' ', box_color);

    /* Draw border */
    ui_draw_border(box_x, box_y, box_width, 5, box_color);

    /* Draw title and message */
    int title_x = box_x + (box_width - title_len) / 2;
    int msg_x = box_x + (box_width - msg_len) / 2;

    render_put_string_colored(title_x, box_y + 1, title, title_color);
    render_put_string_colored(msg_x, box_y + 3, message, text_color);
}

void ui_draw_border(int x, int y, int w, int h, Color color) {
    /* Top and bottom borders */
    render_draw_horizontal_line(x, y, w, CHAR_BORDER_H, color);
    render_draw_horizontal_line(x, y + h - 1, w, CHAR_BORDER_H, color);

    /* Left and right borders */
    render_draw_vertical_line(x, y, h, CHAR_BORDER_V, color);
    render_draw_vertical_line(x + w - 1, y, h, CHAR_BORDER_V, color);

    /* Corners */
    render_set_cell(x, y, CHAR_CORNER, color);
    render_set_cell(x + w - 1, y, CHAR_CORNER, color);
    render_set_cell(x, y + h - 1, CHAR_CORNER, color);
    render_set_cell(x + w - 1, y + h - 1, CHAR_CORNER, color);
}

/* ============================================================================
 *                           UTILITY FUNCTIONS
 * ============================================================================ */

int ui_strlen(const char *str) {
    int len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

void ui_number_to_string(int number, char *buffer, int digits, char pad_char) {
    int i;
    int has_digit = 0;

    /* Handle negative numbers */
    int negative = (number < 0);
    if (negative) {
        number = -number;
    }

    /* Fill buffer from right to left */
    for (i = digits - 1; i >= 0; i--) {
        if (number > 0 || !has_digit) {
            buffer[i] = '0' + (number % 10);
            number /= 10;
            has_digit = 1;
        } else {
            buffer[i] = pad_char;
        }
    }

    /* Add negative sign if needed and space padding */
    if (negative && pad_char == ' ') {
        for (i = 0; i < digits - 1; i++) {
            if (buffer[i] != ' ') {
                if (i > 0) {
                    buffer[i - 1] = '-';
                } else {
                    buffer[0] = '-'; /* Overflow: just put - at start */
                }
                break;
            }
        }
    }

    buffer[digits] = '\0';
}

void ui_flash_score(int duration) {
    g_score_flash_timer = duration;
}

void ui_animate_life_lost(void) {
    g_life_lost_timer = 30; /* Flash for 30 frames */
}

void ui_clear_game_area(void) {
    /* Clear only rows 1-23 (preserve HUD) */
    for (int y = SKY_START_ROW; y <= GROUND_END_ROW; y++) {
        Color layer_color = render_get_layer_color(y);
        render_fill_rect(0, y, SCREEN_WIDTH, 1, ' ', layer_color);
    }
}
