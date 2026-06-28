// sketch.ino - Main game loop and core game logic
// This file contains the main Arduino entry point (setup/loop) and the core
// game state management, level initialization, player movement, rendering,
// and game loop coordination for the raycasting engine.

#include "constants.h"
#include "level.h"
#include "input.h"
#include "display.h"
#include "sound.h"
#include "render_state.h"
#include "objects.h"
#include "game_state.h"

//=============================================================================
// GLOBAL VARIABLES
//=============================================================================

// Timing and visual effects
uint8_t g_tick_counter = 0;         // Global frame counter for animations
uint8_t lamp_is_dark = 0;           // Lamp state flag
uint8_t hud_effect = 0;             // Current HUD effect type

// Entity rendering
Entity* g_current_render_entity = 0;

//=============================================================================
// ENEMY SPRITE METADATA TABLE
// Format: bits, mask, src_w, src_h, scale_size
//=============================================================================

const uint16_t ENEMY_META[] PROGMEM = {
    (uint16_t)enemy_nub_bits, (uint16_t)enemy_nub_mask, 20, 20, 32,
    (uint16_t)enemy_small_bits, (uint16_t)enemy_small_mask, 20, 20, 32,
    (uint16_t)enemy_medium_bits, (uint16_t)enemy_medium_mask, 24, 24, 32,
    (uint16_t)enemy_hard_bits, (uint16_t)enemy_hard_mask, 24, 24, 32,
    (uint16_t)boss_1_bits, (uint16_t)boss_1_mask, 24, 24, 32,
    (uint16_t)boss_2_bits, (uint16_t)boss_2_mask, 24, 24, 32,
    (uint16_t)boss_3_bits, (uint16_t)boss_3_mask, 24, 24, 32,
    (uint16_t)bigpotato_bits, (uint16_t)bigpotato_mask, SPR_BIBPOTATO_WH, SPR_BIBPOTATO_WH, 32
};

//=============================================================================
// GAME STATE VARIABLES
//=============================================================================

RenderState g_render;               // Current render state
Player player;                      // Player object
float velocity_x = 0.0f;            // Player velocity X
float velocity_y = 0.0f;            // Player velocity Y
Entity entity[MAX_ENTITIES];        // Enemy entities array
uint8_t num_entities = 0;           // Number of active entities
Item items[MAX_ITEMS];              // Items array
uint8_t num_items = 0;              // Number of active items
Projectile projectiles[MAX_PROJECTILES];  // Projectiles array
uint8_t flash_screen = 0;           // Screen flash timer
uint8_t fist_side = 0;              // Fist attack side
int8_t bob_off[2] = {0, 0};         // Weapon bobbing offsets
uint8_t bob_tick = 0;               // Bobbing animation tick
bool melee_attacking = false;       // Melee attack flag
uint8_t melee_timer = 0;            // Melee attack timer
Door doors[MAX_DOORS];              // Doors array
uint8_t num_doors = 0;              // Number of active doors

// Cheat system variables
uint8_t cheat_progress = 0;         // Cheat code progress
bool cheat_menu_showing = false;    // Cheat menu visibility
uint8_t cheat_menu_selection = 0;   // Cheat menu selection

// Rotation constants
constexpr float ROT_SIN = 0.2f;     // Rotation sine factor
constexpr float ROT_COS = 0.98f;    // Rotation cosine factor

//=============================================================================
// LEVEL DATA
//=============================================================================

const uint8_t* level_data[MAX_LEVELS + 1] = {
    map_level_1,
    map_level_2,
    map_level_3,
    map_secret_level
};

//=============================================================================
// MESSAGE STRINGS
//=============================================================================

const char msg_level_1[] PROGMEM = "1 CARGO";
const char msg_level_2[] PROGMEM = "2 CREW";
const char msg_level_3[] PROGMEM = "3 BRIDGE";
const char msg_secret_lev[] PROGMEM = "SPACE";
const char msg_god_mode[] PROGMEM = "GOD";
const char msg_exit[] PROGMEM = "EXIT";

const char* const cheat_menu_items[] PROGMEM = {
    msg_level_1, msg_level_2, msg_level_3, msg_secret_lev, msg_god_mode, msg_exit
};

const char msg_weapon_blaster[] PROGMEM = "DL 44";
const char msg_weapon_plasma_cutter[] PROGMEM = "211 V";
const char msg_weapon_bfg9000[] PROGMEM = "BFG";
const char msg_book_cyberiad[] PROGMEM = "CYBERIAD";
const char msg_book_solaris[] PROGMEM = "SOLARIS";
const char msg_book_invincible[] PROGMEM = "INVINCIBLE";
const char msg_sepulki[] PROGMEM = "SEPULKI";
const char msg_message_1[] PROGMEM = "THEY ARE INSIDE";
const char msg_message_2[] PROGMEM = "WHAT THE ...";
const char msg_message_3[] PROGMEM = "NOW WHAT?";
const char msg_message_4[] PROGMEM = "GET OVER HERE!";
const char msg_final_bad[] PROGMEM = "WASTED";
const char msg_final_good1[] PROGMEM = "TO INFINITY";
const char msg_final_good2[] PROGMEM = "AND";
const char msg_final_good3[] PROGMEM = "BEYOND";
const char msg_intro_1[] PROGMEM = "FAR FAR AWAY";
const char msg_intro_2[] PROGMEM = "NEAR TAIRIYA";

//=============================================================================
// SOUND DATA
//=============================================================================

static const uint8_t snd_weapons[] PROGMEM = {
    snd_shoot_melee, snd_shoot_blaster, snd_shoot_laser, snd_shoot_plasma, snd_door_open
};

//=============================================================================
// WEAPON RESOURCES
//=============================================================================

struct WeaponResource {
    const uint8_t* bits;
    const uint8_t* mask;
};

const WeaponResource weapon_res[] PROGMEM = {
    {bmp_fist_bits, bmp_fist_mask},
    {bmp_blaster_bits, bmp_blaster_mask},
    {bmp_plasma_cutter_bits, bmp_plasma_cutter_mask},
    {bmp_bfg9000_bits, bmp_bfg9000_mask}
};

//=============================================================================
// SPRITE RENDER DATA
//=============================================================================

struct SpriteRenderData {
    const uint8_t* bits;
    const uint8_t* mask;
    uint8_t src_w;
    uint8_t src_h;
    uint8_t mirror;
    uint8_t invert;
};

//=============================================================================
// FORWARD DECLARATIONS
//=============================================================================

void update_music(void);
uint8_t stateToLevel(uint8_t state);
const uint8_t* getLevelData(uint8_t state);
void showNotification(const uint8_t* text, const uint8_t* icon, uint8_t w, uint8_t h);
void updateNotification(void);
void drawCheatMenu(void);
bool performAttack(uint8_t weapon);
bool handleShooting(void);
void initializeLevel(const uint8_t level_index);
void loopLevelStart(void);
void loopLevelComplete(void);
void handleDoorInteraction(void);
void updateEntities(const uint8_t level[]);
uint8_t getDoorState(uint8_t x, uint8_t y);
void updateDoor(Door* d);
void setup(void);
void jumpTo(uint8_t new_state);
void collectItems(void);
uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y);
void updateMeleeAnimation(void);
bool checkCollisionAtPoint(float x, float y, bool is_player);
Door* getDoorAt(uint8_t x, uint8_t y);
void renderMap(const uint8_t level[]);
bool isVisible(float x1, float y1, float x2, float y2);
void renderGun(int8_t y, bool is_moving, int8_t roll_x);
void drawBeginEnd(bool isIntro);
void loopIntroFinal(void);
void handleWeaponChange(void);
void handleCheatCode(void);
void handleCheatMenu(void);
void loopGamePlay(void);
void spawnProjectile(float start_x, float start_y, float dir_x, float dir_y, uint8_t proj_type);
void spawnPlayerProjectile(uint8_t weapon_type);
void updateProjectiles(const uint8_t level[]);
void renderProjectiles(void);
void loop(void);
void loopMiniGame(void);

//=============================================================================
// STATE TO LEVEL MAPPING
//=============================================================================

uint8_t stateToLevel(uint8_t state) {
    switch(state) {
        case STATE_LEVEL_1: return 0;
        case STATE_LEVEL_2: return 1;
        case STATE_LEVEL_3: return 2;
        case STATE_SECRET: return 3;
        default: return 0;
    }
}

const uint8_t* getLevelData(uint8_t state) {
    switch(state) {
        case STATE_LEVEL_1: return map_level_1;
        case STATE_LEVEL_2: return map_level_2;
        case STATE_LEVEL_3: return map_level_3;
        case STATE_SECRET: return map_secret_level;
        default: return map_level_1;
    }
}

//=============================================================================
// NOTIFICATION SYSTEM
//=============================================================================

uint8_t notify_item_type = 0;
uint8_t notify_timer = 0;

// Item metadata table: bits, mask, w, h, scale
const int16_t ITEM_META[] PROGMEM = {
    (int16_t)bmp_well_bits, (int16_t)bmp_well_mask, 16, 16, 56,
    (int16_t)bmp_well_bits, (int16_t)bmp_well_mask, 16, 16, 48,
    (int16_t)bmp_box_bits, (int16_t)bmp_box_mask, 16, 16, 32,
    (int16_t)bmp_sep_bits, (int16_t)NULL, 8, 8, 24,
    (int16_t)bmp_blaster_icon_bits, (int16_t)bmp_blaster_icon_mask, 32, 16, -20,
    (int16_t)bmp_plasma_cutter_icon_bits, (int16_t)bmp_plasma_cutter_icon_mask, 32, 16, -20,
    (int16_t)bmp_bfg9000_icon_bits, (int16_t)bmp_bfg9000_icon_mask, 32, 16, -20,
    (int16_t)bmp_book_bits, (int16_t)bmp_book_mask, 12, 12, 32,
    (int16_t)bmp_book_bits, (int16_t)bmp_book_mask, 12, 12, 32,
    (int16_t)bmp_book_bits, (int16_t)bmp_book_mask, 12, 12, 32,
    (int16_t)egg_item_bits, (int16_t)egg_item_mask, 12, 12, 32
};

const char* const notification_titles[] PROGMEM = {
    msg_weapon_blaster,
    msg_weapon_plasma_cutter,
    msg_weapon_bfg9000,
    msg_book_cyberiad,
    msg_book_solaris,
    msg_book_invincible,
    msg_sepulki 
};

const uint8_t NOTIFY_STR_LEN[] PROGMEM = {
    5,   // "DL-44"
    6,   // "211-V"
    8,   // "BFG-9000"
    8,   // "CYBERIAD"
    7,   // "SOLARIS"
    10,  // "INVINCIBLE"
    7    // "SEPULKI"
};

//=============================================================================
// NOTIFICATION UPDATE
//=============================================================================

