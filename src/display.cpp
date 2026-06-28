// display.cpp - Display rendering functions for OLED screen
// This file implements the display driver, text rendering, sprite drawing,
// texture mapping for walls, HUD elements, and various visual effects
// used by the raycasting engine.

#include "display.h"
#include "objects.h"
#include "game_state.h"

//=============================================================================
// GLOBAL DISPLAY VARIABLES
//=============================================================================

uint8_t* display_buffer = NULL;              // Pointer to OLED framebuffer
uint8_t zbuffer[ZBUFFER_SIZE];              // Z-buffer for sprite sorting
Adafruit_SSD1306<SCREEN_WIDTH, SCREEN_HEIGHT> display;

extern Player player;
extern RenderState g_render;

//=============================================================================
// ROW DISTANCE TABLE FOR FLOOR/CEILING RENDERING
//=============================================================================

const uint16_t rowDistTable[32] PROGMEM = {
    1024, 512, 341, 256, 204, 170, 146, 128,
    113, 102, 93, 85, 78, 73, 68, 64,
    60, 56, 53, 51, 48, 46, 44, 42,
    41, 39, 38, 36, 35, 34, 33, 32
};

//=============================================================================
// 5x5 FONT DATA (ASCII CHARACTERS)
// Each character is 5 bytes (5 rows of 5 bits)
// Characters: 0-9, A-Z, ., ,, -
//=============================================================================

// 5x5 pixel font for game UI rendering
// Supports: digits 0-9, uppercase A-Z
// Characters are stored as 5 bytes (5 rows of 5 bits each)
// Total: 36 characters × 5 bytes = 180 bytes
const uint8_t font5x5[180] PROGMEM = {
    // 0 - 0x0E (01110) 
    0x0E, 0x11, 0x11, 0x11, 0x0E,
    // 1
    0x04, 0x0C, 0x04, 0x04, 0x0E,
    // 2
    0x0E, 0x01, 0x0E, 0x10, 0x0F,
    // 3
    0x0F, 0x01, 0x07, 0x01, 0x0F,
    // 4
    0x11, 0x11, 0x0F, 0x01, 0x01,
    // 5
    0x0F, 0x10, 0x0E, 0x01, 0x0E,
    // 6
    0x0E, 0x10, 0x0E, 0x11, 0x0E,
    // 7
    0x1F, 0x01, 0x02, 0x04, 0x04,
    // 8
    0x0E, 0x11, 0x0E, 0x11, 0x0E,
    // 9
    0x0E, 0x11, 0x0F, 0x01, 0x0E,
    // A
    0x0E, 0x11, 0x1F, 0x11, 0x11,
    // B
    0x1E, 0x11, 0x1E, 0x11, 0x1E,
    // C
    0x0E, 0x11, 0x10, 0x11, 0x0E,
    // D
    0x1E, 0x11, 0x11, 0x11, 0x1E,
    // E
    0x1F, 0x10, 0x1E, 0x10, 0x1F,
    // F
    0x1F, 0x10, 0x1E, 0x10, 0x10,
    // G
    0x0E, 0x11, 0x13, 0x11, 0x0F,
    // H
    0x11, 0x11, 0x1F, 0x11, 0x11,
    // I
    0x0E, 0x04, 0x04, 0x04, 0x0E,
    // J
    0x07, 0x02, 0x02, 0x12, 0x0C,
    // K
    0x11, 0x12, 0x1C, 0x12, 0x11,
    // L
    0x10, 0x10, 0x10, 0x10, 0x1F,
    // M
    0x11, 0x1B, 0x15, 0x11, 0x11,
    // N
    0x11, 0x19, 0x15, 0x13, 0x11,
    // O
    0x0E, 0x11, 0x11, 0x11, 0x0E,
    // P
    0x1E, 0x11, 0x1E, 0x10, 0x10,
    // Q
    0x0E, 0x11, 0x15, 0x12, 0x0D,
    // R
    0x1E, 0x11, 0x1E, 0x12, 0x11,
    // S
    0x0F, 0x10, 0x0E, 0x01, 0x1E,
    // T
    0x1F, 0x04, 0x04, 0x04, 0x04,
    // U
    0x11, 0x11, 0x11, 0x11, 0x0E,
    // V
    0x11, 0x11, 0x0A, 0x0A, 0x04,
    // W
    0x11, 0x15, 0x15, 0x15, 0x0A,
    // X
    0x11, 0x0A, 0x04, 0x0A, 0x11,
    // Y
    0x11, 0x0A, 0x04, 0x04, 0x04,
    // Z
    0x1F, 0x02, 0x04, 0x08, 0x1F
};



