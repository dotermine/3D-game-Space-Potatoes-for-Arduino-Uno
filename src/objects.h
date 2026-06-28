// objects.h - Game sprite data declarations
// This file declares all the bitmap and mask data for sprites used in the game,
// including enemies, weapons, items, effects, and UI elements.
// All data is stored in PROGMEM to conserve RAM.

#ifndef OBJECTS_H
#define OBJECTS_H

#include <avr/pgmspace.h>
#include <stdint.h>

//=============================================================================
// SPRITE DIMENSION CONSTANTS
//=============================================================================

#define SPR_SMALL_ENEMY_W       18  // Small enemy sprite width
#define SPR_SMALL_ENEMY_H       20  // Small enemy sprite height
#define SPR_MEDIUM_ENEMY_W      24  // Medium enemy sprite width
#define SPR_MEDIUM_ENEMY_H      24  // Medium enemy sprite height
#define SPR_HARD_ENEMY_W        24  // Hard enemy sprite width
#define SPR_HARD_ENEMY_H        24  // Hard enemy sprite height
#define SPR_BOSS_W              24  // Boss sprite width
#define SPR_BOSS_H              24  // Boss sprite height
#define SPR_SPLASH_W            16  // Splash effect width
#define SPR_SPLASH_H            16  // Splash effect height
#define SPR_HIT_W               32  // Hit effect width
#define SPR_HIT_H               32  // Hit effect height
#define SPR_DEAD_W              16  // Dead enemy sprite width
#define SPR_DEAD_H              16  // Dead enemy sprite height
#define SPR_BIBPOTATO_WH        20  // Big potato sprite size (square)
#define SPR_WEAPON_W            16  // Weapon icon width
#define SPR_WEAPON_H            12  // Weapon icon height
#define SPR_EGG_W               12  // Easter egg sprite width
#define SPR_EGG_H               12  // Easter egg sprite height
#define SPR_BOX_W               16  // Box sprite width
#define SPR_BOX_H               16  // Box sprite height
#define SPR_MEDKIT_FLOOR_W      16  // Medkit floor sprite width
#define SPR_MEDKIT_FLOOR_H      16  // Medkit floor sprite height
#define SPR_WALL_PATTERN_W      16  // Wall pattern width
#define SPR_WALL_PATTERN_H      16  // Wall pattern height
#define SPR_DOOR_PATTERN_W      16  // Door pattern width
#define SPR_DOOR_PATTERN_H      32  // Door pattern height

//=============================================================================
// UI SPRITES
//=============================================================================

extern const uint8_t bmp_logo_s_bits[];      // Small game logo
extern const uint8_t bmp_logo_p_bits[];      // Large game logo
extern const uint8_t bmp_spaceship_bits[];   // Spaceship for secret level

//=============================================================================
// ENEMY SPRITES
//=============================================================================

extern const uint8_t enemy_nub_bits[];       // Nub enemy bitmap
extern const uint8_t enemy_nub_mask[];       // Nub enemy mask
extern const uint8_t enemy_small_bits[];     // Small enemy bitmap
extern const uint8_t enemy_small_mask[];     // Small enemy mask
extern const uint8_t enemy_medium_bits[];    // Medium enemy bitmap
extern const uint8_t enemy_medium_mask[];    // Medium enemy mask
extern const uint8_t enemy_hard_bits[];      // Hard enemy bitmap
extern const uint8_t enemy_hard_mask[];      // Hard enemy mask
extern const uint8_t boss_1_bits[];          // Boss 1 bitmap
extern const uint8_t boss_1_mask[];          // Boss 1 mask
extern const uint8_t boss_2_bits[];          // Boss 2 bitmap
extern const uint8_t boss_2_mask[];          // Boss 2 mask
extern const uint8_t boss_3_bits[];          // Boss 3 bitmap
extern const uint8_t boss_3_mask[];          // Boss 3 mask
extern const uint8_t bigpotato_bits[];       // Big potato bitmap
extern const uint8_t bigpotato_mask[];       // Big potato mask

//=============================================================================
// WEAPON SPRITES
//=============================================================================

extern const uint8_t bmp_fist_bits[];        // Fist weapon bitmap
extern const uint8_t bmp_fist_mask[];        // Fist weapon mask
extern const uint8_t bmp_blaster_bits[];     // Blaster weapon bitmap
extern const uint8_t bmp_blaster_mask[];     // Blaster weapon mask
extern const uint8_t bmp_plasma_cutter_bits[];  // Plasma cutter bitmap
extern const uint8_t bmp_plasma_cutter_mask[];  // Plasma cutter mask
extern const uint8_t bmp_bfg9000_bits[];     // BFG9000 weapon bitmap
extern const uint8_t bmp_bfg9000_mask[];     // BFG9000 weapon mask

//=============================================================================
// WEAPON ICON SPRITES (for HUD notifications)
//=============================================================================

extern const uint8_t bmp_blaster_icon_bits[];
extern const uint8_t bmp_plasma_cutter_icon_bits[];
extern const uint8_t bmp_bfg9000_icon_bits[];
extern const uint8_t bmp_blaster_icon_mask[];
extern const uint8_t bmp_plasma_cutter_icon_mask[];
extern const uint8_t bmp_bfg9000_icon_mask[];

//=============================================================================
// ITEM SPRITES
//=============================================================================

extern const uint8_t bmp_book_bits[];        // Book item bitmap
extern const uint8_t bmp_book_mask[];        // Book item mask
extern const uint8_t bmp_well_bits[];        // Well/floor item bitmap
extern const uint8_t bmp_well_mask[];        // Well/floor item mask
extern const uint8_t bmp_box_bits[];         // Box item bitmap
extern const uint8_t bmp_box_mask[];         // Box item mask
extern const uint8_t egg_item_bits[];        // Easter egg bitmap
extern const uint8_t egg_item_mask[];        // Easter egg mask
extern const uint8_t weapon_item_bits[];     // Weapon item bitmap
extern const uint8_t weapon_item_mask[];     // Weapon item mask
extern const uint8_t medkit_floor_bits[];    // Medkit floor bitmap
extern const uint8_t ammo_floor_bits[];      // Ammo floor bitmap
extern const uint8_t bmp_sep_bits[];      // Sepulki bitmap

//=============================================================================
// EFFECT SPRITES
//=============================================================================

extern const uint8_t eff_blaster_bmp[];      // Blaster explosion effect
extern const uint8_t eff_blaster_msk[];      // Blaster explosion mask
extern const uint8_t eff_cutter_bmp[];       // Cutter explosion effect
extern const uint8_t eff_cutter_msk[];       // Cutter explosion mask
extern const uint8_t eff_bfg_bmp[];          // BFG explosion effect
extern const uint8_t eff_bfg_msk[];          // BFG explosion mask
extern const uint8_t eff_splash_bmp[];       // Splash effect
extern const uint8_t eff_splash_mask[];      // Splash effect mask
extern const uint8_t eff_spawn_bmp[];        // Spawn effect
extern const uint8_t eff_spawn_mask[];       // Spawn effect mask

//=============================================================================
// PROJECTILE SPRITES
//=============================================================================

extern const uint8_t proj_enemy_bmp[];       // Enemy projectile bitmap
extern const uint8_t proj_enemy_mask[];      // Enemy projectile mask

//=============================================================================
// DAMAGE SPRITES
//=============================================================================

extern const uint8_t hit_character_bits[];   // Hit effect bitmap
extern const uint8_t hit_character_mask[];   // Hit effect mask

//=============================================================================
// DAMAGE RESOURCES
//=============================================================================

extern const uint8_t* const damage_res[];    // Array of damage sprite pointers

#endif