// Display notification popup for item pickups (weapons, books, sepulki)
// Handles both icon + text display with animated border and fade effects
void updateNotification(void) {
    uint8_t t = notify_timer;
    if (0 == t) return;  // No active notification
    
    // Decrement timer each frame
    notify_timer = t - 1;
    
    // Blink effect: skip drawing on alternating frames during start/end phases
    // This creates a subtle blinking animation when notification appears/disappears
    if ((t > (uint8_t)((NOTIFY_DURATION * 3) - 4) || 4 >= t) && (g_tick_counter & 1)) return;
    
    uint8_t typ = notify_item_type;
    uint8_t idx = 0;        // Index into notification_titles[] array
    uint8_t meta_idx = 0;   // Index into ITEM_META[] array
    bool has_icon = true;   // Flag indicating if this item has an icon sprite
    
    // Determine which notification type we're displaying and get corresponding indices
    // Weapons: E_WEAPON_BLASTER (0x0D) to E_WEAPON_BFG9000 (0x0F) -> indices 0-2
    if (typ >= E_WEAPON_BLASTER && typ <= E_WEAPON_BFG9000) {
        idx = typ - E_WEAPON_BLASTER;
        meta_idx = 4 + idx;  // Weapon icons start at ITEM_META index 4
    }
    // Books (easter eggs): E_EASTER_EGG_BOOK_1 (0x10) to E_EASTER_EGG_BOOK_3 (0x12) -> indices 3-5
    else if (typ >= E_EASTER_EGG_BOOK_1 && typ <= E_EASTER_EGG_BOOK_3) {
        idx = 3 + (typ - E_EASTER_EGG_BOOK_1);
        meta_idx = 7 + (typ - E_EASTER_EGG_BOOK_1);
    }
    // Sepulki special item -> index 6, meta index 3
    else if (typ == E_SEPULKI) {
        idx = 6;
        meta_idx = 3;  // Sepulki icon is at ITEM_META index 3
        has_icon = true;
    }
    // Unknown item type - ignore
    else {
        return;
    }
    
    // Safety check: ensure index is within bounds
    if (idx > 6) return;
    
    // Get notification text from PROGMEM
    const char *text_ptr = (const char*)pgm_read_word(&notification_titles[idx]);
    
    // Get icon data from ITEM_META table (5 fields per entry: bits, mask, w, h, scale)
    uint16_t base = (uint16_t)meta_idx * 5;
    const uint8_t *icon_ptr = (const uint8_t*)pgm_read_word(&ITEM_META[base]);
    const uint8_t *mask_ptr = (const uint8_t*)pgm_read_word(&ITEM_META[base + 1]);
    uint8_t iw = (uint8_t)pgm_read_word(&ITEM_META[base + 2]);
    uint8_t ih = (uint8_t)pgm_read_word(&ITEM_META[base + 3]);
    uint8_t len = pgm_read_byte(&NOTIFY_STR_LEN[idx]);
    
    // Fallback: if no icon available, display just the text
    if (icon_ptr == 0 || !has_icon) {
        // Center the text horizontally
        uint8_t tx = (SCREEN_WIDTH - ((len << 2) + (len << 1))) >> 1;
        drawText5x5(tx, 50, text_ptr, 0xFFFF);
        return;
    }
    
    // Calculate horizontal position for icon + text combination
    // Text width = len * 6 (5px char + 1px spacing)
    uint8_t tx = SCREEN_WIDTH - ((len << 2) + (len << 1));
    if (icon_ptr != 0) {
        tx -= (uint8_t)(iw + 4);  // Icon width + 4px spacing
    }
    tx >>= 1;  // Center on screen
    
    const uint8_t ny = 50;  // Vertical position (near bottom of screen)
    const uint8_t x_start = 2;
    const uint8_t x_end = 125;
    uint8_t w_count = x_end - x_start + 1;
    
    // Draw notification background border
    // Using two rows at the bottom of the display (pages 6 and 7)
    uint16_t addr6 = 768 + x_start;  // Page 6 start address
    uint16_t addr7 = 896 + x_start;  // Page 7 start address
    
    // Draw horizontal lines (top and bottom of notification box)
    while (w_count--) {
        display_buffer[addr6++] = 0x01;  // Top border row
        display_buffer[addr7++] = 0x80;  // Bottom border row
    }
    // Draw rounded corners
    display_buffer[768 + x_start] = 0xFF;
    display_buffer[896 + x_start] = 0xFF;
    display_buffer[768 + x_end] = 0xFF;
    display_buffer[896 + x_end] = 0xFF;
    
    // Draw icon sprite at calculated position
    if (icon_ptr != 0) {
        // Vertically center the icon within the 12px tall notification area
        int8_t iy = ny + ((12 - (int8_t)ih) >> 1);
        drawSprite(tx, iy, icon_ptr, mask_ptr, iw, ih, iw, ih, 0, 0);
        tx += iw + 4;  // Move text position after icon
    }
    
    // Draw notification text
    drawText5x5(tx, ny + 3, text_ptr, 0xFFFF);
}

// Activate a notification popup for a specific item type
// The notification will appear for (NOTIFY_DURATION * 3) frames
void showNotification(uint8_t item_type) {
    // Only show notifications for weapons, books, and sepulki
    if ((item_type >= E_WEAPON_BLASTER && item_type <= E_WEAPON_BFG9000) ||
        (item_type >= E_EASTER_EGG_BOOK_1 && item_type <= E_EASTER_EGG_BOOK_3) ||
        (item_type == E_SEPULKI)) {
        notify_item_type = item_type;
        // Notification duration: 3 phases (appear, hold, disappear)
        notify_timer = (NOTIFY_DURATION << 1) + NOTIFY_DURATION;
    }
}


//=============================================================================
// ITEM COLLECTION
//=============================================================================

void collectItems(void) {
    int8_t p_ix = (int8_t)player.pos.x;
    int8_t p_iy = (int8_t)player.pos.y;
    
    for (uint8_t i = 0; num_items > i; i++) {
        Item *item = &items[i];
        if (item->collected) continue;
        
        int8_t dx = (int8_t)item->x - p_ix;
        int8_t dy = (int8_t)item->y - p_iy;
        if ((int16_t)(dx * dx + dy * dy) > ITEM_COLLECTOR_DIST_SQ_INT) continue;
        
        uint8_t type = item->type;
        uint8_t do_collect = 0;
        uint8_t notify_type = 0;
        uint8_t spawn_fx = 0;
        
        // Metal box pickup
        if (type == E_METAL_BOX) {
            if (0 == player.ammo[WEAPON_FIST]) {
                player.ammo[WEAPON_FIST] = 1;
                player.current_weapon = WEAPON_FIST;
                do_collect = 1;
            }
        }
        
        // Weapon pickups
        uint8_t weapon_delta = (uint8_t)(type - E_WEAPON_BLASTER);
        if ((uint8_t)(E_WEAPON_BFG9000 - E_WEAPON_BLASTER + 1) > weapon_delta) {
            uint8_t wType = weapon_delta + 1;
            player.weapons_unlocked |= (1 << wType);
            player.current_weapon = wType;
            player.ammo[wType] = MAX_AMMO;
            do_collect = 1;
            notify_type = type;
        }
        
        // Easter egg books
        uint8_t book_delta = (uint8_t)(type - E_EASTER_EGG_BOOK_1);
        if ((uint8_t)(E_EASTER_EGG_BOOK_3 - E_EASTER_EGG_BOOK_1 + 1) > book_delta) {
            easter_egg_count++;
            do_collect = 1;
            notify_type = type;
            if (easter_egg_count >= 3) {
                spawn_fx = 1;
            }
        }
        
        // Sepulki pickup
        if (type == E_SEPULKI) {
            do_collect = 1;
            notify_type = type;
        }
        
        if (do_collect > 0) {
            item->collected = true;
            playSound(snd_item_pickup);
            if (notify_type > 0) {
                showNotification(notify_type);
            }
            if (spawn_fx > 0) {
                addEffect(EFFECT_SPLASH_DAMAGE, player.pos.x, player.pos.y);
            }
            break;
        }
    }
}

//=============================================================================
// CHEAT MENU
//=============================================================================

void drawCheatMenu() {
    display.clearDisplay();
    for(uint8_t i = 0; 6 > i; i++) {
        drawText5x5(18, (i+1) << 3, (const char*)pgm_read_ptr(&cheat_menu_items[i]), 0xFFFF);
        if(i == cheat_menu_selection) {
            uint8_t* p = &display.getBuffer()[((i+1) << 7) + 8];
            *p |= 0x1F;
            *(p+1) |= 0x0E;
            *(p+2) |= 0x04;
        }
    }
    display.display();
}

//=============================================================================
// WEAPON SOUND TABLE
//=============================================================================

const uint8_t SOUND_TABLE[] PROGMEM = {
    snd_shoot_melee, snd_shoot_blaster, snd_shoot_laser, snd_shoot_plasma
};

//=============================================================================
// PERFORM ATTACK
//=============================================================================


bool performAttack(uint8_t weapon) {
    // ============ FIST / MELEE ATTACK ============
    if (weapon == WEAPON_FIST) {
        // Prevent attack spam - must wait for animation to finish
        if (melee_attacking) return false;
        
        // Start melee attack animation
        melee_attacking = true;
        melee_timer = 0;
        playSound(snd_shoot_melee);
        return true;
    }
    
    // ============ BOX THROW ============
    if (weapon == WEAPON_BOX) {
        // Box throw is handled separately in handleShooting()
        // This case should never be reached, but kept for safety
        return true;
    }
    
    // ============ RANGED WEAPONS (Blaster, Cutter, BFG) ============
    // Save current shake state before spawning projectile
    uint8_t saved_timer = g_render.shake_timer;
    int8_t saved_x = g_render.shake_x;
    
    // Clear shake to avoid affecting projectile spawn position
    g_render.shake_timer = 0;
    g_render.shake_x = 0;
    
    // Spawn the projectile in the direction player is facing
    spawnPlayerProjectile(weapon);
    
    // Restore shake state
    g_render.shake_timer = saved_timer;
    g_render.shake_x = saved_x;
    
    // Play appropriate sound effect for the weapon
    uint8_t sound;
    switch(weapon) {
        case WEAPON_BLASTER:
            sound = snd_shoot_blaster;
            break;
        case WEAPON_CUTTER:
            sound = snd_shoot_laser;
            break;
        case WEAPON_BFG:
            sound = snd_shoot_plasma;
            break;
        default:
            sound = snd_shoot_blaster;
            break;
    }
    playSound(sound);
    
    return true;
}


//=============================================================================
// HANDLE SHOOTING
//=============================================================================

bool handleShooting(void) {
    uint8_t weapon = player.current_weapon;
    uint8_t has_box = (weapon == WEAPON_FIST && player.ammo[WEAPON_FIST] > 0);
    
    // Throw box logic
    if (has_box && !melee_attacking) {
        if (input_fire() && player.shoot_timer == 0) {
            player.shoot_timer = WEAPON_SHOOT_DELAY;
            throwHeldItem();
            return true;
        }
        if (player.shoot_timer > 0) player.shoot_timer--;
        return false;
    }
    
    if (!input_fire()) {
        player.flags &= ~0x03;
        player.shoot_timer = 0;
        return false;
    }
    
    if (player.shoot_timer > 0) {
        player.shoot_timer--;
        return false;
    }
    
    // Check ammo
    if (weapon != WEAPON_FIST && weapon != WEAPON_BOX) {
        if (player.ammo[weapon] == 0) {
            if (1 > (player.flags & 0x02)) playSound(snd_item_pickup);
            player.flags |= 0x02;
            return false;
        }
        player.ammo[weapon]--;
    }
    
    player.shoot_timer = WEAPON_SHOOT_DELAY;
    performAttack(weapon);
    return true;
}

//=============================================================================
// LEVEL INITIALIZATION
//=============================================================================

