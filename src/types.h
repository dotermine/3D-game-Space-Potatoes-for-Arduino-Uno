// types.h - Game type definitions and inline utilities
// This file defines all game object types, block types, and helper functions
// for type checking used throughout the game engine.

#ifndef _types_h
#define _types_h

#include <stdint.h>

#pragma pack(push, 1)

//=============================================================================
// BLOCK TYPE DEFINITIONS
// These values are used in level data to identify map elements
//=============================================================================

// Floor and basic blocks
#define E_FLOOR                 0x00    // Empty floor
#define E_PLAYER                0x01    // Player start position

// Enemy types (2-0x0A)
#define E_NUB                   0x03    // Basic enemy
#define E_EASY                  0x04    // Easy difficulty enemy
#define E_MEDIUM                0x05    // Medium difficulty enemy
#define E_HARD                  0x06    // Hard difficulty enemy
#define E_BOSS_1                0x07    // First boss
#define E_BOSS_2                0x08    // Second boss
#define E_BOSS_3                0x09    // Third boss
#define E_BIGPOTATO             0x0A    // Big potato enemy

// Item types (0x0D-0x17)
#define E_WEAPON_BLASTER        0x0D    // Blaster weapon pickup
#define E_WEAPON_PLASMA_CUTTER  0x0E    // Plasma cutter pickup
#define E_WEAPON_BFG9000        0x0F    // BFG9000 pickup
#define E_EASTER_EGG_BOOK_1     0x10    // Easter egg book 1
#define E_EASTER_EGG_BOOK_2     0x11    // Easter egg book 2
#define E_EASTER_EGG_BOOK_3     0x12    // Easter egg book 3
#define E_SEPULKI               0x13    // Sepulki item
#define E_MEDKIT_FLOOR          0x14    // Floor medkit (healing)
#define E_AMMO_FLOOR            0x15    // Floor ammo (recharges)
#define E_METAL_BOX             0x17    // Throwable metal box

// Special blocks (0xF0-0xFE)
#define E_MESSAGE_1             0xF0    // Message trigger 1
#define E_MESSAGE_2             0xF1    // Message trigger 2
#define E_MESSAGE_3             0xF2    // Message trigger 3
#define E_MESSAGE_4             0xF3    // Message trigger 4
#define E_MINI_GAME             0xF4    // Mini-game trigger
#define E_EXIT                  0xF5    // Level exit

// Wall types (0xF6-0xFE)
#define E_WALL_TYPE_1           0xF6    // Basic wall
#define E_WALL_TYPE_2           0xF7    // Left
#define E_WALL_TYPE_3           0xF8    // Right
#define E_WALL_TYPE_4           0xF9    // Medkit
#define E_WALL_TYPE_5           0xFA    // Charge
#define E_WALL_DOOR_LEVEL       0xFB    // Level door wall
#define E_DOOR                  0xFC    // Interactive door
#define E_WALL_ILLUMINATOR      0xFD    // Illuminator pattern wall
#define E_WALL_MONITOR          0xFE    // Monitor pattern wall


//=============================================================================
// INLINE TYPE CHECKING FUNCTIONS
// These are defined as inline for performance (no function call overhead)
//=============================================================================

// Check if a block is any type of wall
static inline uint8_t is_wall(uint8_t block) {
    return (block >= E_WALL_TYPE_1 && block <= E_WALL_DOOR_LEVEL);
}

// Check if a block is solid (blocks movement)
static inline uint8_t is_solid(uint8_t block) {
    return (block >= E_WALL_TYPE_1) || (block == E_METAL_BOX);
}

// Check if a block is an enemy entity
static inline uint8_t is_enemy(uint8_t block) {
    return (block == E_EASY || block == E_MEDIUM || block == E_HARD ||
            block == E_BOSS_1 || block == E_BOSS_2 || block == E_BOSS_3 ||
            block == E_BIGPOTATO || block == E_NUB);
}

// Check if a block is a collectible item
static inline uint8_t is_item(uint8_t block) {
    return (block == E_WEAPON_BLASTER || block == E_WEAPON_PLASMA_CUTTER ||
            block == E_WEAPON_BFG9000 || block == E_EASTER_EGG_BOOK_1 ||
            block == E_EASTER_EGG_BOOK_2 || block == E_EASTER_EGG_BOOK_3 ||
            block == E_SEPULKI || block == E_METAL_BOX);
}

// Check if a block is a floor item (healing/ammo)
static inline uint8_t is_floor_item(uint8_t block) {
    return (block == E_MEDKIT_FLOOR || block == E_AMMO_FLOOR);
}

// Check if a block is throwable (box)
static inline uint8_t is_throwable(uint8_t block) {
    return (block == E_METAL_BOX);
}

//=============================================================================
// COORDINATE STRUCTURE
//=============================================================================

struct Coords {
    float x;    // X position in world space
    float y;    // Y position in world space
};

// Helper function to create a coordinate pair
static inline Coords create_coords(float x, float y) {
    Coords c = { x, y };
    return c;
}

#pragma pack(pop)

#endif