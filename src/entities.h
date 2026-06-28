// entities.h - Entity system header
// This file declares all entity types, structures, and functions
// used for enemies, items, projectiles, doors, and throwable objects.

#ifndef _entities_h
#define _entities_h

#include "types.h"
#include "constants.h"

//=============================================================================
// ENEMY PROPERTY FLAGS
//=============================================================================

#define MASK_MELEE      (1 << 0)    // Enemy can perform melee attacks
#define MASK_SHOOTER    (1 << 1)    // Enemy can shoot projectiles
#define MASK_FLYING     (1 << 2)    // Enemy can fly/move through air

// Enemy properties table (indexed by enemy type)
// Each entry is a bitmask of the above flags
extern const uint8_t ENEMY_PROPERTIES_TABLE[] PROGMEM;

//=============================================================================
// ENTITY STRUCTURES
//=============================================================================

// Door structure - compact bitfield representation
struct Door {
    uint8_t x : 4;      // X position in level grid
    uint8_t y : 4;      // Y position in level grid
    uint8_t state : 2;  // Door state (S_OPEN, S_CLOSE, S_OPENING, S_CLOSING)
    uint8_t timer : 6;  // Animation timer
};

// Player structure
struct Player {
    Coords pos;         // World position
    Coords dir;         // Looking direction vector
    Coords plane;       // Camera plane vector
    uint8_t health;     // Current health (0-100)
    uint8_t current_weapon;     // Currently selected weapon
    uint8_t ammo[5];            // Ammo for each weapon
    uint8_t weapons_unlocked;   // Bitmask of unlocked weapons
    uint8_t shoot_timer;        // Cooldown timer for shooting
    uint8_t flags;              // Player state flags
    uint8_t held_item;          // Item currently held
};

// Enemy entity structure
struct Entity {
    Coords pos;         // World position
    uint8_t health;     // Current health
    uint8_t type;       // Enemy type (E_EASY, E_MEDIUM, etc.)
    uint8_t state;      // Enemy state (S_STAND, S_MOVE, etc.)
    uint8_t timer;      // Animation/state timer
    uint8_t shoot_timer;        // Shooting cooldown
};

// Item structure
struct Item {
    uint8_t type : 5;       // Item type
    uint8_t x : 5;          // X position in level grid
    uint8_t y : 5;          // Y position in level grid
    uint8_t collected : 1;  // 1 if already collected
};

// Projectile structure
struct Projectile {
    float x;                // World X position
    float y;                // World Y position
    float vx_dir;           // X velocity direction
    float vy_dir;           // Y velocity direction
    uint8_t state;          // State flags (timer, height, type)
};

// Projectile state flags
#define PROJ_MASK_TYPE      0x0F    // Type mask
#define STATE_FLYING        0x10    // Projectile is in flight
#define STATE_EXPLODE       0x20    // Projectile is exploding

// Throwable object structure
struct Throwable {
    float x, y;             // World position
    float vx, vy;           // Velocity
    uint8_t type;           // Object type
    uint8_t active;         // 1 if active
    uint8_t timer;          // Lifetime timer
};

//=============================================================================
// GLOBAL EXTERNAL DECLARATIONS
//=============================================================================

extern Door doors[MAX_DOORS];
extern uint8_t num_doors;
extern Throwable throwables[MAX_THROWABLES];
extern uint8_t num_throwables;
extern Entity* g_current_render_entity;

//=============================================================================
// ENTITY FUNCTIONS
//=============================================================================

// Create a new entity at the specified grid position
Entity create_entity(uint8_t type, uint8_t x, uint8_t y);

// Get the current state of a door at (x,y)
uint8_t getDoorState(uint8_t x, uint8_t y);

// Update a door's state (open/close animation)
void updateDoor(Door* door);

// Update enemy AI (movement, attack, state transitions)
void updateEnemyAI(Entity* e);

// Spawn an enemy projectile
void spawnEnemyProjectile(float x, float y, float dx, float dy, float distSq);

// Update throwable objects (movement, collision)
void updateThrowables(void);

// Throw the currently held item
void throwHeldItem(void);

// Add a visual effect at the specified position
void addEffect(uint8_t type, float x, float y);

// Update all effects (timers, active state)
void updateEffects(void);

// Render all active effects
void renderEffects(void);

// Apply damage to an entity
extern void applyDamage(Entity* e, uint8_t weapon_type);

#endif