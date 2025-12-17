#ifndef __GAME_RENDER_H__
#define __GAME_RENDER_H__

#include <game_types.h>
#include <game_config.h>

/**
 * @file game_render.h
 * @brief Double-buffered rendering system for ZeOS Miner game
 */

/* ============================================================================
 *                            SCREEN CONSTANTS
 * ============================================================================ */

/* Use game_config.h definitions where available, provide defaults otherwise */
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH        80
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT       25
#endif

/* VGA buffer size (80*25*2 bytes for char+attr pairs) */
#ifndef SCREEN_SIZE
#define SCREEN_SIZE         4000
#endif

#define BUFFER_SIZE         2000        /* 80*25 chars only */

/* Screen areas - use game_config.h names if available */
#ifndef STATUS_TOP_ROW
#define STATUS_TOP_ROW      ROW_STATUS_TOP
#endif
#ifndef SKY_START_ROW
#define SKY_START_ROW       ROW_SKY_START
#endif
#ifndef SKY_END_ROW
#define SKY_END_ROW         ROW_SKY_END
#endif
#ifndef GROUND_START_ROW
#define GROUND_START_ROW    ROW_GROUND_START
#endif
#ifndef GROUND_END_ROW
#define GROUND_END_ROW      ROW_GROUND_END
#endif
#ifndef STATUS_BOTTOM_ROW
#define STATUS_BOTTOM_ROW   ROW_STATUS_BOTTOM
#endif

/* Layer definitions for Dig Dug scoring */
#ifndef LAYER1_START
#define LAYER1_START        LAYER_1_START
#endif
#ifndef LAYER1_END
#define LAYER1_END          LAYER_1_END
#endif
#ifndef LAYER2_START
#define LAYER2_START        LAYER_2_START
#endif
#ifndef LAYER2_END
#define LAYER2_END          LAYER_2_END
#endif
#ifndef LAYER3_START
#define LAYER3_START        LAYER_3_START
#endif
#ifndef LAYER3_END
#define LAYER3_END          LAYER_3_END
#endif
#ifndef LAYER4_START
#define LAYER4_START        LAYER_4_START
#endif
#ifndef LAYER4_END
#define LAYER4_END          LAYER_4_END
#endif

/* ============================================================================
 *                            COLOR DEFINITIONS
 * ============================================================================ */

/* VGA Color constants */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GRAY    7
#define COLOR_DARK_GRAY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

/* Game-specific colors - use ifndef to avoid conflicts with game_config.h */
#ifndef COLOR_SKY_BG
#define COLOR_SKY_BG        COLOR_LIGHT_BLUE
#endif
#ifndef COLOR_LAYER1_BG
#define COLOR_LAYER1_BG     COLOR_BROWN
#endif
#ifndef COLOR_LAYER2_BG
#define COLOR_LAYER2_BG     COLOR_RED
#endif
#ifndef COLOR_LAYER3_BG
#define COLOR_LAYER3_BG     COLOR_MAGENTA
#endif
#ifndef COLOR_LAYER4_BG
#define COLOR_LAYER4_BG     COLOR_DARK_GRAY
#endif
#ifndef COLOR_STATUS_BG
#define COLOR_STATUS_BG     COLOR_BLACK
#endif

/* ============================================================================
 *                            STRUCTURES
 * ============================================================================ */

/**
 * @brief Color pair for foreground and background.
 */
typedef struct {
    unsigned char fg;   /**< Foreground color (0-15) */
    unsigned char bg;   /**< Background color (0-15) */
} Color;

/**
 * @brief Screen cell containing character and color.
 */
typedef struct {
    char character;     /**< Character to display */
    Color color;        /**< Colors for this cell */
} ScreenCell;

/**
 * @brief Double buffer structure for flicker-free rendering.
 */
typedef struct {
    ScreenCell cells[SCREEN_HEIGHT][SCREEN_WIDTH];  /**< Screen cells grid */
    int dirty;                                      /**< Flag indicating changes */
} ScreenBuffer;

/* Global buffers */
extern ScreenBuffer g_front_buffer;    /* Currently displayed buffer */
extern ScreenBuffer g_back_buffer;     /* Buffer being drawn to */

/* ============================================================================
 *                            INITIALIZATION
 * ============================================================================ */

/**
 * @brief Initialize the rendering system.
 * 
 * Clears both front and back buffers and sets up default colors.
 */
void render_init(void);

/**
 * @brief Cleanup rendering resources.
 */
void render_cleanup(void);

/* ============================================================================
 *                            BUFFER OPERATIONS
 * ============================================================================ */

/**
 * @brief Clear the back buffer with appropriate background colors.
 * 
 * Each row gets its layer-appropriate background color.
 */
void render_clear(void);