void initializeLevel(const uint8_t level_index) {
    // Reset player stats for level 1
    if (level_index == 0) {
        player.health = 100;
        player.current_weapon = WEAPON_FIST;
        player.weapons_unlocked = 1;
        memset(player.ammo, 0, 5);
        fist_side = 0;
        easter_egg_count = 0;
        player.held_item = 0;
        velocity_x = velocity_y = 0.0f;
        bob_tick = 0;
        bob_off[0] = bob_off[1] = 0;
        g_render.shake_timer = 0;
        g_render.shake_x = g_render.shake_y = 0;
    }
    
    melee_attacking = false;
    player.shoot_timer = 0;
    flash_screen = 0;
    
    // Clear all dynamic arrays
    memset(entity, 0, sizeof(entity));
    memset(items, 0, sizeof(items));
    memset(projectiles, 0, sizeof(projectiles));
    memset(doors, 0, sizeof(doors));
    num_entities = num_items = num_doors = 0;
    game.is_secret_level = false;
    
    // Setup secret level
    if (level_index == 3) {
        game.is_secret_level = true;
        player.current_weapon = WEAPON_BFG;
        player.ammo[WEAPON_BFG] = MAX_AMMO;
        player.weapons_unlocked |= (1 << WEAPON_BFG);
        game.is_secret_level = true;
    }
    
    const uint8_t* level_ptr = level_data[level_index];
    float p_x = 2.5f;
    float p_y = 2.5f;
    
    // Parse level data and spawn objects
    for (uint8_t y = 0; LEVEL_HEIGHT > y; y++) {
        for (uint8_t x = 0; LEVEL_WIDTH > x; x++) {
            uint8_t block = getBlockAt(level_ptr, x, y);
            
            if (block == E_PLAYER) {
                p_x = (float)x + 0.5f;
                p_y = (float)y + 0.5f;
                continue;
            }
            
            if (block == E_DOOR) {
                if (MAX_DOORS > num_doors) {
                    struct Door* d = &doors[num_doors];
                    d->x = x;
                    d->y = y;
                    d->state = S_CLOSE;
                    d->timer = 0;
                    num_doors++;
                }
                continue;
            }
            
            if (is_enemy(block)) {
                if (MAX_ENTITIES > num_entities) {
                    entity[num_entities++] = create_entity(block, x, y);
                }
                continue;
            }
            
            if (block == E_MEDKIT_FLOOR || block == E_AMMO_FLOOR || is_item(block)) {
                if (MAX_ITEMS > num_items) {
                    struct Item* it = &items[num_items];
                    it->type = block;
                    it->x = x;
                    it->y = y;
                    it->collected = 0;
                    num_items++;
                }
            }
        }
    }
    
    // Set player position and orientation
    player.pos.x = p_x;
    player.pos.y = p_y;
    player.dir.x = 1.0f;
    player.dir.y = 0.0f;
    player.plane.x = 0.0f;
    player.plane.y = -0.66f;
}

//=============================================================================
// LEVEL START
//=============================================================================

void loopLevelStart(void) {
    uint8_t level_idx = stateToLevel(game.state);
    const char* level_name = (const char*)pgm_read_word(&cheat_menu_items[level_idx]);
    if (level_name != 0) {
        drawFullscreenMessage(level_name);
    }
    initializeLevel(level_idx);
}

//=============================================================================
// LEVEL COMPLETE
//=============================================================================

void loopLevelComplete() {
    uint8_t current_level_idx = stateToLevel(game.state);
    if (2 > current_level_idx) {
        game.level_initialized = false;
        game.state = levelToState(current_level_idx + 1, false);
    } else {
        if (easter_egg_count >= 3 && game.state != STATE_SECRET) {
            game.state = STATE_SECRET;
            game.is_secret_level = true;
            game.level_initialized = false;
        } else {
            game.state = STATE_WIN;
            game.isVictory = true;
        }
    }
}

//=============================================================================
// DOOR INTERACTION
//=============================================================================

void handleDoorInteraction() {
    float checkDist = DOOR_CHECK_DIST;
    uint8_t bx = (uint8_t)(player.pos.x + player.dir.x * checkDist);
    uint8_t by = (uint8_t)(player.pos.y + player.dir.y * checkDist);
    
    if(bx >= LEVEL_WIDTH || by >= LEVEL_HEIGHT) return;
    
    uint8_t level_idx = stateToLevel(game.state);
    uint8_t block = getBlockAt(level_data[level_idx], bx, by);
    if(block != E_DOOR) return;
    
    for(uint8_t i = 0; num_doors > i; i++) {
        if(doors[i].x == bx && doors[i].y == by) {
            if(doors[i].state == S_CLOSE || doors[i].state == S_CLOSING) {
                doors[i].state = S_OPENING;
                doors[i].timer = DOOR_ANIMATION_TIME;
                playSound(snd_door_open);
            }
            return;
        }
    }
    
    // Create new door if not found
    if(MAX_DOORS > num_doors) {
        doors[num_doors].x = bx;
        doors[num_doors].y = by;
        doors[num_doors].state = S_OPENING;
        doors[num_doors].timer = DOOR_ANIMATION_TIME;
        num_doors++;
        playSound(snd_door_open);
    }
}

//=============================================================================
// ENTITY UPDATE
//=============================================================================

void updateEntities(const uint8_t level[]) {
    // Update doors
    for (uint8_t i = 0; num_doors > i; i++) {
        updateDoor(&doors[i]);
    }
    
    // Update enemies
    for (uint8_t i = 0; num_entities > i; i++) {
        Entity *e = &entity[i];
        if (is_enemy(e->type)) {
            updateEnemyAI(e);
        }
    }
}

//=============================================================================
// DOOR STATE FUNCTIONS
//=============================================================================

uint8_t getDoorState(uint8_t x, uint8_t y) {
    for (uint8_t i = 0; num_doors > i; i++) {
        if (doors[i].x == x && doors[i].y == y) {
            return doors[i].state;
        }
    }
    return S_CLOSE;
}

void updateDoor(Door *d) {
    if (0 == d) return;
    
    uint8_t tx = d->x;
    uint8_t ty = d->y;
    if (tx >= LEVEL_WIDTH || ty >= LEVEL_HEIGHT) {
        d->state = S_CLOSE;
        return;
    }
    
    // Check if occupied by player or enemy
    uint8_t occupied = ((uint8_t)player.pos.x == tx && (uint8_t)player.pos.y == ty);
    if (0 == occupied) {
        for (uint8_t i = 0; num_entities > i; i++) {
            Entity *e = &entity[i];
            if (e->state != S_DEAD) {
                if ((uint8_t)e->pos.x == tx && (uint8_t)e->pos.y == ty) {
                    if (is_enemy(e->type)) {
                        occupied = 1;
                        break;
                    }
                }
            }
        }
    }
    
    uint8_t s = d->state;
    if (s == S_OPENING) {
        if (d->timer > 0) {
            d->timer--;
            if (0 == d->timer) {
                d->state = S_OPEN;
                d->timer = DOOR_OPEN_TIME;
            }
        }
    } else if (s == S_OPEN) {
        if (occupied != 0) {
            d->timer = DOOR_OPEN_TIME;
        } else if (d->timer > 0) {
            d->timer--;
            if (0 == d->timer) {
                d->state = S_CLOSING;
                d->timer = DOOR_ANIMATION_TIME;
            }
        }
    } else if (s == S_CLOSING) {
        if (occupied != 0) {
            d->state = S_OPENING;
            d->timer = DOOR_ANIMATION_TIME - d->timer;
        } else if (d->timer > 0) {
            d->timer--;
            if (0 == d->timer) {
                d->state = S_CLOSE;
            }
        }
    }
}

//=============================================================================
// ARDUINO SETUP
//=============================================================================

void setup() {
    initGameState();
    g_render.shake_x = 0;
    g_render.shake_y = 0;
    g_render.shake_timer = 0;
    velocity_x = 0.0f;
    velocity_y = 0.0f;
    
    memset(entity, 0, sizeof(entity));
    memset(items, 0, sizeof(items));
    memset(projectiles, 0, sizeof(projectiles));
    
    setupDisplay();
    input_setup();
    sound_init();
    update_music();
}

//=============================================================================
// JUMP TO STATE
//=============================================================================

void jumpTo(uint8_t new_state) {
    game.state = new_state;
    game.is_secret_level = (new_state == STATE_SECRET);
    game.level_initialized = false;
    game.isVictory = (new_state == STATE_WIN);
    cheat_progress = 0;
    cheat_menu_showing = false;
    velocity_x = velocity_y = 0;
    
    if(new_state == STATE_INTRO) {
        easter_egg_count = 0;
    }
    
    update_music();
}

//=============================================================================
// LEVEL DATA ACCESS
//=============================================================================

uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y) {
    if(x >= LEVEL_WIDTH || y >= LEVEL_HEIGHT) return E_FLOOR;
    uint16_t index = (uint16_t)(LEVEL_HEIGHT - 1 - y) * LEVEL_WIDTH + x;
    return pgm_read_byte(level + index);
}

//=============================================================================
// WEAPON TO EFFECT MAPPING
//=============================================================================

const uint8_t WEAPON_TO_EFFECT[] PROGMEM = {
    EFFECT_SPLASH_DAMAGE,
    EFFECT_BLASTER_EXPLOSION,
    EFFECT_CUTTER_EXPLOSION,
    EFFECT_BFG_EXPLOSION
};

//=============================================================================
// APPLY DAMAGE TO ENTITY
//=============================================================================

void applyDamage(Entity* e, uint8_t weapon_type) {
    g_render.shake_timer = SHAKE_DURATION_DAMAGE;
    
    uint8_t base = weapon_type + 2;
    uint8_t mult = (0 == game.difficulty) ? 1 : (2 == game.difficulty) ? 5 : 3;
    uint8_t damage = base * mult;
    
    if (e->health <= damage) {
        e->health = 0;
        e->state = S_DYING;
        e->timer = ENEMY_DEATH_DURATION;
        flash_screen = 6;
        playSound(snd_enemy_death);
        e->timer = (e->timer & 0x0F) | (4 << 4);
        return;
    }
    
    e->health -= damage;
    e->state = S_DAMAGED;
    e->timer = ENEMY_HURT_DURATION;
    
    uint8_t w_type;
    if (weapon_type == 1) {
        w_type = 1;
    } else if (weapon_type == 2) {
        w_type = 2;
    } else if (weapon_type == 3) {
        w_type = 3;
    } else {
        w_type = 0;
    }
    e->timer = (e->timer & 0x0F) | (w_type << 4);
    playSound(snd_enemy_hurt);
}

//=============================================================================
// MELEE ANIMATION
//=============================================================================

void updateMeleeAnimation(void) {
    if (1 > melee_attacking) return;
    
    if (1 == melee_timer || 4 == melee_timer) {
        bool hit = false;
        const float max_melee_dist_sq = MELEE_RADIUS_SQ;
        
        for (uint8_t i = 0; MAX_ENTITIES > i; i++) {
            Entity *e = &entity[i];
            if (1 > is_enemy(e->type) || e->state == S_DEAD) continue;
            
            float dx = e->pos.x - player.pos.x;
            float dy = e->pos.y - player.pos.y;
            
            if (max_melee_dist_sq > (dx * dx + dy * dy)) {
                if ((dx * player.dir.x + dy * player.dir.y) > 0.5f) {
                    applyDamage(e, WEAPON_FIST);
                    hit = true;
                }
            }
        }
        if (hit != 0) {
            playSound(snd_shoot_melee);
        }
    }
    
    melee_timer++;
    if (melee_timer > 5) {
        melee_attacking = false;
        melee_timer = 0;
    }
}

//=============================================================================
// COLLISION DETECTION
//=============================================================================

const int8_t COLLISION_OFFSETS[] PROGMEM = {
    -60, -60, 60, -60, -60, 60, 60, 60
};

bool checkCollisionAtPoint(float x, float y, bool is_player) {
    uint8_t level_idx = stateToLevel(game.state);
    const uint8_t *m_ptr = level_data[level_idx];
    int16_t x_fixed = (int16_t)(x * 256.0f);
    int16_t y_fixed = (int16_t)(y * 256.0f);
    
    // Check 4 corners
    for (uint8_t corner = 0; corner < 4; corner++) {
        uint8_t idx = corner << 1;
        int8_t ox = (int8_t)pgm_read_byte(&COLLISION_OFFSETS[idx]);
        int8_t oy = (int8_t)pgm_read_byte(&COLLISION_OFFSETS[idx + 1]);
        
        uint8_t cell_x = (uint8_t)((x_fixed + ox) >> 8);
        uint8_t cell_y = (uint8_t)((y_fixed + oy) >> 8);
        
        if (cell_x >= LEVEL_WIDTH || cell_y >= LEVEL_HEIGHT) {
            return true;
        }
        
        uint8_t block = getBlockAt(m_ptr, cell_x, cell_y);
        if (block >= E_WALL_TYPE_1) {
            if (E_WALL_DOOR_LEVEL >= block || block == E_METAL_BOX) {
                return true;
            }
            if (block == E_DOOR) {
                bool door_blocks = true;
                for (uint8_t i = 0; i < num_doors; i++) {
                    if (doors[i].x == cell_x && doors[i].y == cell_y) {
                        if (doors[i].state == S_OPEN) {
                            door_blocks = false;
                        }
                        break;
                    }
                }
                if (door_blocks) return true;
            }
        }
    }
    
    // Check enemy collision for player
    if (is_player) {
        for (uint8_t i = 0; i < num_entities; i++) {
            Entity *e = &entity[i];
            if (e->state == S_DEAD) continue;
            
            float dx = e->pos.x - x;
            float dy = e->pos.y - y;
            if (ENEMY_COLLISION_RADIUS_SQ > (dx * dx + dy * dy)) {
                return true;
            }
        }
    }
    
    return false;
}