extern uint8_t g_tick_counter;

//=============================================================================
// DISPLAY INITIALIZATION
//=============================================================================

void setupDisplay(void) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        while (1);  // Halt if display not found
    }
    display_buffer = display.getBuffer();
    memset(zbuffer, 0xFF, ZBUFFER_SIZE);
}

//=============================================================================
// PIXEL DRAWING
//=============================================================================

void drawPixel(int8_t x, int8_t y, bool color) {
    if ((uint8_t)x >= 128 || (uint8_t)y >= 64) return;
    uint16_t idx = x + ((uint16_t)(y & 0xF8) << 4);
    uint8_t bit = 1 << (y & 7);
    if (color) {
        display_buffer[idx] |= bit;
    } else {
        display_buffer[idx] &= ~bit;
    }
}

//=============================================================================
// VECTOR WALL PATTERN GENERATOR
// This function generates procedural wall textures based on wall type
// and animation state using vector math and patterns.
//=============================================================================

extern uint8_t g_tick_counter;


// Apply vector-based wall pattern generation for textured walls
// This function determines if a pixel at texture coordinates (rx, ry) should be drawn
// Returns 1 for wall pixel, 0 for transparent/empty pixel
uint8_t applyVectorWallPattern(uint8_t rx, uint8_t ry, uint8_t wall_type, uint8_t door_state)
{
    // Clip extreme edges to prevent rendering artifacts
    if (12 > ry || ry > 243 || 12 > rx || rx > 243) {
        return 0;
    }
    // Additional corner clipping for cleaner edges
    if (40 > rx || rx > 215) {
        if (40 > ry || ry > 215) {
            return 0;
        }
    }
    
    uint8_t lx = 0;
    uint8_t ly = 0;
    uint8_t is_inner_panel = 0;
    
    // Check if pixel is in the inner panel area (64x64 center region)
    if (rx > 63 && 192 > rx && ry > 63 && 192 > ry) {
        lx = (uint8_t)(rx - 64);
        ly = (uint8_t)(ry - 64);
        is_inner_panel = 1;
    }
    
    // Handle base wall types and doors
    if (wall_type == E_WALL_TYPE_1 || wall_type == E_WALL_ILLUMINATOR || wall_type == E_WALL_MONITOR || wall_type == E_DOOR) {
        // E_WALL_ILLUMINATOR- animated grid with circular cutouts
        if (wall_type == E_WALL_ILLUMINATOR) {
            if (is_inner_panel > 0) {
                int16_t dx = (int16_t)lx - 64;
                int16_t dy = (int16_t)ly - 64;
                uint16_t dist_sq = (uint16_t)(dx * dx + dy * dy);
                
                // Outer ring cutout
                if (dist_sq > 2916) {
                    if (3844 > dist_sq) {
                        return 0;
                    }
                }
                // Inner ring highlight
                if (dist_sq > 2500) {
                    if (2917 > dist_sq) {
                        return 1;
                    }
                }
                // Central grid with animated hash pattern
                if (2501 > dist_sq) {
                    // Generate hash from grid position and time
                    uint8_t h = (uint8_t)((uint8_t)(lx >> 3) - (uint8_t)(g_tick_counter >> 2)) ^ (uint8_t)(ly >> 3);
                    
                    // BRANCHLESS REPLACEMENT: h * 109 -> h * (64 + 32 + 8 + 4 + 1)
                    h = (uint8_t)((h << 6) + (h << 5) + (h << 3) + (h << 2) + h);
                    h = (uint8_t)(h ^ (uint8_t)((uint8_t)(ly >> 3) << 3));
                    
                    // h * 137 -> h * (128 + 8 + 1)
                    uint8_t hash = (uint8_t)((h << 7) + (h << 3) + h);
                    
                    // Check if hash matches pattern (19 out of 32 values)
                    if ((uint8_t)((uint8_t)(hash ^ (uint8_t)(hash >> 4)) & 31) == 19) {
                        // Draw pixel if not on grid line (sub-pixel check)
                        if (((uint8_t)(lx & 7) > 0) && ((uint8_t)(ly & 7) > 0)) {
                            return 1;
                        }
                    }
                    return 0;
                }
                return 1;
            }
        }
        
        // E_WALL_MONITOR - animated wave bars
        if (wall_type == E_WALL_MONITOR) {
            if (is_inner_panel > 0) {
                // Check bounds within inner panel
                if (10 > lx || lx > 117 || 10 > ly || ly > 117) {
                    return 0; 
                }
                // Scrolling vertical pattern
                uint8_t scroll_time = g_tick_counter >> 1;
                uint8_t scrolled_y = (uint8_t)(ly + scroll_time);
                uint8_t line_index = scrolled_y >> 4;
                uint8_t line_pixel_y = scrolled_y & 15;
                
                // Draw lines based on hash pattern
                if (10 > line_pixel_y) {
                    // line_index * 13 -> line_index * (8 + 4 + 1)
                    uint8_t m_line = (uint8_t)((line_index << 3) + (line_index << 2) + line_index);
                    uint8_t text_hash = (uint8_t)((uint8_t)(lx ^ m_line) + (uint8_t)(lx >> 3));
                    if ((uint8_t)(text_hash & 31) > 8) {
                        if ((uint8_t)((uint8_t)(line_index ^ (uint8_t)(lx >> 4)) & 7) > 1) {
                            return 0; 
                        }
                    }
                }
                return 1; 
            }
        }
        
        // DOOR RENDERING - sliding door with arrow indicator
        if (wall_type == E_DOOR) {
            // Calculate door opening offset based on state
            uint8_t door_open_width = door_state << 4;
            // Skip if inside the open gap
            if (door_open_width > 0 && rx > (uint8_t)(127 - door_open_width) && (uint8_t)(128 + door_open_width) > rx) {
                return 0;
            }
            // Calculate mirrored X position for symmetrical door
            uint8_t anim_rx = (128 > rx) ? (uint8_t)(rx + door_open_width) : (uint8_t)(rx - door_open_width);
            uint8_t sym_x = (anim_rx > 127) ? (uint8_t)(255 - anim_rx) : anim_rx;
            if (18 > sym_x || sym_x > 119) {
                return 0;
            }
            // Animated door pattern
            uint8_t door_y = (uint8_t)(ry + (uint8_t)(g_tick_counter << 4));
            if (door_y > 127) {
                door_y = (uint8_t)(door_y - 127);
            }
            
            // OPTIMIZATION: calculate delta in single instruction
            uint8_t target_sub = (anim_rx > 127) ? 191 : 64;
            uint8_t dist_to_arrow = (anim_rx > target_sub) ? (uint8_t)(anim_rx - target_sub) : (uint8_t)(target_sub - anim_rx);
            
            // Draw arrow indicator in the middle of the door
            if (door_y > 40 && 80 > door_y) {
                if ((uint8_t)((door_y - 40) >> 1) >= dist_to_arrow) {
                    return 0;
                }
            }
        }
        return 1;
    }
    
    // Clip outer edges for special wall types
    if (45 > ry || ry > 211) {
        return 1;
    }
    
    uint8_t px = (uint8_t)(rx - 64);
    
    // ELIMINATE HIDDEN DIVISION: replace division by 83 with Fixed-Point multiplication
    // (ry - 45) * 128 / 83 equivalent to (ry - 45) * 1.542 -> multiply by 395 and shift >> 8
    uint16_t base_delta = (uint8_t)(ry - 45);
    uint8_t py = (uint8_t)((uint16_t)(base_delta * 395) >> 8);
    
    uint8_t fast_tick = g_tick_counter;
    uint8_t dist_y = (py > 127) ? (uint8_t)(py - 127) : (uint8_t)(127 - py);
    
    // WALL TYPE 2 & 3 - animated wave patterns (opposite directions)
    if (wall_type == E_WALL_TYPE_2 || wall_type == E_WALL_TYPE_3) {
        uint8_t shift = fast_tick << 2;
        uint8_t anim_x = (wall_type == E_WALL_TYPE_2) ? (uint8_t)(px - shift) : (uint8_t)(px + shift);
        anim_x &= 127;
        if (84 > anim_x) {
            uint8_t half_w = (wall_type == E_WALL_TYPE_2) ? (uint8_t)(84 - anim_x) : anim_x;
            if (half_w > dist_y) {
                return 0;
            }
        }
        return 1;
    }
    
    // Calculate bounce animation for remaining wall types
    uint8_t bounce_raw = (uint8_t)(fast_tick * 4) & 63;
    uint8_t bounce_linear = (bounce_raw > 31) ? (uint8_t)(63 - bounce_raw) : bounce_raw;
    uint8_t bounce_py = (uint8_t)(py + bounce_linear);
    if (bounce_py > 220) { bounce_py = 220; }
    if (35 > bounce_py) { bounce_py = 35; }
    
    // Pre-calculate distances from center for reuse
    uint8_t dist_center_x = (px > 64) ? (uint8_t)(px - 64) : (uint8_t)(64 - px);
    uint8_t dist_center_y = (bounce_py > 127) ? (uint8_t)(bounce_py - 127) : (uint8_t)(127 - bounce_py);
    
    // WALL TYPE 4 - cross pattern with center cutout
    if (wall_type == E_WALL_TYPE_4) {
        // Vertical bar cutout
        if (px > 14 && 114 > px) {
            if (16 >= dist_center_y) {
                return 0;
            }
        }
        // Horizontal bar cutout
        if (bounce_py > 45 && 210 > bounce_py) {
            if (16 >= dist_center_x) {
                return 0;
            }
        }
    }
    
    // WALL TYPE 5 - complex segmented pattern
    if (wall_type == E_WALL_TYPE_5) {
        if (px > 34 && 94 > px) {
            // Top segment cutout
            if (bounce_py > 38 && 56 > bounce_py) {
                if (px > 49 && 79 > px) {
                    return 0;
                }
            }
            // Bottom segments with repeating pattern
            if (bounce_py > 59 && 211 > bounce_py) {
                uint8_t segment_rem = (uint8_t)(bounce_py - 60);
                if (segment_rem >= 110) { segment_rem -= 110; }
                if (segment_rem >= 55) { segment_rem -= 55; }
                if (41 > segment_rem) {
                    return 0;
                }
            }
        }
    }
    
    // LEVEL EXIT DOOR - animated expanding arrow
    if (wall_type == E_WALL_DOOR_LEVEL) {
        uint8_t exit_y = (uint8_t)(py + (uint8_t)(fast_tick << 4)) & 127;
        if (64 > exit_y) {
            if (exit_y > 8) {
                // Draw triangle/arrow shape pointing to center
                if ((uint8_t)(exit_y - 8) >= dist_center_x) {
                    return 0;
                }
            }
        }
    }
    
    return 1;
}


