// display.h - updated font declaration
#ifndef DISPLAY_H
#define DISPLAY_H

#include "SSD1306.h"
#include "constants.h"
#include "entities.h"
#include "render_state.h"
#include "objects.h"

//=============================================================================
// GLOBAL DISPLAY VARIABLES
//=============================================================================

extern Adafruit_SSD1306<SCREEN_WIDTH, SCREEN_HEIGHT> display;
extern uint8_t* display_buffer;                 // OLED framebuffer pointer
extern uint8_t zbuffer[ZBUFFER_SIZE];           // Z-buffer for sprite sorting
extern uint8_t hud_effect;                      // Current HUD effect (0=none, 1=heal, 2=ammo)
extern uint8_t lamp_is_dark;                    // Lamp state flag

//=============================================================================
// LOOKUP TABLES
//=============================================================================

extern const uint16_t rowDistTable[32];         // Row distance table for floor/ceiling
extern const uint8_t font5x5[180];              // 5x5 pixel font data (36 chars: 0-9, A-Z)

//=============================================================================
// DISPLAY FUNCTIONS
//=============================================================================

// Setup and initialize the OLED display
void setupDisplay(void);

// Draw a single pixel at (x,y) with the specified color
void drawPixel(int8_t x, int8_t y, bool color);

// Draw a scaled sprite with optional mirroring and inversion
// Parameters:
//   x,y - screen position (top-left corner)
//   bitmap, mask - sprite data (PROGMEM)
//   src_w, src_h - source sprite dimensions
//   dst_w, dst_h - destination (scaled) dimensions
//   mirror - 0=normal, 1=horizontal flip
//   invert - 0=normal, 1=color inversion
void drawSprite(int x, int y, const uint8_t bitmap[], const uint8_t mask[],
                uint8_t src_w, uint8_t src_h, int dst_w, int dst_h,
                uint8_t mirror, uint8_t invert);

// Draw 5x5 pixel text at position (x,y)
// If num != 0xFFFF, draws number instead of string pointer
void drawText5x5(uint8_t x, uint8_t y, const char* ptr, uint16_t num);

// Draw the HUD (health and ammo circular gauges)
void drawHud(uint8_t health, uint8_t current_weapon, const uint8_t ammo[]);

// Display a fullscreen message with a 500ms delay
void drawFullscreenMessage(const char* text);

// Draw a textured vertical wall line (raycasting column renderer)
void drawTexturedVLine(uint8_t x, uint8_t start_y, uint8_t end_y,
                       const uint8_t level[], uint8_t map_x, uint8_t map_y);

// Generate procedural wall texture pattern
// Returns 1 for wall pixel, 0 for transparent
uint8_t applyWallPattern(uint8_t tex_x, uint8_t tex_y, uint8_t wall_type, uint8_t map_x, uint8_t map_y);

// Draw circular gauge for health or ammo display
void drawCircleGauge(uint8_t value, uint8_t max_val, uint8_t left, uint8_t is_hp);

extern uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y);

#endif