Door* getDoorAt(uint8_t x, uint8_t y) {
    for(uint8_t i = 0; num_doors > i; i++) {
        if(doors[i].x == x && doors[i].y == y) return &doors[i];
    }
    return NULL;
}

//=============================================================================
// RENDER MAP (RAYCASTER)
//=============================================================================

void renderMap(const uint8_t level[]) {
    const uint8_t half_h = RENDER_HEIGHT >> 1;
    int16_t v_height_int = (int16_t)VIEW_HEIGHT + g_render.shake_y;
    
    // Secret level rendering mode
    if (game.is_secret_level) {
        static uint8_t bob_counter = 0;
        static int8_t bob_dir = 1;
        int8_t bob_y = 0;
        int8_t bob_x = 0;
        
        // Calculate bob
        int16_t vel_x_int = (int16_t)(velocity_x * 1000.0f);
        int16_t vel_y_int = (int16_t)(velocity_y * 1000.0f);
        if (0 > vel_x_int) vel_x_int = -vel_x_int;
        if (0 > vel_y_int) vel_y_int = -vel_y_int;
        
        if (vel_x_int > 1 || vel_y_int > 1) {
            bob_counter += bob_dir;
            if (bob_counter > 5) { bob_dir = -1; bob_counter = 5; }
            if (1 > bob_counter) { bob_dir = 1; }
            bob_y = bob_counter;
            bob_x = (bob_counter > 3) ? (int8_t)(6 - bob_counter) : (int8_t)bob_counter;
        } else {
            bob_counter = 0;
        }
        
        // Calculate starfield parameters
        uint8_t sc = (uint8_t)(player.dir.x * 64.0f + 64);
        if (0.0f > player.dir.y) sc = 127 - sc;
        
        int16_t f_spd_int = (int16_t)(((int32_t)((int16_t)(velocity_x * 16.0f)) * (int16_t)(player.dir.x * 16.0f) + (int32_t)((int16_t)(velocity_y * 16.0f)) * (int16_t)(player.dir.y * 16.0f)) >> 2);
        int8_t pitch = (f_spd_int > 15) ? 15 : ((f_spd_int < -15) ? -15 : (int8_t)f_spd_int);
        
        // Draw starfield
        memset(display_buffer, 0, 1024);
        for (uint8_t i = 16; i > 0; i--) {
            uint8_t rnd = pgm_read_byte(&font5x5[(uint8_t)(i * 7) & 127]);
            uint8_t sx = (((i & 3) << 5) + (rnd & 31) - sc + bob_x + g_render.shake_x) & 127;
            uint8_t sy = (((i & 12) << 2) + (rnd >> 4) + pitch + bob_y + g_render.shake_y) & 63;
            display_buffer[sx + ((sy >> 3) << 7)] |= (1 << (sy & 7));
        }
        return;
    }
    
    // Floor healing/ammo effects
    uint8_t player_cell_x = (uint8_t)player.pos.x;
    uint8_t player_cell_y = (uint8_t)player.pos.y;
    uint8_t current_standing_block = getBlockAt(level, player_cell_x, player_cell_y);
    hud_effect = 0;
    
    if (current_standing_block == E_MEDKIT_FLOOR) {
        hud_effect = 1;
        static uint8_t heal_timer = 0;
        heal_timer++;
        if (heal_timer > 6) {
            heal_timer = 0;
            if (100 > player.health) {
                player.health += 10;
                if (player.health > 100) player.health = 100;
            }
        }
    }
    
    if (current_standing_block == E_AMMO_FLOOR) {
        hud_effect = 2;
        static uint8_t ammo_timer = 0;
        ammo_timer++;
        if (ammo_timer > 6) {
            ammo_timer = 0;
            for (uint8_t w = WEAPON_BLASTER; w <= WEAPON_BFG; w++) {
                if (MAX_AMMO > player.ammo[w]) {
                    player.ammo[w] += 1;
                    if (player.ammo[w] > MAX_AMMO) player.ammo[w] = MAX_AMMO;
                }
            }
        }
    }
    
    // Raycasting loop
    for (uint8_t x = 0; SCREEN_WIDTH > x; x += RES_DIVIDER) {
        float camera_x = ((x + g_render.shake_x) * INV_SW) - 1.0f;
        g_render.rayDirX = player.dir.x + player.plane.x * camera_x;
        g_render.rayDirY = player.dir.y + player.plane.y * camera_x;
        
        if (g_render.rayDirX == 0.0f) g_render.rayDirX = 0.001f;
        if (g_render.rayDirY == 0.0f) g_render.rayDirY = 0.001f;
        
        g_render.delta_x = (float)__builtin_fabsf(1.0f / g_render.rayDirX);
        g_render.delta_y = (float)__builtin_fabsf(1.0f / g_render.rayDirY);
        
        int8_t map_x = (int8_t)player.pos.x;
        int8_t map_y = (int8_t)player.pos.y;
        int8_t step_x, step_y;
        float side_x, side_y;
        
        // DDA setup
        if (0.0f > g_render.rayDirX) {
            step_x = -1;
            side_x = (player.pos.x - (float)map_x) * g_render.delta_x;
        } else {
            step_x = 1;
            side_x = ((float)map_x + 1.0f - player.pos.x) * g_render.delta_x;
        }
        
        if (0.0f > g_render.rayDirY) {
            step_y = -1;
            side_y = (player.pos.y - (float)map_y) * g_render.delta_y;
        } else {
            step_y = 1;
            side_y = ((float)map_y + 1.0f - player.pos.y) * g_render.delta_y;
        }
        
        uint8_t hit = 0;
        uint8_t side = 0;
        g_render.doorOffset = 0.0f;
        uint8_t depth = 20;
        
        // DDA loop
        for (; depth > 0; --depth) {
            if (map_x < 0 || map_x >= LEVEL_WIDTH || map_y < 0 || map_y >= LEVEL_HEIGHT) {
                hit = 1;
                break;
            }
            
            if (side_y > side_x) {
                side_x += g_render.delta_x;
                map_x += step_x;
                side = 0;
            } else {
                side_y += g_render.delta_y;
                map_y += step_y;
                side = 1;
            }
            
            if (0 > map_x || map_x >= LEVEL_WIDTH || 0 > map_y || map_y >= LEVEL_HEIGHT) {
                hit = 1;
                break;
            }
            
            g_render.wall_type = getBlockAt(level, map_x, map_y);
            if (g_render.wall_type >= E_WALL_TYPE_1) {
                // Door collision
                if (g_render.wall_type == E_DOOR) {
                    uint8_t door_state = getDoorState(map_x, map_y);
                    if (door_state == S_OPEN) continue;
                    
                    if (door_state != S_CLOSE) {
                        float dist = (side == 0) ? (side_x - g_render.delta_x) : (side_y - g_render.delta_y);
                        float hit_pos = (side == 0) ? (player.pos.y + dist * g_render.rayDirY) : (player.pos.x + dist * g_render.rayDirX);
                        hit_pos -= (float)((int16_t)hit_pos);
                        
                        uint8_t door_timer = 0;
                        for (uint8_t i_door = 0; i_door < num_doors; i_door++) {
                            if (doors[i_door].x == map_x && doors[i_door].y == map_y) {
                                door_timer = doors[i_door].timer;
                                break;
                            }
                        }
                        
                        float current_offset = (float)(DOOR_ANIMATION_TIME - door_timer) * (0.5f / DOOR_ANIMATION_TIME);
                        if (door_state == S_CLOSING) {
                            current_offset = 0.5f - current_offset;
                        }
                        
                        if (hit_pos > (0.5f - current_offset) && (0.5f + current_offset) > hit_pos) {
                            continue;
                        }
                        g_render.doorOffset = current_offset;
                    }
                }
                hit = 1;
                break;
            }
        }
        
        // Render wall
        if (hit) {
            g_render.perpWallDist = (side == 0) ? (side_x - g_render.delta_x) : (side_y - g_render.delta_y);
            if (MIN_WALL_DISTANCE > g_render.perpWallDist) {
                g_render.perpWallDist = MIN_WALL_DISTANCE;
            }
            
            g_render.wall_x = (side == 0) ? (player.pos.y + g_render.perpWallDist * g_render.rayDirY) : (player.pos.x + g_render.perpWallDist * g_render.rayDirX);
            g_render.wall_x -= (float)((int16_t)g_render.wall_x);
            
            g_render.flipped = ((side == 0 && g_render.rayDirX > 0.0f) || (side == 1 && 0.0f > g_render.rayDirY));
            g_render.inv_dist = 1.0f / g_render.perpWallDist;
            
            int16_t line_height = (int16_t)(RENDER_HEIGHT * g_render.inv_dist);
            int16_t draw_start = half_h + (int16_t)(v_height_int * g_render.inv_dist) - (line_height >> 1);
            
            int16_t ds = (0 > draw_start) ? 0 : draw_start;
            int16_t de = draw_start + line_height;
            if (de >= RENDER_HEIGHT) de = RENDER_HEIGHT - 1;
            
            if (de > ds && RENDER_HEIGHT > ds && RENDER_HEIGHT > de) {
                drawTexturedVLine(x, (uint8_t)ds, (uint8_t)de, level, (uint8_t)map_x, (uint8_t)map_y);
            }
        } else if (RENDER_HEIGHT > half_h) {
            drawTexturedVLine(x, half_h, half_h, level, 0, 0);
        }
    }
}

//=============================================================================
// VISIBILITY CHECK (LINE OF SIGHT)
//=============================================================================

bool isVisible(float x1, float y1, float x2, float y2) {
    uint8_t ix = (uint8_t)(x1 * 8.0f);
    uint8_t iy = (uint8_t)(y1 * 8.0f);
    uint8_t ex = (uint8_t)(x2 * 8.0f);
    uint8_t ey = (uint8_t)(y2 * 8.0f);
    
    int8_t dx = (int8_t)(ex - ix);
    int8_t dy = (int8_t)(ey - iy);
    uint8_t abs_dx = (dx > 0) ? dx : -dx;
    uint8_t abs_dy = (dy > 0) ? dy : -dy;
    uint8_t steps = (abs_dx > abs_dy) ? abs_dx : abs_dy;
    
    if (2 > steps) return true;
    
    int8_t step_x = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
    int8_t step_y = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);
    
    const uint8_t *lvl_ptr = level_data[stateToLevel(game.state)];
    int8_t err = (int8_t)(abs_dx - abs_dy);
    
    while (steps--) {
        int8_t e2 = err << 1;
        if (e2 > -abs_dy) {
            err -= abs_dy;
            ix += step_x;
        }
        if (abs_dx > e2) {
            err += abs_dx;
            iy += step_y;
        }
        
        uint8_t cx = ix >> 3;
        uint8_t cy = iy >> 3;
        uint8_t block = getBlockAt(lvl_ptr, cx, cy);
        
        if (block >= E_WALL_TYPE_1) {
            if (block == E_DOOR) {
                Door* door = getDoorAt(cx, cy);
                if (door != 0 && door->state == S_OPEN) continue;
            }
            return false;
        }
    }
    return true;
}