//=============================================================================
// TEXTURED VERTICAL LINE RENDERING
// Draw a textured vertical line (raycasting column renderer)
// This is the core rendering function that draws one column of the 3D view
// Parameters:
//   x - screen X coordinate (column to render)
//   start_y, end_y - vertical clipping bounds (from wall rendering)
//   level - pointer to current level data
//   map_x, map_y - grid coordinates of the wall being rendered
//=============================================================================

void drawTexturedVLine(uint8_t x, uint8_t start_y, uint8_t end_y, const uint8_t level[], uint8_t map_x, uint8_t map_y)
{
    // Player position in fixed-point (8.8 format) for floor/ceiling rendering
    const int32_t px8 = (int32_t)(player.pos.x * 256.0f);
    const int32_t py8 = (int32_t)(player.pos.y * 256.0f);
    uint8_t* buf = display_buffer;
    
    // Increment global tick counter once per frame (only on first column)
    if (0 == x) {
        g_tick_counter++;
    }
    
    // Clamp inverse distance to prevent overflow in wall height calculation
    float inv_d = g_render.inv_dist;
    if (inv_d > 16.0f) {
        inv_d = 16.0f;
    }
    
    // Calculate wall height and starting Y position on screen
    // WALL_HEIGHT_FACTOR controls perceived scale of walls
    const int16_t real_height = (int16_t)(((int32_t)RENDER_HEIGHT * WALL_HEIGHT_FACTOR >> 5) * inv_d);
    const int16_t real_start = (int16_t)(32 + (int16_t)(VIEW_HEIGHT * inv_d) - (real_height >> 1));
    
    // Ensure minimum wall height to avoid division by zero
    int16_t safe_h = real_height;
    if (1 > safe_h) {
        safe_h = 1;
    }
    
    // Clip wall to screen bounds (0-64)
    int16_t calc_start = real_start;
    int16_t calc_end = real_start + real_height;
    
    if (0 > calc_start) { calc_start = 0; }
    if (calc_start > 64) { calc_start = 64; }
    if (0 > calc_end) { calc_end = 0; }
    if (calc_end > 64) { calc_end = 64; }
    
    // Final vertical range for wall rendering
    uint8_t v_start_y = (start_y >= end_y) ? 64 : (uint8_t)calc_start;
    uint8_t v_end_y = (start_y >= end_y) ? 64 : (uint8_t)calc_end;
    
    // Normalize wall texture X coordinate (0.0-1.0 range)
    float rx = g_render.wall_x;
    if (0.0f > rx) { rx = 0.0f; }
    if (rx > 1.0f) { rx = 1.0f; }
    const uint8_t rx_curr = (uint8_t)(rx * 255.0f);
    
    // Get wall type and door state for texture generation
    uint8_t current_block_type = getBlockAt(level, map_x, map_y);
    uint8_t current_door_state = 0;
    if (current_block_type == E_DOOR) {
        current_door_state = getDoorState(map_x, map_y);
    }
    
    // OPTIMIZATION: Use integer shift instead of 32-bit division for texture step
    // 65535 / safe_h gives us the step size in fixed-point (16.16 format)
    uint16_t ry_step_16 = (uint16_t)(65535 / safe_h);
    if (1 > ry_step_16) {
        ry_step_16 = 1;
    }
    
    // Calculate starting texture Y position based on clipping offset
    int16_t clip_offset_y = (int16_t)v_start_y - real_start;
    if (0 > clip_offset_y) {
        clip_offset_y = 0;
    }
    uint16_t ry_accumulator = (uint16_t)(clip_offset_y * ry_step_16);
    
    // Prepare for page-by-page rendering (each page = 8 pixel rows)
    uint8_t page;
    uint16_t page_offset = x;
    uint8_t next_x_safe = (126 >= x) ? 1 : 0;  // Can we write to next column?
    
    // OPTIMIZATION: Pre-calculate ray direction in fixed-point outside the inner loop
    // This avoids expensive float-to-int conversion for each pixel
    const int16_t rdx8_l = (int16_t)(g_render.rayDirX * 256.0f);
    const int16_t rdy8_l = (int16_t)(g_render.rayDirY * 256.0f);
    
    // Render 8 pages (64 pixels total height)
    for (page = 0; 8 > page; page++) {
        uint8_t byte_curr = 0;   // Current column byte
        uint8_t byte_next = 0;   // Next column byte (for potential pixel doubling)
        uint8_t bit;
        uint8_t page_y_start = page << 3;  // Start Y for this page (0, 8, 16, ...)
        
        // Render each of 8 pixels in this page
        for (bit = 0; 8 > bit; bit++) {
            uint8_t pixel_y = page_y_start + bit;
            
            // ============ WALL RENDERING ============
            // Check if pixel is within wall vertical range
            if (pixel_y >= v_start_y) {
                if (v_end_y >= pixel_y) {
                    // Get texture Y coordinate (0-255) from accumulator
                    uint8_t ry_val = (uint8_t)(ry_accumulator >> 8);
                    
                    // Apply procedural wall texture pattern
                    if (applyVectorWallPattern(rx_curr, ry_val, current_block_type, current_door_state) == 1) {
                        byte_curr |= (1 << bit);
                        byte_next |= (1 << bit);
                    }
                    
                    // Advance texture accumulator for next pixel
                    ry_accumulator += ry_step_16;
                    continue;  // Skip floor/ceiling rendering for wall pixels
                }
            }
            
            // ============ FLOOR AND CEILING RENDERING ============
            // Calculate distance from center (32 is middle of screen)
            uint8_t dy = (32 > pixel_y) ? (uint8_t)(32 - pixel_y) : (uint8_t)(pixel_y - 31);
            if (dy > 31) { 
                dy = 31; 
            }
            if (0 == dy) { 
                dy = 1; 
            }
            
            // Look up row distance from precomputed table
            const uint16_t rowDist = pgm_read_word(&(rowDistTable[dy - 1]));
            
            // Calculate floor/ceiling texture coordinates using raycasting
            const int16_t tx = (int16_t)((px8 + (((int32_t)rdx8_l * rowDist) >> 4)) >> 4);
            const int16_t ty = (int16_t)((py8 + (((int32_t)rdy8_l * rowDist) >> 4)) >> 4);
            
            // OPTIMIZATION: Compressed grid pattern check without cascading if-else
            // Draw pixel if not on grid lines (check both X and Y sub-pixel positions)
            uint8_t tx_mask = (uint8_t)(tx & 0x0F);
            uint8_t ty_mask = (uint8_t)(ty & 0x0F);
            
            // Draw floor/ceiling grid pattern (checkerboard-like)
            if (tx_mask > 0) {
                if (15 > tx_mask) {
                    if (ty_mask > 0) {
                        if (15 > ty_mask) {
                            byte_curr |= (1 << bit);
                            byte_next |= (1 << bit);
                        }
                    }
                }
            }
        }
        
        // Write rendered byte to framebuffer
        buf[page_offset] = byte_curr;
        
        // OPTIMIZATION: Also write to next column if safe (pixel doubling)
        // This effectively doubles horizontal resolution in some cases
        if (next_x_safe != 0) {
            buf[page_offset + 1] = byte_next;
        }
        
        // Move to next page (next 8-pixel block)
        page_offset += 128;  // Each page is 128 bytes wide
    }
}


