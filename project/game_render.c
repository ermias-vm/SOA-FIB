/**
 * @file game_render.c
 * @brief Double-buffered rendering system implementation for ZeOS Miner
 *
 * Uses direct screen buffer writes via fd=10 for efficient rendering.
 */

#include <game_config.h>
#include <game_logic.h>
#include <game_map.h>
#include <game_render.h>
#include <game_types.h>
#include <game_ui.h>
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

void render_clear_black(void) {
    Color black_color;
    black_color.fg = COLOR_WHITE;
    black_color.bg = COLOR_BLACK;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            g_back_buffer.cells[y][x].character = ' ';
            g_back_buffer.cells[y][x].color = black_color;
        }
    }
    g_back_buffer.dirty = 1;
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

    if (y == STATUS_TOP_ROW || y == STATUS_BOTTOM_ROW) {
        /* Status bars: white text on black */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_BLACK;
    } else if (y >= SKY_START_ROW && y <= SKY_END_ROW) {
        /* Sky area: completely black - empty cells */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_BLACK;
    } else if (y >= LAYER1_START && y <= LAYER1_END) {
        /* Layer 1: brown background (200 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_BROWN;
    } else if (y >= LAYER2_START && y <= LAYER2_END) {
        /* Layer 2: red background (300 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_RED;
    } else if (y >= LAYER3_START && y <= LAYER3_END) {
        /* Layer 3: magenta background (400 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_MAGENTA;
    } else if (y >= LAYER4_START && y <= LAYER4_END) {
        /* Layer 4: blue background (500 pts) */
        color.fg = COLOR_WHITE;
        color.bg = COLOR_BLUE;
    } else if (y == ROW_BORDER) {
        /* Bottom border: gray # on black */
        color.fg = COLOR_DARK_GRAY;
        color.bg = COLOR_BLACK;
    } else {
        /* Default: white on black (includes status bars) */
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

/* ============================================================================
 *                     GAME RENDERING (M5.9)
 * ============================================================================ */

void render_game(GameLogicState *state) {
    if (!state) return;

    /* 1. Clear the buffer (with colors by layer) */
    render_clear();

    /* 2. Draw the map (terrain and tunnels) */
    render_map();

    /* 3. Draw entities */
    render_entities(state);

    /* 4. Draw HUD - pass time_elapsed directly (ui_draw_time handles conversion) */
    ui_draw_hud(state->lives, state->score, state->round, state->time_elapsed, 0);

    /* 5. Draw special screens if needed */
    switch (state->scene) {
    case SCENE_PAUSED:
        ui_draw_pause_screen();
        break;
    case SCENE_ROUND_CLEAR:
        ui_draw_level_clear_screen(state->round, state->score);
        break;
    case SCENE_GAME_OVER:
        ui_draw_game_over_screen(state->score);
        break;
    case SCENE_MENU:
        ui_draw_menu_screen();
        break;
    default:
        break;
    }

    /* 6. Present the frame */
    render_present();
}

void render_map(void) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        /* Skip status rows */
        if (y == STATUS_TOP_ROW || y == STATUS_BOTTOM_ROW) {
            continue;
        }

        Color layer_color = render_get_layer_color(y);
        Color empty_color;
        empty_color.fg = COLOR_WHITE;
        empty_color.bg = COLOR_BLACK;

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            TileType tile = map_get_tile(x, y);
            char display_char;
            Color cell_color;

            switch (tile) {
            case TILE_DIRT:
                /* Solid dirt - space with colored background */
                display_char = ' ';
                cell_color = layer_color;
                break;

            case TILE_EMPTY:
                /* Tunnel - empty space, black background */
                display_char = ' ';
                cell_color = empty_color;
                break;

            case TILE_SKY:
                /* Sky - space with black background (empty cells) */
                display_char = ' ';
                cell_color.fg = COLOR_WHITE;
                cell_color.bg = COLOR_BLACK;
                break;

            case TILE_WALL:
                /* Wall - space with layer background color */
                display_char = ' ';
                cell_color = layer_color;
                break;

            case TILE_GEM:
                /* Collectible gem */
                display_char = '$';
                cell_color.fg = COLOR_YELLOW;
                cell_color.bg = COLOR_BLACK;
                break;

            case TILE_BONUS:
                /* Bonus item (100 points) */
                display_char = '+';
                cell_color.fg = COLOR_YELLOW;
                cell_color.bg = layer_color.bg; /* Usar color de capa de tierra */
                break;

            case TILE_BORDER:
                /* Bottom border - gray # on black */
                display_char = '#';
                cell_color.fg = COLOR_DARK_GRAY;
                cell_color.bg = COLOR_BLACK;
                break;

            default:
                display_char = ' ';
                cell_color = empty_color;
                break;
            }

            render_set_cell(x, y, display_char, cell_color);
        }
    }
}

void render_entities(GameLogicState *state) {
    if (!state) return;

    /* Render order: rocks -> enemies -> pump -> player */
    /* (player is drawn last to always be visible) */

    render_rocks(state->rocks, state->rock_count);
    render_enemies(state->enemies, state->enemy_count);

    /* Draw pump before player */
    if (state->player.is_pumping) {
        render_pump(&state->player);
    }

    render_player(&state->player);
}