//=============================================================================
// MELEE ATTACK POSITION TABLES
//=============================================================================

const int8_t MELEE_LEFT_X[] PROGMEM = {0, -8, -8, 0};
const int8_t MELEE_LEFT_Y[] PROGMEM = {0, 6, 6, 0};
const int8_t MELEE_RIGHT_X[] PROGMEM = {0, -16, -16, 0};
const int8_t MELEE_RIGHT_Y[] PROGMEM = {0, 0, 0, 0};

//=============================================================================
// RENDER GUN (WEAPON VIEW MODEL)
//=============================================================================

void renderGun(int8_t y, bool is_moving, int8_t roll_x) {
    uint8_t w_phase = (g_tick_counter >> 3) & 7;
    int8_t space_wave = (4 > w_phase) ? (int8_t)(w_phase >> 1) : (int8_t)((8 - w_phase) >> 1);
    space_wave -= 1;
    if (melee_attacking != 0) space_wave = 0;
    
    uint8_t total_x = 48 + roll_x + g_render.shake_x;
    uint8_t total_y = y + space_wave + g_render.shake_y;
    
    // Secret level: show spaceship
    if (game.is_secret_level > 0) {
        drawSprite(48 + roll_x + g_render.shake_x, total_y, bmp_spaceship_bits, 0, 32, 16, 32, 16, 0, 0);
        return;
    }
    
    uint8_t cw = player.current_weapon;
    if (cw > 3) return;
    
    uint8_t t = player.shoot_timer;
    uint8_t has_box = (player.ammo[WEAPON_FIST] > 0);
    uint8_t is_holding_box = (0 == cw) && has_box && !melee_attacking;
    
    const uint8_t *bits = (const uint8_t*)pgm_read_word(&(weapon_res[cw].bits));
    const uint8_t *mask = (const uint8_t*)pgm_read_word(&(weapon_res[cw].mask));
    
    // Draw weapon
    if (cw > 0) {
        drawSprite(total_x, total_y, bits, mask, 32, 28, 32, 28, 0, 0);
    } else {
        // Fist rendering with melee animation
        uint8_t fx[2];
        uint8_t fy[2];
        fx[0] = total_x + 20;
        fy[0] = 42 + g_render.shake_y;
        fx[1] = total_x - 16;
        fy[1] = 42 + g_render.shake_y;
        
        if (melee_attacking != 0) {
            uint8_t phase = melee_timer;
            if (phase > 3) phase = 3;
            int8_t lx = (int8_t)pgm_read_byte(&MELEE_LEFT_X[phase]);
            int8_t ly = (int8_t)pgm_read_byte(&MELEE_LEFT_Y[phase]);
            int8_t rx = (int8_t)pgm_read_byte(&MELEE_RIGHT_X[phase]);
            int8_t ry = (int8_t)pgm_read_byte(&MELEE_RIGHT_Y[phase]);
            
            fx[1] += lx;
            fy[1] += ly;
            fx[0] += rx;
            fy[0] += ry;
            
            if (fy[0] > 55) fy[0] = 55;
            if (fy[1] > 55) fy[1] = 55;
            if (6 > fx[0]) fx[0] = 6;
            if (6 > fx[1]) fx[1] = 6;
            if (fx[0] > 90) fx[0] = 90;
            if (fx[1] > 90) fx[1] = 90;
        }
        
        for (uint8_t hand = 0; 2 > hand; hand++) {
            drawSprite(fx[hand], fy[hand], bits, mask, 28, 28, 28, 28, hand, 0);
        }
    }
    
    // Draw held box
    if (is_holding_box != 0) {
        int8_t box_swing_y = 0;
        int8_t box_swing_x = 0;
        if (t > 5) {
            box_swing_y = -14 + (8 - t);
            box_swing_x = -6 + (8 - t);
        } else if (t > 2) {
            box_swing_y = 6 - (t - 2) * 2;
            box_swing_x = 8 + (t - 2) * 2;
        } else if (t > 0) {
            box_swing_y = -2;
            box_swing_x = 16;
        }
        drawSprite(total_x + box_swing_x, total_y - 2 + box_swing_y, bmp_box_bits, bmp_box_mask, 16, 16, 32, 32, 0, 0);
    }
}

//=============================================================================
// MENU STRINGS
//=============================================================================

const char menu_start[] PROGMEM = "START";
const char menu_easy[] PROGMEM = "LEVEL EASY";
const char menu_norm[] PROGMEM = "LEVEL NORM";
const char menu_hard[] PROGMEM = "LEVEL HARD";
const char menu_m_off[] PROGMEM = "MUSIC OFF";
const char menu_m_on[] PROGMEM = "MUSIC ON";

const char* const MENU_STRINGS[] PROGMEM = {
    menu_start, menu_easy, menu_norm, menu_hard
};

static const char* const FINAL_GOOD_MS[] PROGMEM = {
    msg_final_good1, msg_final_good2, msg_final_good3
};

static const uint8_t FINAL_GOOD_XO[] PROGMEM = {35, 55, 45};

//=============================================================================
// DRAW BEGIN/END SCREENS (INTRO/LOSE/WIN)
//=============================================================================

// Draw intro/lose/win screens with animations
// Handles the title screen menu, intro cinematic, game over screens, and victory screen
void drawBeginEnd(bool isIntro)
{
    static uint8_t st, cur, lock, tm, init_done = 0;
    static uint8_t last_btn = 0xFF;
    static uint16_t f, sc;
    // explosion_frame variable removed for maximum code compression
    
    uint8_t i;
    uint8_t *b;
    uint8_t next_state = 0;  // Register flag for single-point state transitions
    
    // Initialize static variables on first call
    if (0 == init_done) {
        f = st = sc = tm = cur = 0;
        lock = 1;
        last_btn = 0xFF;
        init_done = 1;
    }
    
    f++;  // Increment frame counter
    display.clearDisplay();
    
    // ============ INPUT HANDLING ============
    uint8_t current_btn = 0;
    if (input_fire()) { current_btn |= 1; }
    if (input_up())   { current_btn |= 2; }
    if (input_down()) { current_btn |= 4; }
    
    uint8_t pressed = current_btn & ~last_btn;
    last_btn = current_btn;
    
    if (0 == current_btn) {
        lock = 0;  // Release lock when no buttons pressed
    }
    
    // Fire button: advance to next state
    if ((pressed & 1) && 0 == lock) {
        if (!isIntro || st == 1) {
            next_state = isIntro ? 1 : 2;  // 1 -> STATE_LEVEL_1, 2 -> STATE_INTRO
        }
    }
    
    uint8_t w = (f & 8) >> 3;  // Wing animation frame for spaceship
    b = display.getBuffer();
    
    if (0 == next_state) {
        // ============ BACKGROUND STARS ============
        if (st != 0 || !isIntro) {
            uint8_t s = (uint8_t)(f >> 1);
            for (i = 16; i--;) {
                uint8_t r = pgm_read_byte(&font5x5[(uint8_t)(i * 7) & 127]);
                uint8_t sx = (((i & 3) << 5) + (r & 31) - s) & 127;
                uint8_t sy = (((i & 12) << 2) + (r >> 4)) & 63;
                b[(uint16_t)sx + (((uint16_t)sy >> 3) << 7)] |= (1 << (sy & 7));
            }
        }
        
        // ============ INTRO MENU SCREEN ============
        if (isIntro && 0 == st) {
            // Draw game logos
            drawSprite(10, 1, bmp_logo_s_bits, 0, BMP_LOGO_S_WIDTH, BMP_LOGO_S_HEIGHT, 
                       BMP_LOGO_S_WIDTH, BMP_LOGO_S_HEIGHT, 0, 0);
            drawSprite(58, 10, bmp_logo_p_bits, 0, BMP_LOGO_P_WIDTH, BMP_LOGO_P_HEIGHT, 
                       BMP_LOGO_P_WIDTH, BMP_LOGO_P_HEIGHT, 0, 0);
            
            // Menu navigation
            if (0 == lock && pressed != 0) {
                if (pressed & 2) {  // Up
                    cur = (0 == cur) ? 2 : cur - 1;
                } else if (pressed & 4) {  // Down
                    cur = (2 == cur) ? 0 : cur + 1;
                } else if (pressed & 1) {  // Fire/Select
                    if (0 == cur) {
                        // Start game
                        st = 1;
                        f = sc = tm = 0;
                        lock = 1;
                    } else if (1 == cur) {
                        // Cycle difficulty
                        game.difficulty = (game.difficulty == 2) ? 0 : game.difficulty + 1;
                    } else {
                        // Toggle music
                        game.music_on = !game.music_on;
                    }
                }
            }
            
            // Draw menu items
            drawText5x5(34, 33, (const char*)pgm_read_ptr(&MENU_STRINGS), 0xFFFF);
            uint8_t diff_idx = game.difficulty + 1;
            drawText5x5(34, 42, (const char*)pgm_read_ptr(&MENU_STRINGS[diff_idx]), 0xFFFF);
            drawText5x5(34, 51, game.music_on ? menu_m_on : menu_m_off, 0xFFFF);
            
            // Draw selection cursor
            uint8_t cur_y = 33 + cur * 9;
            uint16_t idx = 26 + (((uint16_t)cur_y >> 3) << 7);
            uint8_t sh = cur_y & 7;
            b[idx++] |= (0x1F << sh);
            b[idx++] |= (0x0E << sh);
            b[idx]   |= (0x04 << sh);
        } 
        else {
            // ============ INTRO CINEMATIC ============
            if (isIntro) {
                int16_t ty = 64 - f;
                // Scrolling text "FAR FAR AWAY"
                if (ty > -10 && 64 > ty) { 
                    drawText5x5(36, ty, msg_intro_1, 0xFFFF); 
                }
                // Scrolling text "NEAR TAIRIYA"
                if (ty + 12 > -10 && 64 > ty + 12) { 
                    drawText5x5(35, ty + 12, msg_intro_2, 0xFFFF); 
                }
                
                // Spaceship approaches after text scrolls off
                if (-12 > ty) {
                    if (100 > tm) { tm++; }
                    int8_t sy = (64 - tm > 46) ? 64 - tm : 46;
                    drawSprite(48, sy + w, bmp_spaceship_bits, 0, 32, 16, 32, 16, 0, 0);
                    
                    // Animated potatoes appear from sides when ship stops
                    if (sy == 46) {
                        sc += 4;
                        uint8_t sz = (uint8_t)(((uint32_t)26 * sc) >> 8);
                        if (sz > 2) {
                            // BRANCHLESS WALK CYCLE TRIGGER:
                            // Pass (f & 8) >> 3 as the mirror argument to drawSprite.
                            // Every 8 frames the potato sprites flip horizontally,
                            // creating a smooth walking animation without code bloat!
                            uint8_t walk_mirror = (f & 8) >> 3;
                            
                            drawSprite(2, 18, bigpotato_bits, bigpotato_mask, 
                                       SPR_BIBPOTATO_WH, SPR_BIBPOTATO_WH, sz, sz, walk_mirror, 0);
                            drawSprite(126 - sz, 18, bigpotato_bits, bigpotato_mask, 
                                       SPR_BIBPOTATO_WH, SPR_BIBPOTATO_WH, sz, sz, walk_mirror, 0);
                        }
                        if (sc > 270) {
                            next_state = 1;  // Start game
                        }
                    }
                }
            } 
            else {
                // ============ LOSE / WIN SCREENS ============
                if (f == 1) {
                    st = 0;
                    sc = 256;
                    tm = 0;
                    lock = 1;
                }
                
                // ============ LOSE SCREEN ============
                if (game.state == STATE_LOSE) {
                    // Phase 1: Ship falling (0-20 frames)
                    if (20 > tm) {
                        tm++;
                        drawSprite(64 - (sc >> 4), 46 + w, bmp_spaceship_bits, 0, 32, 16, 
                                   sc >> 3, sc >> 4, 0, 0);
                    } 
                    // Phase 2: Explosion animation (20-33 frames)
                    else if (33 > tm) { 
                        tm++;
                        uint8_t boom_progress = tm - 20;
                        uint8_t boom_size = (12 > boom_progress) ? (10 + (boom_progress << 1)) : 0;
                        
                        if (boom_size > 0) {
                            drawSprite(64 - (boom_size >> 1), (46 + w) - (boom_size >> 1),
                                       eff_splash_bmp, eff_splash_mask,
                                       15, 15,
                                       boom_size, boom_size, boom_progress & 1, (boom_progress >> 2) & 1);
                                       
                            // Screen flash during explosion
                            if (4 > boom_progress) {
                                flash_screen = 2;
                            }
                        }
                    }
                    // Phase 3: Aftermath (33-52 frames)
                    else if (52 >= tm) {
                        tm++;
                    }
                    // Phase 4: "WASTED" text appears (52+ frames)
                    else {
                        if (255 > tm) { tm++; }
                        
                        int8_t ty = -15 + (tm - 52);
                        if (ty > 18) { ty = 18; }
                        
                        drawText5x5(45, ty, msg_final_bad, 0xFFFF);
                        
                        if (tm > 210) {
                            next_state = 2;  // Return to intro
                        }
                    }
                }
                // ============ WIN SCREEN ============
                else {
                    // Phase 1: Ship flies away
                    if (0 == st) {
                        sc -= 10;
                        if (10 > sc) {
                            st = 1;
                            tm = 0;
                        }
                        drawSprite(64 - (sc >> 4), 46 + w, bmp_spaceship_bits, 0, 32, 16, 
                                   sc >> 3, sc >> 4, 0, 0);
                    } 
                    // Phase 2: Victory text appears
                    else {
                        if (255 > tm) { tm++; }
                        int8_t ty = (-25 + tm > 15) ? 15 : -25 + tm;
                        
                        // Display three lines: "TO INFINITY", "AND", "BEYOND"
                        for (i = 0; 3 > i; i++) {
                            uint8_t xo = pgm_read_byte(&FINAL_GOOD_XO[i]);
                            const char *msg = (const char*)pgm_read_word(&FINAL_GOOD_MS[i]);
                            drawText5x5(xo, ty + (i << 3), msg, 0xFFFF);
                        }
                        
                        if (tm > 200) {
                            next_state = 2;  // Return to intro
                        }
                    }
                }
            }
        }
        display.display();
    }

    // ============================================================================
    // SINGLE STATE TRANSITION POINT
    // All duplicate register resets removed for maximum code compression
    // ============================================================================
    if (next_state != 0) {
        lock = 1;
        f = st = sc = tm = cur = 0;
        init_done = 0;
        (next_state == 1) ? jumpTo(STATE_LEVEL_1) : jumpTo(STATE_INTRO);
    }
}


