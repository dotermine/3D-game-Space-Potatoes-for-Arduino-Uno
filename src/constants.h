// constants.h - Core game constants and configuration
// This file defines all compile-time constants used throughout the game engine.
// It includes screen dimensions, physics parameters, game mechanics values,
// and various lookup tables for the raycasting renderer and game logic.
// The constants are organized into logical groups for easy maintenance.

#pragma pack(push, 1)

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>
#include <stdint.h>

//=============================================================================
// HARDWARE & DISPLAY CONFIGURATION
//=============================================================================

#define SOUND_PIN           9       // PWM pin for sound output
#define SCREEN_WIDTH        128     // OLED display width in pixels
#define SCREEN_HEIGHT       64      // OLED display height in pixels
#define RENDER_HEIGHT       SCREEN_HEIGHT
#define RENDER_WIDTH        SCREEN_WIDTH
#define HALF_WIDTH          64      // Half of screen width for centering
#define HALF_HEIGHT         32      // Half of screen height for centering

//=============================================================================
// RENDERING ENGINE PARAMETERS
//=============================================================================

#define RES_DIVIDER         2       // Render every N pixels for performance
#define Z_RES_DIVIDER       32      // Z-buffer resolution divider
#define MAX_RENDER_DEPTH    2       // Maximum raycasting depth
#define ZBUFFER_SIZE        16      // Size of the Z-buffer for sprite sorting

//=============================================================================
// LEVEL CONSTANTS
//=============================================================================

#define LEVEL_WIDTH         12      // Level width in cells
#define LEVEL_HEIGHT        12      // Level height in cells
#define LEVEL_SIZE          144     // Total cells in a level (WIDTH * HEIGHT)
#define MAX_LEVELS          4       // Number of levels (3 normal + 1 secret)
#define LEVEL_TRANSITION_TIME 60    // Frames for level transition animation

//=============================================================================
// ENEMY COMBAT CONSTANTS
//=============================================================================

#define ENEMY_MELEE_DIST         1.0f   // Melee attack range in world units
#define ENEMY_MELEE_DIST_SQ      1.0f   // Squared melee range for distance checks
#define ENEMY_MELEE_KNOCKBACK    -20    // Knockback velocity when melee hit
#define MELEE_ANIMATION_DURATION 6      // Frames for melee attack animation
#define ENEMY_ATTACK_DURATION    6      // Frames for enemy attack animation
#define ENEMY_HURT_DURATION      6      // Frames for enemy hurt animation
#define ENEMY_DEATH_DURATION     6      // Frames for enemy death animation
#define ENEMY_SHOOT_DELAY_BOSS   40     // Boss shooting cooldown in frames
#define ENEMY_SHOOT_DELAY_NORMAL 30     // Normal enemy shooting cooldown
#define ENEMY_COLLISION_RADIUS_SQ 0.4f  // Enemy-player collision radius squared

//=============================================================================
// PROJECTILE CONSTANTS
//=============================================================================

#define PROJECTILE_SPEED          4.0f   // Projectile speed in world units/frame
#define PROJECTILE_LIFE           20     // Projectile lifetime in frames
#define PROJECTILE_LIFE_SECRET    40     // Projectile lifetime in secret level
#define PROJECTILE_COLLIDER_DIST_SQ 0.1f // Collision distance squared
#define PROJECTILE_DAMAGE_PLAYER  6      // Damage dealt to player by enemy projectiles

//=============================================================================
// ITEM & COLLECTIBLE CONSTANTS
//=============================================================================

#define ITEM_COLLECTOR_DIST_SQ_INT 1      // Item pickup distance squared (integer)
#define AMMO_PICKUP_AMOUNT         10     // Ammo gained per pickup
#define MEDKIT_HEAL_AMOUNT         10     // Health restored by medkit
#define MAX_AMMO                   50     // Maximum ammunition per weapon

//=============================================================================
// DOOR CONSTANTS
//=============================================================================

#define MAX_DOORS           5          // Maximum interactive doors per level
#define DOOR_ANIMATION_TIME 4          // Frames for door open/close animation
#define DOOR_OPEN_TIME      20         // Frames door stays open before closing

//=============================================================================
// ENTITY & GAME OBJECT LIMITS
//=============================================================================

#define MAX_ENTITIES        7          // Maximum enemies per level
#define MAX_ENEMY_VIEW      80         // Enemy vision range squared
#define MAX_ITEMS           6          // Maximum items per level
#define MAX_THROWABLES      1          // Maximum throwable objects
#define MAX_PROJECTILES     4          // Maximum active projectiles
#define MAX_EFFECTS         8          // Maximum visual effects