void render_entity(Entity *entity, char display_char, Color color) {
    if (!entity || !entity->active) {
        return;
    }
    render_set_cell(entity->pos.x, entity->pos.y, display_char, color);
}

void render_entity_at(int x, int y, char display_char, Color color) {
    render_set_cell(x, y, display_char, color);
}

void render_player(Player *player) {
    if (!player || player->state == PLAYER_DEAD) {
        return;
    }

    Color player_color;
    player_color.fg = COLOR_YELLOW; /* Player is always yellow */
    /* Task 3: Player background is always black */
    player_color.bg = COLOR_BLACK;

    /* Select character based on facing direction */
    char display_char;
    switch (player->facing_dir) {
    case DIR_UP:
        display_char = CHAR_PLAYER_UP;
        break;
    case DIR_DOWN:
        display_char = CHAR_PLAYER_DOWN;
        break;
    case DIR_LEFT:
        display_char = CHAR_PLAYER_LEFT;
        break;
    case DIR_RIGHT:
    default:
        display_char = CHAR_PLAYER_RIGHT;
        break;
    }

    /* Modify appearance based on state */
    switch (player->state) {
    case PLAYER_DIGGING:
        /* Keep directional char */
        break;

    case PLAYER_PUMPING:
    case PLAYER_ATTACKING:
        /* Indicate attacking with brighter color */
        player_color.fg = COLOR_WHITE;
        break;

    default:
        break;
    }

    render_entity(&player->base, display_char, player_color);

    /* Render attack if player is attacking */
    if (player->is_attacking && player->attack_timer > 0) {
        render_player_attack(player);
    }
}

void render_player_attack(Player *player) {
    if (!player) return;

    Color attack_color;
    attack_color.fg = COLOR_WHITE;
    /* Task 3: Attack background is always black */
    attack_color.bg = COLOR_BLACK;

    int px = player->base.pos.x;
    int py = player->base.pos.y;
    int range;
    int dx = 0, dy = 0;
    char attack_char;

    /* Determine attack direction and range */
    switch (player->facing_dir) {
    case DIR_UP:
        range = ATTACK_RANGE_V;
        dy = -1;
        attack_char = CHAR_ATTACK_V;
        break;
    case DIR_DOWN:
        range = ATTACK_RANGE_V;
        dy = 1;
        attack_char = CHAR_ATTACK_V;
        break;
    case DIR_LEFT:
        range = ATTACK_RANGE_H;
        dx = -1;
        attack_char = CHAR_ATTACK_H;
        break;
    case DIR_RIGHT:
        range = ATTACK_RANGE_H;
        dx = 1;
        attack_char = CHAR_ATTACK_H;
        break;
    default:
        return;
    }

    /* Render attack characters in range */
    for (int i = 1; i <= range; i++) {
        int ax = px + dx * i;
        int ay = py + dy * i;

        /* Check bounds */
        if (ax < 0 || ax >= SCREEN_WIDTH || ay < 0 || ay >= SCREEN_HEIGHT) {
            break;
        }

        /* Check if solid block stops attack */
        if (map_is_solid(ax, ay)) {
            break;
        }

        render_set_cell(ax, ay, attack_char, attack_color);
    }
}

void render_enemies(Enemy *enemies, int count) {
    if (!enemies) return;

    for (int i = 0; i < count; i++) {
        Enemy *enemy = &enemies[i];

        if (!enemy->base.active || enemy->state == ENEMY_DEAD) {
            continue;
        }

        Color enemy_color;
        enemy_color.bg = COLOR_BLACK; /* Default: Black background */

        char display_char;

        /* Determine character based on type */
        if (enemy->base.type == ENTITY_POOKA) {
            display_char = CHAR_POOKA;
            enemy_color.fg = COLOR_LIGHT_RED;
        } else if (enemy->base.type == ENTITY_FYGAR) {
            display_char = CHAR_FYGAR;
            /* Fygar is always green */
            enemy_color.fg = COLOR_GREEN;
        } else {
            display_char = '?';
            enemy_color.fg = COLOR_WHITE;
        }

        /* Modify appearance based on state */
        switch (enemy->state) {
        case ENEMY_INFLATING:
            /* Change character based on inflate level */
            switch (enemy->inflate_level) {
            case 1:
                display_char = CHAR_INFLATE_1;
                break;
            case 2:
                display_char = CHAR_INFLATE_2;
                break;
            case 3:
                display_char = CHAR_INFLATE_3;
                break;
            default:
                break;
            }
            enemy_color.fg = COLOR_LIGHT_MAGENTA;
            break;

        case ENEMY_GHOST:
            /* Task 4: Ghost mode - white enemy with earth color background */
            enemy_color.fg = COLOR_WHITE;
            enemy_color.bg = render_get_layer_color(enemy->base.pos.y).bg;
            break;

        case ENEMY_PARALYZED:
            /* Paralyzed enemies blink (alternate visibility) */
            if (enemy->paralyzed_timer % 2 == 0) {
                enemy_color.fg = COLOR_LIGHT_CYAN;
            } else {
                enemy_color.fg = COLOR_BLACK; /* Invisible on blink */
            }
            break;

        default:
            break;
        }

        render_entity(&enemy->base, display_char, enemy_color);

        /* Render Fygar fire if active */
        if (enemy->base.type == ENTITY_FYGAR && enemy->fire_active) {
            render_fire(enemy->base.pos.x, enemy->base.pos.y, enemy->base.dir,
                        enemy->fire_duration);
        }
    }
}