//=============================================================================
// INTRO/FINAL LOOP
//=============================================================================

void loopIntroFinal() {
    display.clearDisplay();
    bool isIntro = (game.state == STATE_INTRO);
    drawBeginEnd(isIntro);
    display.display();
    delay(15);
}

//=============================================================================
// WEAPON CHANGE HANDLER
//=============================================================================

void handleWeaponChange() {
    static uint8_t weapon_button_pressed = 0;
    
    if(!input_use()) {
        weapon_button_pressed = 0;
        return;
    }
    if(weapon_button_pressed) return;
    if(game.is_secret_level) return;
    
    weapon_button_pressed = 1;
    uint8_t w = player.current_weapon;
    
    for(uint8_t i = 0; 4 > i; i++) {
        w = (w + 1) & 3;
        if(w == 0 || (player.weapons_unlocked & (1 << w))) {
            player.current_weapon = w;
            break;
        }
    }
    playSound(snd_item_pickup);
}

//=============================================================================
// CHEAT CODE SYSTEM
//=============================================================================

const uint8_t CHEAT_SEQ[] PROGMEM = {3, 3, 4, 4, 2, 5, 2, 5, 7, 8};
static uint8_t cheat_idx = 0;
static uint16_t last_input_t = 0;
static uint8_t ch_btn_lock = 0;
static uint8_t m_lock = 0;

void handleCheatCode(void) {
    uint16_t now = (uint16_t)g_tick_counter;
    uint8_t d = (uint8_t)(~PIND);
    uint8_t b = (uint8_t)(~PINB);
    uint8_t cur = 0;
    
    if (d & 0xBC) {
        if (d & _BV(3)) cur = 3;
        else if (d & _BV(4)) cur = 4;
        else if (d & _BV(2)) cur = 2;
        else if (d & _BV(5)) cur = 5;
        else if (d & _BV(7)) cur = 7;
    } else if (b & _BV(0)) {
        cur = 8;
    }
    
    if (0 == cur) {
        ch_btn_lock = 0;
        return;
    }
    if (ch_btn_lock != 0 || (3 > (uint16_t)(now - last_input_t))) return;
    if ((uint16_t)(now - last_input_t) > 60) cheat_idx = 0;
    
    last_input_t = now;
    ch_btn_lock = 1;
    
    if (cur == pgm_read_byte(&CHEAT_SEQ[cheat_idx])) {
        cheat_idx++;
        if (cheat_idx >= sizeof(CHEAT_SEQ)) {
            cheat_menu_showing = true;
            cheat_idx = 0;
            m_lock = 1;
            flash_screen = 4;
        }
    } else {
        cheat_idx = (cur == pgm_read_byte(&CHEAT_SEQ[0])) ? 1 : 0;
    }
}

void handleCheatMenu(void) {
    uint8_t d = (uint8_t)(~PIND);
    uint8_t b = (uint8_t)(~PINB);
    uint8_t fire_use = (d & _BV(7)) || (b & _BV(0));
    uint8_t any = (d & 0x3C) || fire_use;
    
    if (0 == any) {
        m_lock = 0;
        return;
    }
    if (m_lock != 0) return;
    
    // Navigation
    if (d & _BV(3)) {
        if (0 == cheat_menu_selection) cheat_menu_selection = 5;
        else cheat_menu_selection--;
        m_lock = 1;
    } else if (d & _BV(4)) {
        if (5 == cheat_menu_selection) cheat_menu_selection = 0;
        else cheat_menu_selection++;
        m_lock = 1;
    }
    
    // Selection
    if (fire_use != 0) {
        if (3 >= cheat_menu_selection) {
            game.state = STATE_LEVEL_1 + cheat_menu_selection;
            game.is_secret_level = (cheat_menu_selection == 3);
            game.level_initialized = false;
            initializeLevel(cheat_menu_selection);
        } else if (cheat_menu_selection == 4) {
            player.health = 100;
            for (uint8_t i = 0; 4 > i; i++) {
                player.ammo[i] = 99;
            }
            player.weapons_unlocked = 0x0F;
            flash_screen = 2;
        }
        cheat_menu_showing = false;
        m_lock = 1;
    }
}

//=============================================================================
// EFFECT WEAPON BITMAPS
//=============================================================================

const uint8_t* const EFF_WEAPON_BMPS[] PROGMEM = {
    eff_splash_bmp, eff_blaster_bmp, eff_cutter_bmp, eff_bfg_bmp
};

const uint8_t* const EFF_WEAPON_MSKS[] PROGMEM = {
    eff_splash_mask, eff_blaster_msk, eff_cutter_msk, eff_bfg_msk
};

//=============================================================================
// RENDER WORLD SPRITE
//=============================================================================

bool renderWorldSprite(float wx, float wy, float inv_det, const void* raw_data, int16_t base_size_scale) {
    const SpriteRenderData* data = (const SpriteRenderData*)raw_data;
    if (0 == data->bits) return false;
    
    float dx = wx - player.pos.x;
    float dy = wy - player.pos.y;
    
    float ty = inv_det * (-player.plane.y * dx + player.plane.x * dy);
    if (0.15f >= ty || ty > 10.0f) return false;
    
    float tx = inv_det * (player.dir.y * dx - player.dir.x * dy);
    float abs_tx = (tx > 0.0f) ? tx : -tx;
    if (abs_tx > ty * 3.0f) return false;
    if (1 > isVisible(wx, wy, player.pos.x, player.pos.y)) return false;
    
    float inv_ty = 1.0f / ty;
    float screen_tx = 64.0f * tx * inv_ty;
    if (screen_tx > 250.0f) screen_tx = 250.0f;
    if (-250.0f > screen_tx) screen_tx = -250.0f;
    
    int16_t sx = 64 + (int16_t)screen_tx;
    int16_t abs_scale = (base_size_scale > 0) ? base_size_scale : -base_size_scale;
    
    float calculated_h = (float)abs_scale * inv_ty * 1.15f;
    if (calculated_h > 128.0f) calculated_h = 128.0f;
    if (calculated_h < 4.0f) calculated_h = 4.0f;
    
    int16_t draw_h = (int16_t)calculated_h;
    int16_t draw_w = draw_h;
    
    // Special handling for wide sprites
    if (data->src_w == 32 && data->src_h == 16) {
        draw_w = draw_h << 1;
        if (draw_w > 128) draw_w = 128;
    }
    
    uint8_t safe_sx = (sx > 0) ? ((sx < 128) ? sx : 127) : 0;
    uint8_t z_idx = safe_sx >> 4;
    uint8_t z_val = (uint8_t)(ty * 8.0f);
    if (ZBUFFER_SIZE > z_idx && z_val > zbuffer[z_idx]) return false;
    
    int16_t screen_y = 32 - (draw_h >> 1);
    
    // Entity-specific positioning
    Entity* e = (Entity*)g_current_render_entity;
    if (e != 0) {
        uint8_t enemy_type = e->type;
        uint8_t props = pgm_read_byte(&ENEMY_PROPERTIES_TABLE[enemy_type]);
        
        // Flying enemies bob
        if (props & MASK_FLYING) {
            uint8_t entity_phase = g_tick_counter + 8 + ((uint8_t)(wx * 8.0f) ^ (uint8_t)(wy * 8.0f));
            uint8_t item_phase = (entity_phase >> 1) & 15;
            int8_t item_wave = (8 > item_phase) ? (int8_t)(item_phase >> 1) : (int8_t)((16 - item_phase) >> 1);
            item_wave -= 2;
            screen_y += item_wave;
        }
        
        // Enemy-specific Y offsets
        switch(enemy_type) {
            case E_NUB: screen_y -= (draw_h >> 1) + (draw_h >> 2); break;
            case E_EASY:
            case E_BOSS_1:
            case E_BOSS_2:
            case E_BOSS_3:
            case E_BIGPOTATO:
                screen_y += (draw_h >> 2); break;
            case E_MEDIUM:
            case E_HARD:
                break;
        }
    } else {
        // Item bobbing
        uint8_t entity_phase = g_tick_counter + 8 + ((uint8_t)(wx * 8.0f) ^ (uint8_t)(wy * 8.0f));
        uint8_t item_phase = (entity_phase >> 1) & 15;
        int8_t item_wave = (8 > item_phase) ? (int8_t)(item_phase >> 1) : (int8_t)((16 - item_phase) >> 1);
        item_wave -= 2;
        if (data->bits == bmp_well_bits) {
            screen_y += 4;
        } else {
            screen_y += item_wave;
        }
    }
    
    // Draw sprite
    if (0 == e || e->state != S_DYING) {
        drawSprite(sx - (draw_w >> 1), screen_y, data->bits, data->mask, data->src_w, data->src_h, draw_w, draw_h, data->mirror, data->invert);
    }
    
    // Draw damage/death effect overlay
    if (e != 0) {
        uint8_t clean_timer = e->timer & 0x0F;
        if (clean_timer > 0 && (e->state == S_DAMAGED || e->state == S_DYING)) {
            const uint8_t *eff_bmp = 0;
            const uint8_t *eff_msk = 0;
            uint8_t tex_size = 15;
            uint8_t base_t = 6;
            uint8_t max_size = 32;
            
            if (e->state == S_DYING) {
                eff_bmp = eff_splash_bmp;
                eff_msk = eff_splash_mask;
                base_t = 12;
                max_size = 32;
            } else {
                uint8_t w_type = (e->timer >> 4) & 3;
                eff_bmp = (const uint8_t*)pgm_read_word(&EFF_WEAPON_BMPS[w_type]);
                eff_msk = (const uint8_t*)pgm_read_word(&EFF_WEAPON_MSKS[w_type]);
                if (w_type == 3) tex_size = 24;
            }
            
            uint8_t progress = base_t - clean_timer;
            int16_t eff_size = (draw_h >> 2) + 6 + (progress << 1);
            if (eff_size > max_size) eff_size = max_size;
            if (10 > eff_size) eff_size = 10;
            
            uint8_t z_val_eff = (z_val > 1) ? (z_val - 2) : 0;
            if (ZBUFFER_SIZE > z_idx && zbuffer[z_idx] >= z_val_eff) {
                drawSprite(sx - (eff_size >> 1), (screen_y + (draw_h >> 1)) - (eff_size >> 1), eff_bmp, eff_msk, tex_size, tex_size, eff_size, eff_size, (progress & 1), 0);
            }
        }
    }
    
    return true;
}


