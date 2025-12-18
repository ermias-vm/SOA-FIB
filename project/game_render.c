/**
 * @file game_render.c
 * @brief Double-buffered rendering system implementation for ZeOS Miner
 *
 * Uses direct screen buffer writes via fd=10 for efficient rendering.
 */

#include <game_render.h>
#include <game_types.h>
#include <libc.h>

/* ============================================================================
 *                            GLOBAL VARIABLES
 * ============================================================================ */

/* Double buffers for flicker-free rendering */
ScreenBuffer g_front_buffer;
ScreenBuffer g_back_buffer;

/* VGA format buffer for direct screen writes (80*25*2 = 4000 bytes) */
static char g_vga_buffer[SCREEN_SIZE];

/* Default color for rendering operations */
static Color g_default_color = {COLOR_WHITE, COLOR_BLACK};

/* ============================================================================
 *                            INITIALIZATION
 * ============================================================================ */

void render_init(void) {
    /* Initialize default color */
    g_default_color.fg = COLOR_WHITE;
    g_default_color.bg = COLOR_BLACK;

    /* Clear both buffers */
    render_clear_buffer(&g_front_buffer);
    render_clear_buffer(&g_back_buffer);

    /* Clear the VGA buffer */
    for (int i = 0; i < SCREEN_SIZE; i += 2) {
        g_vga_buffer[i] = ' ';
        g_vga_buffer[i + 1] = 0x07; /* Light gray on black */
    }

    /* Write initial cleared buffer to screen */
    write(10, g_vga_buffer, SCREEN_SIZE);
}

void render_cleanup(void) {
    /* Fill VGA buffer with spaces */
    for (int i = 0; i < SCREEN_SIZE; i += 2) {
        g_vga_buffer[i] = ' ';
        g_vga_buffer[i + 1] = 0x07; /* Light gray on black */
    }

    /* Write to screen */
    write(10, g_vga_buffer, SCREEN_SIZE);
}

/* ============================================================================
 *                            BUFFER OPERATIONS
 * ============================================================================ */

void render_clear(void) {
    render_clear_buffer(&g_back_buffer);
}

void render_clear_buffer(ScreenBuffer *buffer) {
    if (!buffer) return;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        Color layer_color = render_get_layer_color(y);

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            buffer->cells[y][x].character = ' ';
            buffer->cells[y][x].color = layer_color;
        }
    }
    buffer->dirty = 1;
}

void render_set_cell(int x, int y, char c, Color color) {
    if (!render_is_valid_pos(x, y)) {
        return;
    }

    g_back_buffer.cells[y][x].character = c;
    g_back_buffer.cells[y][x].color = color;
    g_back_buffer.dirty = 1;
}

void render_put_char(int x, int y, char c) {
    render_set_cell(x, y, c, g_default_color);
}

void render_put_string(int x, int y, const char *str) {
    render_put_string_colored(x, y, str, g_default_color);
}

void render_put_string_colored(int x, int y, const char *str, Color color) {
    if (!str || !render_is_valid_pos(x, y)) {
        return;
    }

    int pos = x;
    while (*str && pos < SCREEN_WIDTH) {
        render_set_cell(pos, y, *str, color);
        str++;
        pos++;
    }
}

/* ============================================================================
 *                            DRAWING PRIMITIVES
 * ============================================================================ */

void render_fill_rect(int x, int y, int w, int h, char c, Color color) {
    for (int row = y; row < y + h && row < SCREEN_HEIGHT; row++) {
        for (int col = x; col < x + w && col < SCREEN_WIDTH; col++) {
            if (render_is_valid_pos(col, row)) {
                render_set_cell(col, row, c, color);
            }
        }
    }
}

void render_draw_horizontal_line(int x, int y, int length, char c, Color color) {
    for (int i = 0; i < length && (x + i) < SCREEN_WIDTH; i++) {
        if (render_is_valid_pos(x + i, y)) {
            render_set_cell(x + i, y, c, color);
        }
    }
}

void render_draw_vertical_line(int x, int y, int length, char c, Color color) {
    for (int i = 0; i < length && (y + i) < SCREEN_HEIGHT; i++) {
        if (render_is_valid_pos(x, y + i)) {
            render_set_cell(x, y + i, c, color);
        }
    }
}

/* ============================================================================
 *                            COLOR MANAGEMENT
 * ============================================================================ */