void render_rocks(Rock *rocks, int count) {
    if (!rocks) return;

    for (int i = 0; i < count; i++) {
        Rock *rock = &rocks[i];

        if (!rock->base.active) {
            continue;
        }

        Color rock_color;
        rock_color.fg = COLOR_DARK_GRAY; /* Gray rock character */

        /* Determine background color based on rock state */
        if (rock->state == ROCK_FALLING || rock->state == ROCK_BLINKING) {
            /* Falling or landed rocks have black background */
            rock_color.bg = COLOR_BLACK;
        } else {
            /* Stable/wobbling rocks match earth layer color */
            rock_color.bg = render_get_layer_color(rock->base.pos.y).bg;
        }

        char display_char = '#'; /* Rock displayed as # */

        /* Blinking animation when hitting earth */
        if (rock->state == ROCK_BLINKING) {
            if (rock->blink_timer % 2 == 0) {
                rock_color.fg = COLOR_WHITE; /* Blink to white */
            }
        } else if (rock->state == ROCK_WOBBLING) {
            /* Wobble animation - slightly different appearance */
            if (rock->wobble_timer % 2 == 0) {
                rock_color.fg = COLOR_LIGHT_GRAY;
            }
        }

        render_entity(&rock->base, display_char, rock_color);
    }
}

void render_pump(Player *player) {
    if (!player || !player->is_pumping || player->pump_length <= 0) {
        return;
    }

    Color pump_color;
    pump_color.fg = COLOR_CYAN;
    pump_color.bg = render_get_layer_color(player->base.pos.y).bg;

    int x = player->base.pos.x;
    int y = player->base.pos.y;

    /* Determine pump character and direction */
    char pump_char;
    int dx = 0, dy = 0;

    switch (player->pump_dir) {
    case DIR_UP:
        pump_char = '|';
        dy = -1;
        break;
    case DIR_DOWN:
        pump_char = '|';
        dy = 1;
        break;
    case DIR_LEFT:
        pump_char = '-';
        dx = -1;
        break;
    case DIR_RIGHT:
        pump_char = '-';
        dx = 1;
        break;
    default:
        return;
    }

    /* Draw the pump line */
    for (int i = 1; i <= player->pump_length; i++) {
        int px = x + (dx * i);
        int py = y + (dy * i);

        if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
            pump_color.bg = render_get_layer_color(py).bg;
            render_set_cell(px, py, pump_char, pump_color);
        }
    }

    /* Draw pump tip */
    int tip_x = x + (dx * player->pump_length);
    int tip_y = y + (dy * player->pump_length);
    if (tip_x >= 0 && tip_x < SCREEN_WIDTH && tip_y >= 0 && tip_y < SCREEN_HEIGHT) {
        pump_color.fg = COLOR_YELLOW;
        render_set_cell(tip_x, tip_y, '+', pump_color);
    }
}

void render_explosion(int x, int y) {
    Color explosion_color;
    explosion_color.fg = COLOR_YELLOW;
    explosion_color.bg = COLOR_RED;

    /* Simple explosion pattern */
    render_set_cell(x, y, '*', explosion_color);

    /* Particles around */
    if (x > 0) render_set_cell(x - 1, y, '+', explosion_color);
    if (x < SCREEN_WIDTH - 1) render_set_cell(x + 1, y, '+', explosion_color);
    if (y > 0) render_set_cell(x, y - 1, '+', explosion_color);
    if (y < SCREEN_HEIGHT - 1) render_set_cell(x, y + 1, '+', explosion_color);
}

void render_fire(int x, int y, int dir, int length) {
    (void)length; /* Not used - we always render full range */

    Color fire_color;
    fire_color.fg = COLOR_RED;
    fire_color.bg = COLOR_BLACK; /* Fire: red on black background */

    int dx = 0;
    switch (dir) {
    case DIR_LEFT:
        dx = -1;
        break;
    case DIR_RIGHT:
        dx = 1;
        break;
    default:
        return; /* Fygar only fires horizontally */
    }

    /* Draw fire for the full range (2 cells) */
    for (int i = 1; i <= FYGAR_FIRE_RANGE; i++) {
        int fx = x + (dx * i);

        if (fx >= 0 && fx < SCREEN_WIDTH) {
            render_set_cell(fx, y, '*', fire_color);
        }
    }
}

void render_dig_particles(int x, int y) {
    /* Simple particle effect when digging */
    Color particle_color;
    particle_color.fg = COLOR_BROWN;
    particle_color.bg = render_get_layer_color(y).bg;

    /* Show briefly (managed by game timer in practice) */
    render_set_cell(x, y, '.', particle_color);
}