//=============================================================================
// PLAYER MOVEMENT VARIABLES
//=============================================================================

int16_t i_speed = 0;
int16_t idx256 = 256;
int16_t idy256 = 0;
uint8_t dir_initialized = 0;

//=============================================================================
// MAIN GAME LOOP
//=============================================================================

void loopGamePlay(void) {
    float det, inv_det;
    uint8_t in = 0;
    SpriteRenderData srd;
    int8_t sz = 48;
    
    // Cheat handling
    handleCheatCode();
    if (cheat_menu_showing) {
        handleCheatMenu();
        drawCheatMenu();
        return;
    }
    
    handleWeaponChange();
    display.clearDisplay();
    memset(zbuffer, 0xFF, ZBUFFER_SIZE);
    
    // Initialize direction
    if (1 > dir_initialized) {
        idx256 = (int16_t)(player.dir.x * 256.0f);
        idy256 = (int16_t)(player.dir.y * 256.0f);
        dir_initialized = 1;
    }
    
    // Input handling
    if (input_up()) in |= 1;
    if (input_down()) in |= 2;
    if (input_left()) in |= 4;
    if (input_right()) in |= 8;
    
    // Rotation
    if (in & 12) {
        float rot_sign = (in & 4) ? -1.0f : 1.0f;
        float odx = player.dir.x;
        float current_cos = ROT_COS;
        float current_sin = ROT_SIN;
        
        if (game.is_secret_level) {
            current_cos = 0.99968f;
            current_sin = 0.02499f;
        }
        
        float r_sin = rot_sign * current_sin;
        player.dir.x = odx * current_cos + player.dir.y * r_sin;
        player.dir.y = -odx * r_sin + player.dir.y * current_cos;
        
        float opx = player.plane.x;
        player.plane.x = opx * current_cos + player.plane.y * r_sin;
        player.plane.y = -opx * r_sin + player.plane.y * current_cos;
        
        idx256 = (int16_t)(player.dir.x * 256.0f);
        idy256 = (int16_t)(player.dir.y * 256.0f);
    }
    
    // Movement
    uint8_t step_accel = game.is_secret_level ? 40 : 120;
    if (in & 1) i_speed += step_accel;
    if (in & 2) i_speed -= step_accel;
    
    int16_t max_speed_limit = game.is_secret_level ? 1000 : 2000;
    if (i_speed > max_speed_limit) i_speed = max_speed_limit;
    if (-max_speed_limit > i_speed) i_speed = -max_speed_limit;
    
    float f_move = (float)i_speed * 0.0009765625f;
    float fx = player.dir.x * f_move;
    float fy = player.dir.y * f_move;
    
    float new_x = player.pos.x + fx;
    float new_y = player.pos.y + fy;
    float check_x = new_x + WALL_DIST * (fx > 0 ? 1.0f : -1.0f);
    float check_y = new_y + WALL_DIST * (fy > 0 ? 1.0f : -1.0f);
    
    // Collision detection and resolution
    if (1 > checkCollisionAtPoint(check_x, check_y, 1)) {
        player.pos.x = new_x;
        player.pos.y = new_y;
    } else {
        bool moved = false;
        if (1 > checkCollisionAtPoint(new_x, player.pos.y, 1)) {
            player.pos.x = new_x;
            moved = true;
        }
        if (1 > checkCollisionAtPoint(player.pos.x, new_y, 1)) {
            player.pos.y = new_y;
            moved = true;
        }
        if (!moved) i_speed = 0;
    }
    
    i_speed = (int16_t)(((int32_t)i_speed * 450) >> 10);
    if (!in && 25 > (i_speed > 0 ? i_speed : -i_speed)) i_speed = 0;
    
    g_tick_counter++;
    
    // Update game objects
    updateThrowables();
    handleShooting();
    updateMeleeAnimation();
    collectItems();
    
    // Check level exit
    uint8_t player_x = (uint8_t)player.pos.x;
    uint8_t player_y = (uint8_t)player.pos.y;
    const uint8_t *m_ptr = level_data[stateToLevel(game.state)];
    uint8_t block = getBlockAt(m_ptr, player_x, player_y);
    
    if (block == E_EXIT) {
        if (game.is_secret_level) {
            game.state = STATE_WIN;
            game.isVictory = true;
            game.level_initialized = 0;
            return;
        } else {
            game.level_initialized = 0;
            uint8_t current_level = stateToLevel(game.state);
            if (2 > current_level) {
                game.state = levelToState(current_level + 1, false);
            } else if (easter_egg_count >= 3) {
                game.state = STATE_SECRET;
                game.is_secret_level = true;
            } else {
                game.state = STATE_WIN;
                game.isVictory = true;
            }
            return;
        }
    }
    
    handleDoorInteraction();
    updateEntities(m_ptr);
    
    // Player death check
    if (0 >= player.health) {
        game.state = STATE_LOSE;
        game.level_initialized = 0;
        flash_screen = 0;
        return;
    }
    
    updateProjectiles(m_ptr);
    updateEffects();
    
    // Render world
    renderMap(m_ptr);
    
    // Render entities (enemies)
    det = player.plane.x * player.dir.y - player.dir.x * player.plane.y;
    if ((det > 0 ? det : -det) > 0.00009f) {
        inv_det = 1.0f / det;
        uint8_t tick = g_tick_counter;
        
        for (uint8_t i = 0; MAX_ENTITIES > i; i++) {
            Entity *e = &entity[i];
            uint8_t typ = e->type;
            if (!is_enemy(typ) || e->state == S_DEAD) continue;
            
            float dx = e->pos.x - player.pos.x;
            float dy = e->pos.y - player.pos.y;
            if (dx * dx + dy * dy > 400.0f) continue;
            
            srd.invert = 0;
            srd.mirror = 0;
            g_current_render_entity = e;
            
            if (e->state == S_DAMAGED) {
                srd.invert = ((tick >> 1) & 1);
            }
            
            if (e->state == S_ATTACK) {
                srd.bits = hit_character_bits;
                srd.mask = hit_character_mask;
                srd.src_w = 24;
                srd.src_h = 24;
                sz = 32;
                srd.mirror = ((tick >> 3) & 1) ? 1 : 0;
                renderWorldSprite(e->pos.x, e->pos.y, inv_det, &srd, sz);
                continue;
            }
            
            if (e->state == S_DYING) {
                srd.bits = eff_splash_bmp;
                srd.mask = eff_splash_mask;
                srd.src_w = SPR_DEAD_W;
                srd.src_h = SPR_DEAD_H;
                sz = 24;
                renderWorldSprite(e->pos.x, e->pos.y, inv_det, &srd, sz);
                continue;
            }
            
            // Get enemy sprite data
            uint8_t idx = 255;

            // Check for the boss group range: BOSS_1 to BOSS_3
            if (typ >= E_BOSS_1 && E_BOSS_3 >= typ) {
                idx = (uint8_t)((uint8_t)(typ - E_BOSS_1) + 4);
            } else if (typ == E_BIGPOTATO) {
                idx = 7;
            } else if (typ >= E_NUB && E_HARD >= typ) {
                // If the enum values are sequential (NUB=0, EASY=1, MEDIUM=2, HARD=3),
                // the compiler reduces this arithmetic expression to just two assembly instructions:
                idx = (uint8_t)(typ - E_NUB);
            }
            
            if (idx != 255) {
                uint16_t base = idx * 5;
                srd.bits = (const uint8_t*)pgm_read_word(&ENEMY_META[base]);
                srd.mask = (const uint8_t*)pgm_read_word(&ENEMY_META[base + 1]);
                srd.src_w = (uint8_t)pgm_read_word(&ENEMY_META[base + 2]);
                srd.src_h = (uint8_t)pgm_read_word(&ENEMY_META[base + 3]);
                sz = (int16_t)pgm_read_word(&ENEMY_META[base + 4]);
                
                if (e->state != S_DAMAGED && e->state != S_DYING) {
                    uint8_t anim_phase = ((tick >> 2) + (typ << 1)) & 3;
                    srd.mirror = (anim_phase >= 2) ? 1 : 0;
                } else {
                    srd.mirror = 0;
                }
                renderWorldSprite(e->pos.x, e->pos.y, inv_det, &srd, sz);
            }
        }
        
        // Render items
        static uint8_t well_anim_loc = 0;
        uint8_t well_mirror = (well_anim_loc & 8) ? 1 : 0;
        well_anim_loc++;
        g_current_render_entity = 0;
        
        for (uint8_t i = 0; num_items > i; i++) {
            Item *item = &items[i];
            if (item->collected) continue;
            
            uint8_t typ = item->type;
            float wx = item->x + 0.5f;
            float wy = item->y + 0.5f;
            srd.mirror = 0;
            srd.invert = 0;
            
            uint8_t idx = 255;
            switch (typ) {
                case E_MEDKIT_FLOOR: idx = 0; break;
                case E_AMMO_FLOOR: idx = 1; break;
                case E_METAL_BOX: idx = 2; break;
                case E_SEPULKI: idx = 3; break;
                case E_WEAPON_BLASTER: idx = 4; break;
                case E_WEAPON_PLASMA_CUTTER: idx = 5; break;
                case E_WEAPON_BFG9000: idx = 6; break;
                case E_EASTER_EGG_BOOK_1: idx = 7; break;
                case E_EASTER_EGG_BOOK_2: idx = 8; break;
                case E_EASTER_EGG_BOOK_3: idx = 9; break;
                default: break;
            }
            
            if (idx != 255 && 11 > idx) {
                uint16_t base = idx * 5;
                srd.bits = (const uint8_t*)pgm_read_word(&ITEM_META[base]);
                srd.mask = (const uint8_t*)pgm_read_word(&ITEM_META[base + 1]);
                srd.src_w = (uint8_t)pgm_read_word(&ITEM_META[base + 2]);
                srd.src_h = (uint8_t)pgm_read_word(&ITEM_META[base + 3]);
                sz = (int16_t)pgm_read_word(&ITEM_META[base + 4]);
                
                if (typ == E_MEDKIT_FLOOR || typ == E_AMMO_FLOOR) {
                    srd.mirror = well_mirror;
                }
                if (srd.bits != 0) {
                    renderWorldSprite(wx, wy, inv_det, &srd, sz);
                }
            }
        }
        
        // Render throwables
        for (uint8_t i = 0; MAX_THROWABLES > i; i++) {
            if (!throwables[i].active) continue;
            srd.bits = bmp_box_bits;
            srd.mask = bmp_box_mask;
            srd.src_w = 16;
            srd.src_h = 16;
            srd.mirror = 0;
            srd.invert = 0;
            renderWorldSprite(throwables[i].x, throwables[i].y, inv_det, &srd, 24);
        }
    }
    
    renderProjectiles();
    renderEffects();
    
    // Render gun
    int8_t gun_y = (player.current_weapon != 0) ? 39 : 45;
    if (player.shoot_timer != 0) gun_y += 4;
    renderGun(gun_y, in & 3, 0);
    
    // Low health warning
    uint8_t hp = player.health;
    if (hp && 25 > hp) {
        uint8_t *b_buf = display.getBuffer();
        static uint8_t pulse_phase = 0;
        pulse_phase++;
        uint8_t pulse = (pulse_phase & 4) ? (pulse_phase & 3) : (3 - (pulse_phase & 3));
        uint16_t inner = 2800 + ((25 - hp) << 4) - (pulse * 250);
        uint16_t outer = inner + 700;
        
        for (uint8_t y = 0; 64 > y; y++) {
            int16_t dy = (int16_t)y - 32;
            uint16_t dy2 = (uint16_t)(dy * dy * 6);
            uint8_t mask = 1 << (y & 7);
            uint16_t row_offset = ((uint16_t)y >> 3) << 7;
            uint8_t *row_ptr = &b_buf[row_offset];
            
            for (uint8_t x = 0; 128 > x; x++) {
                int16_t dx = (int16_t)x - 64;
                uint16_t rad = (uint16_t)(dx * dx) + dy2;
                if (rad > inner) {
                    if (rad > outer) {
                        if ((x + y) & 1) row_ptr[x] &= ~mask;
                    } else {
                        if (!((x + y) & 3)) row_ptr[x] &= ~mask;
                    }
                }
            }
        }
    }
    
    drawHud(hp, player.current_weapon, (const uint8_t*)player.ammo);
    
    // Flash screen effect
    if (flash_screen != 0) {
        if (flash_screen & 1) {
            uint8_t *b_buf = display.getBuffer();
            for (uint16_t i_buf = 0; 1024 > i_buf; i_buf++) {
                b_buf[i_buf] ^= 0xFF;
            }
        }
        flash_screen--;
    }
    
    updateNotification();
    display.display();
}