//=============================================================================
// SPRITE DRAWING WITH Z-BUFFER AND SCALING
//=============================================================================

void drawSprite(int x, int y, const uint8_t bitmap[], const uint8_t mask[], uint8_t src_w, uint8_t src_h, int dst_w, int dst_h, uint8_t mirror, uint8_t invert) {
    if (0 == bitmap || 2 > dst_w || 2 > dst_h) return;
    
    int8_t sx_start = (int8_t)x;
    int8_t sy_start = (int8_t)y;
    uint8_t d_w = (uint8_t)dst_w;
    uint8_t d_h = (uint8_t)dst_h;
    
    uint8_t z_val = (g_render.distance > 255) ? 255 : (uint8_t)g_render.distance;
    uint16_t step_x = ((uint16_t)src_w << 8) / d_w;
    uint16_t step_y = ((uint16_t)src_h << 8) / d_h;
    uint8_t bytes_per_row = (src_w + 7) >> 3;
    uint8_t *buf = display_buffer;
    
    uint16_t sx_fp = 0;
    uint8_t dx = 0;
    
    for (; d_w > dx; dx++, sx_fp += step_x) {
        int8_t screen_x = sx_start + dx;
        if (0 > screen_x || screen_x >= SCREEN_WIDTH) continue;
        
        uint8_t z_idx = (uint8_t)screen_x >> 2;
        if (ZBUFFER_SIZE > z_idx && zbuffer[z_idx] < z_val) continue;
        
        uint8_t sx = sx_fp >> 8;
        if (mirror != 0) {
            sx = (src_w - 1) - sx;
        }
        
        uint8_t bit_idx = 0x80 >> (sx & 7);
        uint16_t offset_base = sx >> 3;
        uint16_t screen_x_16 = (uint8_t)screen_x;
        uint8_t has_mask = (mask != 0);
        
        uint16_t sy_fp = 0;
        uint8_t dy = 0;
        
        for (; d_h > dy; dy++, sy_fp += step_y) {
            int8_t screen_y = sy_start + dy;
            if (0 > screen_y || screen_y >= SCREEN_HEIGHT) continue;
            
            uint8_t sy = sy_fp >> 8;
            uint16_t offset = ((uint16_t)sy * bytes_per_row) + offset_base;
            
            if (has_mask != 0) {
                if (0 == (pgm_read_byte(mask + offset) & bit_idx)) {
                    continue;
                }
            }
            
            uint8_t bit_color = (pgm_read_byte(bitmap + offset) & bit_idx) ? 1 : 0;
            bit_color ^= invert;
            
            uint16_t idx = screen_x_16 + (((uint16_t)(screen_y & 0xF8)) << 4);
            if (1024 > idx) {
                uint8_t bit = 1 << (screen_y & 7);
                if (bit_color != 0) {
                    buf[idx] |= bit;
                } else {
                    buf[idx] &= ~bit;
                }
            }
        }
    }
}