//=============================================================================
// WEAPON CONSTANTS
//=============================================================================

#define WEAPON_FIST         0   // Fist/melee weapon ID
#define WEAPON_BLASTER      1   // Blaster weapon ID
#define WEAPON_CUTTER       2   // Plasma cutter ID
#define WEAPON_BFG          3   // BFG9000 weapon ID
#define MAX_WEAPONS         4   // Total number of weapons
#define WEAPON_BOX          5   // Throwable box weapon ID
#define WEAPON_SHOOT_DELAY  5   // Frames between shots

//=============================================================================
// DIFFICULTY CONSTANTS
//=============================================================================

#define DIFF_EASY   1   // Easy difficulty multiplier
#define DIFF_NORM   3   // Normal difficulty multiplier
#define DIFF_HARD   5   // Hard difficulty multiplier

//=============================================================================
// ENEMY STATE MACHINE
//=============================================================================

#define S_STAND     0   // Enemy standing idle
#define S_MOVE      1   // Enemy moving
#define S_DAMAGED   2   // Enemy taking damage
#define S_ATTACK    3   // Enemy attacking
#define S_DYING     4   // Enemy death animation
#define S_DEAD      5   // Enemy dead

//=============================================================================
// DOOR STATE MACHINE
//=============================================================================

#define S_OPEN      0   // Door fully open
#define S_CLOSE     1   // Door fully closed
#define S_OPENING   2   // Door is opening
#define S_CLOSING   3   // Door is closing

//=============================================================================
// PROJECTILE STATE MACHINE
//=============================================================================

#define PS_INACTIVE 0   // Projectile inactive
#define PS_FLYING   1   // Projectile in flight
#define PS_HIT      2   // Projectile hit something
#define PS_EXPLODES 3   // Projectile exploding
#define PS_ENEMY    5   // Enemy projectile

//=============================================================================
// GAME STATE MACHINE
//=============================================================================

#define STATE_INTRO    0   // Title screen
#define STATE_LEVEL_1  1   // Level 1
#define STATE_LEVEL_2  2   // Level 2
#define STATE_LEVEL_3  3   // Level 3
#define STATE_SECRET   4   // Secret level
#define STATE_LOSE     5   // Game over
#define STATE_WIN      6   // Victory

//=============================================================================
// PLAYER FLAGS
//=============================================================================

#define P_FIRE_HELD    0x01    // Fire button is being held
#define P_AUTO_FIRE    0x02    // Auto-fire mode enabled

//=============================================================================
// EFFECT CONSTANTS
//=============================================================================

#define EFFECT_SPLASH_DAMAGE    0   // Splash damage effect
#define EFFECT_BLASTER_EXPLOSION 1  // Blaster explosion effect
#define EFFECT_CUTTER_EXPLOSION 2   // Cutter explosion effect
#define EFFECT_BFG_EXPLOSION    3   // BFG explosion effect
#define EFFECT_DEATH            4   // Death effect

#define EFFECT_DURATION         6   // Effect duration in frames
#define EFFECT_DEATH_DURATION   12  // Death effect duration
#define EFFECT_START_SCALE      4   // Starting scale of effect
#define EFFECT_MAX_SCALE        20  // Maximum scale of effect
#define EFFECT_TIMER_MASK       0x0F    // Timer mask for effect encoding
#define EFFECT_ENEMY_TYPE_MASK  0xC0    // Enemy type mask in effect timer
#define EFFECT_ENEMY_TYPE_SHIFT 6       // Shift for enemy type encoding

// Effect helper macros
#define EFFECT_GET_TIMER(e) ((e)->timer & EFFECT_TIMER_MASK)
#define EFFECT_GET_ENEMY_TYPE(e) (((e)->timer & EFFECT_ENEMY_TYPE_MASK) >> EFFECT_ENEMY_TYPE_SHIFT)
#define EFFECT_SET_TIMER_AND_TYPE(e, t, eth) ((e)->timer = ((t) & EFFECT_TIMER_MASK) | (((eth) << EFFECT_ENEMY_TYPE_SHIFT) & EFFECT_ENEMY_TYPE_MASK))

//=============================================================================
// SCREEN SHAKE CONSTANTS
//=============================================================================