//=============================================================================
// PROJECTILE CONSTANTS
//=============================================================================

const uint8_t PROJ_HIT_EFFECTS[] PROGMEM = {
    EFFECT_SPLASH_DAMAGE, EFFECT_SPLASH_DAMAGE, EFFECT_BLASTER_EXPLOSION,
    EFFECT_CUTTER_EXPLOSION, EFFECT_BFG_EXPLOSION
};

const uint8_t PROJ_TEX_SIZES[] PROGMEM = {12, 12, 16, 16, 24};

const uint8_t* const PROJ_BMPS[] PROGMEM = {
    proj_enemy_bmp, proj_enemy_bmp, eff_blaster_bmp, eff_cutter_bmp, eff_bfg_bmp
};

const uint8_t* const PROJ_MSKS[] PROGMEM = {
    proj_enemy_mask, proj_enemy_mask, eff_blaster_msk, eff_cutter_msk, eff_bfg_msk
};

//=============================================================================
// SPAWN PROJECTILE
//=============================================================================

void spawnProjectile(float start_x, float start_y, float dir_x, float dir_y, uint8_t proj_type) {
    for (uint8_t i = 0; MAX_PROJECTILES > i; i++) {
        if (1 > (projectiles[i].state >> 4)) {
            projectiles[i].x = start_x;
            projectiles[i].y = start_y;
            
            uint8_t is_player = 0;
            if (5 > proj_type && proj_type > 1) {
                is_player = 1;
            }
            
            float speed = 0.42f;
            projectiles[i].vx_dir = dir_x * speed;
            projectiles[i].vy_dir = dir_y * speed;
            
            uint8_t life_timer = 12;
            uint8_t height_code = 0;
            uint8_t packed_type = 0;
            
            if (is_player > 0) {
                height_code = 1;
                packed_type = proj_type - 1;
            } else {
                uint8_t enemy_id = proj_type - 10;
                if (enemy_id == E_NUB) {
                    height_code = 2;
                } else if (enemy_id == E_MEDIUM || enemy_id == E_HARD) {
                    height_code = 1;
                } else {
                    height_code = 0;
                }
                packed_type = 0;
            }
            
            projectiles[i].state = (life_timer << 4) | (height_code << 2) | packed_type;
            break;
        }
    }
}

//=============================================================================
// SPAWN PLAYER PROJECTILE
//=============================================================================

void spawnPlayerProjectile(uint8_t weapon_type) {
    float spawn_x = player.pos.x + player.dir.x * 0.25f;
    float spawn_y = player.pos.y + player.dir.y * 0.25f;
    uint8_t p_type = 2;
    if (weapon_type > 0 && 4 > weapon_type) {
        p_type = weapon_type + 1;
    }
    spawnProjectile(spawn_x, spawn_y, player.dir.x, player.dir.y, p_type);
}

//=============================================================================
// UPDATE PROJECTILES
//=============================================================================

void updateProjectiles(const uint8_t level[]) {
    const uint8_t *curr_level_ptr = level_data[stateToLevel(game.state)];
    
    for (uint8_t i = 0; MAX_PROJECTILES > i; i++) {
        Projectile *p = &projectiles[i];
        uint8_t raw_state = p->state;
        uint8_t timer = raw_state >> 4;
        uint8_t packed_type = raw_state & 0x03;
        
        if (1 > timer) continue;
        timer--;
        p->state = (timer << 4) | (raw_state & 0x0F);
        if (0 == timer) continue;
        
        float step_x = p->vx_dir * 0.5f;
        float step_y = p->vy_dir * 0.5f;
        uint8_t projectile_destroyed = 0;
        
        // Sub-step movement for collision accuracy
        for (uint8_t sub_step = 0; 2 > sub_step; sub_step++) {
            if (projectile_destroyed > 0) break;
            
            p->x += step_x;
            p->y += step_y;
            
            uint8_t bx = (uint8_t)p->x;
            uint8_t by = (uint8_t)p->y;
            
            if (bx >= LEVEL_WIDTH || by >= LEVEL_HEIGHT) {
                p->state = 0;
                projectile_destroyed = 1;
                continue;
            }
            
            // Wall collision
            uint8_t block = getBlockAt(curr_level_ptr, bx, by);
            uint8_t hit_wall = 0;
            if (block >= E_WALL_TYPE_1) {
                hit_wall = 1;
                if (block == E_DOOR) {
                    Door *d = getDoorAt(bx, by);
                    if (d != 0 && d->state == S_OPEN) {
                        hit_wall = 0;
                    }
                }
            }
            
            if (hit_wall > 0) {
                p->state = 0;
                projectile_destroyed = 1;
                playSound(snd_wall_hit);
                uint8_t orig_type = (0 == packed_type) ? 1 : (packed_type + 1);
                uint8_t eff = EFFECT_SPLASH_DAMAGE;
                if (4 >= orig_type) {
                    eff = pgm_read_byte(&PROJ_HIT_EFFECTS[orig_type]);
                }
                addEffect(eff, p->x, p->y);
                continue;
            }
            
            // Enemy projectile vs player
            if (0 == packed_type) {
                float dx = player.pos.x - p->x;
                float dy = player.pos.y - p->y;
                if (PROJECTILE_COLLIDER_DIST_SQ > (dx * dx + dy * dy)) {
                    p->state = 0;
                    projectile_destroyed = 1;
                    uint8_t dmg = PROJECTILE_DAMAGE_PLAYER;
                    if (game.difficulty == 2) {
                        dmg += (dmg >> 1);
                    } else if (0 == game.difficulty) {
                        dmg -= (dmg >> 2);
                    }
                    player.health = (player.health > dmg) ? (player.health - dmg) : 0;
                    flash_screen = 2;
                    playSound(snd_player_hurt);
                    g_render.shake_timer = SHAKE_DURATION_DAMAGE;
                    addEffect(EFFECT_SPLASH_DAMAGE, player.pos.x, player.pos.y);
                    continue;
                }
            } else {
                // Player projectile vs enemies
                for (uint8_t j = 0; num_entities > j; j++) {
                    Entity *e = &entity[j];
                    if (e->state >= S_DEAD || 0 == e->health || 0 == is_enemy(e->type)) continue;
                    
                    float edx = e->pos.x - p->x;
                    float edy = e->pos.y - p->y;
                    if (PROJECTILE_COLLIDER_DIST_SQ > (edx * edx + edy * edy)) {
                        p->state = 0;
                        projectile_destroyed = 1;
                        uint8_t weapon = packed_type;
                        applyDamage(e, weapon);
                        break;
                    }
                }
            }
        }
    }
}

//=============================================================================
// RENDER PROJECTILES
//=============================================================================

void renderProjectiles(void) {
    float det = player.plane.x * player.dir.y - player.dir.x * player.plane.y;
    uint32_t *det_raw = (uint32_t*)&det;
    uint32_t det_abs = *det_raw & 0x7FFFFFFF;
    float f_det_abs = *(float*)&det_abs;
    if (0.0001f > f_det_abs) return;
    
    float inv_det = 1.0f / det;
    
    for (uint8_t i = 0; MAX_PROJECTILES > i; i++) {
        Projectile* p = &projectiles[i];
        uint8_t raw_state = p->state;
        uint8_t timer = raw_state >> 4;
        if (1 > timer) continue;
        
        uint8_t height_code = (raw_state >> 2) & 0x03;
        uint8_t packed_type = raw_state & 0x03;
        
        float dx = p->x - player.pos.x;
        float dy = p->y - player.pos.y;
        float cam_y = inv_det * (-player.plane.y * dx + player.plane.x * dy);
        if (0.01f >= cam_y || cam_y > 10.0f) continue;
        
        float inv_y = 1.0f / cam_y;
        float cam_x = inv_det * (player.dir.y * dx - player.dir.x * dy);
        int16_t sx = 64 + (int16_t)(cam_x * 64.0f * inv_y);
        int16_t sy = 32 + (int16_t)(16.0f * inv_y);
        
        int8_t y_pixel_offset = 0;
        if (0 == height_code) {
            y_pixel_offset = 4;
        } else if (2 == height_code) {
            y_pixel_offset = -14;
        }
        sy += y_pixel_offset;
        
        if (sx < -10 || sx > 138 || sy < -10 || sy > 74) continue;
        
        uint8_t t_idx = (0 == packed_type) ? 1 : (packed_type + 1);
        if (t_idx > 4) t_idx = 0;
        
        const uint8_t *bits = (const uint8_t*)pgm_read_word(&PROJ_BMPS[t_idx]);
        const uint8_t *mask = (const uint8_t*)pgm_read_word(&PROJ_MSKS[t_idx]);
        uint8_t src_wh = pgm_read_byte(&PROJ_TEX_SIZES[t_idx]);
        
        int16_t size_scaled = (int16_t)((inv_y * (float)src_wh) * 0.35f);
        if (10 > size_scaled) size_scaled = 10;
        if (size_scaled > 24) size_scaled = 24;
        
        int16_t half_size = size_scaled >> 1;
        drawSprite(sx - half_size, sy - half_size, bits, mask, src_wh, src_wh, size_scaled, size_scaled, 0, 0);
    }
}

//=============================================================================
// ARDUINO MAIN LOOP
//=============================================================================

void loop() {
    static uint8_t last_state = 0xFF;
    static bool last_secret = false;
    
    // Update music on state change
    if(last_state != game.state || last_secret != game.is_secret_level) {
        last_state = game.state;
        last_secret = game.is_secret_level;
        update_music();
    }
    
    switch(game.state) {
        case STATE_INTRO:
        case STATE_LOSE:
        case STATE_WIN:
            loopIntroFinal();
            break;
            
        case STATE_LEVEL_1:
        case STATE_LEVEL_2:
        case STATE_LEVEL_3:
        case STATE_SECRET:
            if(!game.level_initialized) {
                initializeLevel(stateToLevel(game.state));
                game.level_initialized = true;
                loopLevelStart();
            }
            loopGamePlay();
            break;
    }
}