void render_set_default_color(Color color) {
    g_default_color = color;
}

Color render_get_layer_color(int y) {
    Color color;
    color.fg = COLOR_WHITE;

    if (y == STATUS_TOP_ROW || y == STATUS_BOTTOM_ROW) {
        /* Status bars: white text on black */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_STATUS_BG;
    } else if (y >= SKY_START_ROW && y <= SKY_END_ROW) {
        /* Sky area: light blue background */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_SKY_BG;
    } else if (y >= LAYER1_START && y <= LAYER1_END) {
        /* Layer 1: brown background (200 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_LAYER1_BG;
    } else if (y >= LAYER2_START && y <= LAYER2_END) {
        /* Layer 2: red background (300 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_LAYER2_BG;
    } else if (y >= LAYER3_START && y <= LAYER3_END) {
        /* Layer 3: magenta background (400 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_LAYER3_BG;
    } else if (y >= LAYER4_START && y <= LAYER4_END) {
        /* Layer 4: dark gray background (500 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_LAYER4_BG;
    } else {
        /* Default: black background */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_BLACK;
    }

    return color;
}

Color render_make_color(unsigned char fg, unsigned char bg) {
    Color color;
    color.fg = fg;
    color.bg = bg;
    return color;
}

/* ============================================================================
 *                            PRESENTATION
 * ============================================================================ */

/**
 * @brief Convert a cell to VGA format and write to VGA buffer.
 */
void render_cell_to_vga(char *vga_buffer, const ScreenCell *cell, int offset) {
    /* VGA format: byte 0 = character, byte 1 = attribute */
    /* Attribute: bits 0-3 = foreground, bits 4-6 = background, bit 7 = blink */
    vga_buffer[offset] = cell->character;
    vga_buffer[offset + 1] = (cell->color.bg << 4) | (cell->color.fg & 0x0F);
}

void render_swap_buffers(void) {
    /* Copy back to front buffer */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            g_front_buffer.cells[y][x] = g_back_buffer.cells[y][x];
        }
    }

    /* Clear dirty flag */
    g_front_buffer.dirty = 0;
}

void render_present(void) {
    /* Convert back buffer to VGA format and write to screen */
    render_present_buffer();
}

void render_present_buffer(void) {
    /* Convert ScreenCell format to VGA format */
    int offset = 0;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            ScreenCell *cell = &g_back_buffer.cells[y][x];
            render_cell_to_vga(g_vga_buffer, cell, offset);
            offset += 2;
        }
    }

    /* Write entire buffer to screen using fd=10 */
    write(10, g_vga_buffer, SCREEN_SIZE);

    /* Copy back to front buffer for change tracking */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            g_front_buffer.cells[y][x] = g_back_buffer.cells[y][x];
        }
    }

    /* Clear dirty flag */
    g_back_buffer.dirty = 0;
}

void render_present_full(void) {
    /* Same as render_present_buffer - full screen update */
    render_present_buffer();
}

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

void render_number(int x, int y, int number, int digits) {
    render_number_padded_char(x, y, number, digits, ' ');
}

void render_number_padded(int x, int y, int number, int digits) {
    render_number_padded_char(x, y, number, digits, '0');
}

void render_number_padded_char(int x, int y, int number, int digits, char pad_char) {
    char buffer[16];
    int i;

    /* Handle negative numbers */
    int negative = (number < 0);
    if (negative) {
        number = -number;
    }

    /* Convert to string (reverse order) */
    for (i = digits - 1; i >= 0; i--) {
        if (number > 0) {
            buffer[i] = '0' + (number % 10);
            number /= 10;
        } else {
            buffer[i] = pad_char;
        }
    }

    /* Add negative sign if needed */
    if (negative && pad_char == ' ') {
        /* Find first non-space and replace with minus */
        for (i = 0; i < digits - 1; i++) {
            if (buffer[i] != ' ') {
                buffer[i - 1] = '-';
                break;
            }
        }
    }

    buffer[digits] = '\0';
    render_put_string(x, y, buffer);
}

int render_is_valid_pos(int x, int y) {
    return (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT);
}

const ScreenCell *render_get_cell(int x, int y) {
    if (!render_is_valid_pos(x, y)) {
        return 0; /* NULL */
    }
    return &g_back_buffer.cells[y][x];
}