/**
 * @brief Set a single cell in the back buffer.
 * @param x Column position (0-79)
 * @param y Row position (0-24)
 * @param c Character to place
 * @param color Color for the cell
 */
void render_set_cell(int x, int y, char c, Color color);

/**
 * @brief Put a character at position with current default color.
 * @param x Column position
 * @param y Row position
 * @param c Character to place
 */
void render_put_char(int x, int y, char c);

/**
 * @brief Draw a string starting at position.
 * @param x Starting column
 * @param y Row position
 * @param str String to draw
 */
void render_put_string(int x, int y, const char* str);

/**
 * @brief Draw a string with specific color.
 * @param x Starting column
 * @param y Row position
 * @param str String to draw
 * @param color Color for the string
 */
void render_put_string_colored(int x, int y, const char* str, Color color);

/* ============================================================================
 *                            DRAWING PRIMITIVES
 * ============================================================================ */

/**
 * @brief Fill a rectangular area with character and color.
 * @param x Left coordinate
 * @param y Top coordinate
 * @param w Width in cells
 * @param h Height in cells
 * @param c Character to fill with
 * @param color Color for the filled area
 */
void render_fill_rect(int x, int y, int w, int h, char c, Color color);

/**
 * @brief Draw a horizontal line.
 * @param x Starting column
 * @param y Row position
 * @param length Length in cells
 * @param c Character for the line
 * @param color Color for the line
 */
void render_draw_horizontal_line(int x, int y, int length, char c, Color color);

/**
 * @brief Draw a vertical line.
 * @param x Column position
 * @param y Starting row
 * @param length Length in cells
 * @param c Character for the line
 * @param color Color for the line
 */
void render_draw_vertical_line(int x, int y, int length, char c, Color color);

/* ============================================================================
 *                            COLOR MANAGEMENT
 * ============================================================================ */

/**
 * @brief Set the default color for subsequent operations.
 * @param color New default color
 */
void render_set_default_color(Color color);

/**
 * @brief Get the default background color for a specific row.
 * @param y Row position
 * @return Color appropriate for that layer
 */
Color render_get_layer_color(int y);

/**
 * @brief Create a Color structure from fg/bg values.
 * @param fg Foreground color (0-15)
 * @param bg Background color (0-15)
 * @return Color structure
 */
Color render_make_color(unsigned char fg, unsigned char bg);

/* ============================================================================
 *                            PRESENTATION
 * ============================================================================ */

/**
 * @brief Swap front and back buffers (fast pointer swap).
 */
void render_swap_buffers(void);

/**
 * @brief Present the current front buffer to screen.
 * 
 * Uses ZeOS syscalls (gotoXY, set_color, write) to display the buffer.
 * Only updates cells that have changed since last frame.
 */
void render_present(void);

/**
 * @brief Force a complete screen refresh.
 * 
 * Updates every cell regardless of changes. Use sparingly.
 */
void render_present_full(void);

/* ============================================================================
 *                            UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Render a number with specific digit count.
 * @param x Column position
 * @param y Row position
 * @param number Number to display
 * @param digits Number of digits to show (pad with spaces)
 */
void render_number(int x, int y, int number, int digits);

/**
 * @brief Render a number with zero-padding.
 * @param x Column position
 * @param y Row position
 * @param number Number to display
 * @param digits Number of digits to show (pad with zeros)
 */
void render_number_padded(int x, int y, int number, int digits);

/**
 * @brief Render a number with custom padding character.
 * @param x Column position
 * @param y Row position
 * @param number Number to display
 * @param digits Number of digits to show
 * @param pad_char Character to use for padding
 */
void render_number_padded_char(int x, int y, int number, int digits, char pad_char);

/**
 * @brief Check if coordinates are within screen bounds.
 * @param x Column position
 * @param y Row position
 * @return 1 if valid, 0 if out of bounds
 */
int render_is_valid_pos(int x, int y);

/**
 * @brief Get current back buffer cell (read-only).
 * @param x Column position
 * @param y Row position
 * @return Pointer to cell, or NULL if out of bounds
 */
const ScreenCell* render_get_cell(int x, int y);

/**
 * @brief Clear a specific buffer with layer-appropriate colors.
 * @param buffer Pointer to the buffer to clear
 */
void render_clear_buffer(ScreenBuffer* buffer);

/**
 * @brief Convert internal buffer to VGA format and write to screen.
 * 
 * Uses write(10, ...) to send the buffer directly to video memory.
 * This is more efficient than cell-by-cell rendering.
 */
void render_present_buffer(void);

/**
 * @brief Copy a cell's character and color to a VGA-format buffer.
 * @param vga_buffer Target VGA buffer (2 bytes per character)
 * @param cell Source screen cell
 * @param offset Byte offset in VGA buffer
 */
void render_cell_to_vga(char* vga_buffer, const ScreenCell* cell, int offset);

#endif /* __GAME_RENDER_H__ */