#define SHAKE_DURATION_DAMAGE   10  // Shake duration when player takes damage
#define SHAKE_DURATION_DEATH    10  // Shake duration on enemy death
#define SHAKE_AMPLITUDE         2   // Maximum shake offset in pixels

//=============================================================================
// NOTIFICATION CONSTANTS
//=============================================================================

#define NOTIFY_DURATION         4   // Notification display duration
#define NOTIFY_HEIGHT           20  // Notification height in pixels
#define NOTIFY_Y_START          (SCREEN_HEIGHT - NOTIFY_HEIGHT)  // Y position of notification
#define NOTIFY_ANIM_SPEED       6   // Notification animation speed
#define NOTIFY_CORNER_RADIUS    3   // Notification corner radius

//=============================================================================
// BITMAP DIMENSIONS
//=============================================================================

#define BMP_LOGO_S_WIDTH        48  // Small logo width
#define BMP_LOGO_S_HEIGHT       9   // Small logo height
#define BMP_LOGO_P_WIDTH        58  // Large logo width
#define BMP_LOGO_P_HEIGHT       11  // Large logo height
#define BMP_SPACESHIP_WIDTH     32  // Spaceship sprite width
#define BMP_SPACESHIP_HEIGHT    16  // Spaceship sprite height
#define BMP_FIST_WIDTH          28  // Fist sprite width
#define BMP_FIST_HEIGHT         28  // Fist sprite height
#define BMP_WEAPON_ICON_WIDTH   32  // Weapon icon width
#define BMP_WEAPON_ICON_HEIGHT  16  // Weapon icon height
#define BMP_MEDKIT_WIDTH        10  // Medkit sprite width
#define BMP_MEDKIT_HEIGHT       10  // Medkit sprite height
#define BMP_AMMO_WIDTH          10  // Ammo sprite width
#define BMP_AMMO_HEIGHT         14  // Ammo sprite height
#define BMP_BOOK_WIDTH          12  // Book sprite width
#define BMP_BOOK_HEIGHT         12  // Book sprite height
#define BMP_BIG_POTATO_SIZE     28  // Big potato sprite size
#define BMP_NORMAL_POTATO_SIZE  32  // Normal potato sprite size

//=============================================================================
// DAMAGE SPRITE SIZES
//=============================================================================

#define DAMAGE_FIST_SIZE        16  // Fist damage sprite size
#define DAMAGE_BLASTER_SIZE     16  // Blaster damage sprite size
#define DAMAGE_CUTTER_SIZE      16  // Cutter damage sprite size
#define DAMAGE_BFG_WIDTH        24  // BFG damage sprite width
#define DAMAGE_BFG_HEIGHT       24  // BFG damage sprite height

//=============================================================================
// UI CONSTANTS
//=============================================================================

#define DEAD_CHAR_WIDTH         32  // Dead character sprite width
#define DEAD_CHAR_HEIGHT        12  // Dead character sprite height

//=============================================================================
// RAYCASTING & RENDER CONSTANTS
//=============================================================================

#define WALL_RENDER_OFFSET      0.2f    // Wall render offset for texture alignment
#define DOOR_CHECK_DIST         1.0f    // Distance to check for door interaction
#define MELEE_RADIUS_SQ         2.0f    // Melee attack radius squared
#define INV_SW                  0.015625f   // Inverse of screen width (1/64)
#define WALL_HEIGHT_FACTOR      38      // Wall height calculation factor
#define VIEW_HEIGHT             0.25f   // View height offset
#define MIN_WALL_DISTANCE       0.1f    // Minimum wall distance to prevent overflow
#define MAX_WALL_DISTANCE       8.0f    // Maximum wall render distance
#define PLAYER_OFFSET           0.0f    // Player view height offset

//=============================================================================
// PLAYER PHYSICS CONSTANTS
//=============================================================================

#define DAMPING                 0.98f   // Velocity damping per frame
#define ACCEL_FORCE             0.025f  // Acceleration force
#define MAX_SPEED               0.35f   // Maximum player speed
#define WALL_DIST               0.5f    // Wall collision distance

//=============================================================================
// EFFECT STRUCTURE
//=============================================================================

struct Effect {
    uint8_t type;       // Effect type (EFFECT_*)
    uint8_t active;     // 1 if effect is active
    uint8_t timer;      // Timer for animation
    uint8_t pad;        // Padding for alignment
    int16_t x, y;       // Effect position (fixed-point)
};

extern Effect effects[MAX_EFFECTS];
extern uint8_t num_effects;

#endif

#pragma pack(pop)