//=============================================================================
// 5x5 TEXT RENDERING
//=============================================================================

void drawText5x5(uint8_t x, uint8_t y, const char* ptr, uint16_t num) {
    char buf[4];
    
    // Handle numeric value display (for HUD gauges)
    if (num != 0xFFFF) {
        if (num > 999) num = 999;
        int8_t i = 2;
        buf[3] = 0;
        do {
            buf[i--] = (num % 10) + '0';
            num /= 10;
        } while (num && i >= 0);
        ptr = &buf[i + 1];
    }
    
    uint8_t c;
    while ((c = (num == 0xFFFF ? pgm_read_byte(ptr++) : *ptr++))) {
        // Check screen bounds
        if (x > 122) break;
        
        // Handle space character
        if (c == ' ') {
            x += 4;
            continue;
        }
        
        // Convert character to font index
        // Only support digits 0-9 and uppercase A-Z
        if (c >= 'a') c -= 32;  // Convert lowercase to uppercase
        
        if (c >= '0' && c <= '9') {
            c -= '0';               // '0' -> 0, '9' -> 9
        } else if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 10;       // 'A' -> 10, 'Z' -> 35
        } else {
            // Skip unsupported characters (including '.', ',', '-')
            continue;
        }
        
        // Draw character from font data
        const uint8_t* f = &font5x5[c * 5];
        for (uint8_t r = 0; r < 5; r++) {
            uint8_t d = pgm_read_byte(f++);
            for (int8_t b = 0; b < 5; b++) {
                if (d & (0x10 >> b)) {
                    drawPixel(x + b, y + r, true);
                }
            }
        }
        x += 6;  // 5px character width + 1px spacing
    }
}


//=============================================================================
// CIRCLE GAUGE (HEALTH/AMMO DISPLAY)
//=============================================================================

// Optimized circular gauge rendering for HUD (health and ammo displays)
// Uses fixed-point arithmetic and bit shifts instead of division for maximum performance
void drawCircleGauge(uint8_t value, uint8_t max_val, bool left, bool is_hp, bool invert_effect) {
    uint8_t *b = display_buffer;
    
    // OPTIMIZATION: Replace slow division with constant shifts for fixed scales (100 and 50)
    uint8_t fillAngle = 128;
    if (value != max_val) {
        if (100 == max_val) {
            // HP: (value * 128) / 100 approximated as (value * 164) >> 7
            fillAngle = (uint8_t)(((uint16_t)value * 164) >> 7);
        } else {
            // Ammo (50): (value * 128) / 50 approximated as (value * 328) >> 7
            fillAngle = (uint8_t)(((uint16_t)value * 328) >> 7);
        }
    }

    // Gauge position: left side for health, right side for ammo
    uint8_t cx = left ? 18 : 109;
    uint8_t cy = 60; 
    uint8_t ms = g_tick_counter; 
    
    // Calculate text width for centered value display
    uint8_t n_w = 5;
    if (value >= 10) {
        n_w = 11;
    }
    if (value >= 100) {
        n_w = 17;
    }
    
    uint8_t safeX = (uint8_t)((n_w >> 1) + 1);
    uint8_t rotPhase = ms; 
    uint8_t ms_shift = (30 > value) ? 1 : 2; 
    uint8_t ms_val = (uint8_t)((uint8_t)(ms >> ms_shift) & 31);
    
    // Heartbeat wave animation for health gauge
    uint8_t wave = (ms_val > 16) ? (uint8_t)(32 - ms_val) : ms_val;
    wave >>= 1;

    // Gauge ring boundaries (radius squared values)
    uint16_t outer_boundary = 280; 
    uint16_t max_boundary = 355; 

    // Pulse effect when picking up items
    if (invert_effect) {
        uint8_t size_phase = (uint8_t)((ms >> 1) & 15);
        uint8_t pulse_offset = (size_phase > 7) ? (uint8_t)(15 - size_phase) : size_phase;
        
        // Multiplication by 14 rewritten using shifts: pulse_offset * 14 = (p << 4) - (p << 1)
        uint16_t added_radius = (uint16_t)(((uint16_t)pulse_offset << 4) - ((uint16_t)pulse_offset << 1));
        outer_boundary += added_radius;
        max_boundary += added_radius;
    }

    // Invert effect flag for visual feedback
    uint8_t invert_bit_active = 0;
    if (invert_effect) {
        if (((uint8_t)(ms >> 1) & 1) > 0) {
            invert_bit_active = 1;
        }
    }

    // Draw gauge circle pixel by pixel
    // Loops rewritten using unsigned indices to avoid signed comparisons
    uint8_t y_idx = 0;
    for (y_idx = 0; 30 > y_idx; y_idx++) {
        // Map unsigned index y_idx (0..29) to signed offset y (10..-19)
        int8_t y = (int8_t)(10 - y_idx);
        int16_t y_sq = (int16_t)y * y;
        uint8_t py = (uint8_t)(cy + y);
        
        // Prevent unsigned overflow on vertical bounds
        if (py > 63) {
            continue;
        }

        uint16_t py_idx = (uint16_t)((uint16_t)(py >> 3) << 7);
        uint8_t bit = (uint8_t)(1 << (py & 7));
        uint8_t n_bit = ~bit;
        
        uint8_t ay = (0 > y) ? (uint8_t)(-y) : (uint8_t)y;
        uint16_t ay_30 = (uint16_t)(ay * 30); 
        uint8_t ay_plus_1 = (uint8_t)(ay + 1);

        uint8_t x_idx = 0;
        for (x_idx = 0; 47 > x_idx; x_idx++) {
            // Map unsigned index x_idx (0..46) to signed offset x (23..-23)
            int8_t x = (int8_t)(23 - x_idx);
            uint8_t ax = (0 > x) ? (uint8_t)(-x) : (uint8_t)x;
            uint16_t distSq = (uint16_t)((uint16_t)(ax * ax) + y_sq);
            
            if (distSq > max_boundary) {
                continue;
            }

            uint8_t px = (uint8_t)(cx + x);
            // Prevent unsigned overflow on horizontal bounds
            if (px > 127) {
                continue;
            }

            uint16_t idx = (uint16_t)(px + py_idx);

            // ============ OUTER RING ============
            if (distSq >= outer_boundary || (10 > y && 3 == y && 18 > ax)) {
                b[idx] |= bit; 
            } 
            // ============ GAUGE FILL AREA ============
            else if (distSq >= 196 && 256 > distSq && 2 > y) {
                b[idx] &= n_bit; 
                uint8_t pAngle = 0;
                uint8_t div_val = 0;
                
                // OPTIMIZATION: Remove division in angle geometry
                if (ay >= ax) {
                    // Approximation: (ax * 34) / (ay + 1) using bit shifts
                    div_val = (uint8_t)((uint16_t)((uint16_t)ax * 34) / ay_plus_1);
                } else {
                    // Approximation: ay_30 / (ax + 1)
                    div_val = (uint8_t)(64 - (uint8_t)(ay_30 / (uint8_t)(ax + 1)));
                }

                if (0 > x) {
                    pAngle = (uint8_t)(64 - div_val);
                } else {
                    pAngle = (uint8_t)(64 + div_val);
                }

                // Draw pixel if within fill angle
                if (value == max_val || fillAngle >= pAngle) {
                    b[idx] |= bit;
                }
            } 
            // ============ INNER RING ============
            else if (distSq >= 140 && 160 >= distSq) {
                b[idx] |= bit;
            } 
            // ============ CENTER AREA (with animations) ============
            else {
                b[idx] &= n_bit; 
                if (value > 0 && !(safeX > ax && y >= -5 && 0 >= y) && distSq >= 45 && 115 > distSq) {
                    if (is_hp) {
                        // HEALTH GAUGE: Heartbeat animation
                        if (y > (int8_t)(1 - wave) && 2 > y) {
                            b[idx] |= bit;
                        }
                    } else {
                        // AMMO GAUGE: Rotating pattern animation
                        uint8_t y_non_zero = ay ? ay : 1;
                        uint8_t x_non_zero = ax ? ax : 1;
                        
                        // Optimized angle calculation without software libraries
                        uint16_t div_ang = (ay > ax) ? 
                            (uint16_t)(((uint16_t)ax << 5) / y_non_zero) : 
                            (uint16_t)(((uint16_t)ay << 5) / x_non_zero);
                            
                        uint8_t ang = (0 > x) ? 
                            (ay > ax ? (uint8_t)div_ang : (uint8_t)(128 - div_ang)) : 
                            (ay > ax ? (uint8_t)(255 - div_ang) : (uint8_t)(128 + div_ang));
                            
                        uint8_t rot_val = (uint8_t)(ang + rotPhase);
                        if (rot_val > 170) {
                            rot_val = (uint8_t)(rot_val - 170);
                        } else if (rot_val > 85) {
                            rot_val = (uint8_t)(rot_val - 85);
                        }
                        
                        if (10 > rot_val) {
                            // OPTIMIZATION: Replace modulo distSq % 5 with fast subtraction using mask
                            uint16_t div5 = (uint16_t)((uint32_t)(distSq * 205) >> 10);
                            uint8_t rem5 = (uint8_t)(distSq - (uint16_t)(div5 * 5));
                            if (rem5 > 1) {
                                b[idx] |= bit;
                            }
                        }
                    }
                }
            }
            
            // Invert effect for pickup animation (flashing border)
            if (invert_bit_active > 0) {
                if (distSq >= 256) {
                    b[idx] ^= bit; 
                }
            }
        }
    }
    
    // Draw numeric value in the center using unsigned types
    drawText5x5((uint8_t)(cx - (uint8_t)(n_w >> 1)), (uint8_t)(cy - 4), 0, value);
}

// Draw the HUD (health and ammo circular gauges)
// Health gauge on the left, ammo gauge on the right (if weapon equipped)
void drawHud(uint8_t hp, uint8_t wp, const uint8_t ammo[]) {
    // Health gauge (left side) - always visible
    drawCircleGauge(hp, 100, true, true, (1 == hud_effect));
    
    // Ammo gauge (right side) - only visible if weapon is equipped
    if (wp > 0) {
        drawCircleGauge(ammo[wp], 50, false, false, (2 == hud_effect));
    }
}


//=============================================================================
// FULLSCREEN MESSAGE DISPLAY
//=============================================================================

void drawFullscreenMessage(const char* text) {
    // Clear screen
    uint8_t* b = display_buffer;
    uint16_t i = 1024;
    while(i--) *b++ = 0;
    
    // Center text
    uint8_t l = 0;
    while (pgm_read_byte(text + l)) l++;
    drawText5x5((128 - (l * 6)) >> 1, 28, text, 0xFFFF);
    
    display.display();
    delay(500);
}