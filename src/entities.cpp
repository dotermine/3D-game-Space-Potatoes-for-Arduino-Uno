// entities.cpp - Entity management and AI logic
// This file implements enemy creation, AI behavior, projectiles,
// throwable objects, effects, and all entity-related game logic
// for the raycasting engine.

#include <stdint.h>
#include <math.h>
#include "entities.h"
#include "types.h"
#include "constants.h"
#include "game_state.h"
#include "display.h"
#include "sound.h"
#include "input.h"

//=============================================================================
// EXTERNAL GLOBAL VARIABLES
//=============================================================================

extern Player player;
extern Entity entity[MAX_ENTITIES];
extern uint8_t num_entities;
extern uint8_t flash_screen;
extern GameState game;
extern const uint8_t* level_data[];
extern uint8_t stateToLevel(uint8_t state);
extern uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y);
extern Door* getDoorAt(uint8_t x, uint8_t y);
extern uint8_t isVisible(float x1, float y1, float x2, float y2);
extern void playSound(uint8_t offset);
extern Projectile projectiles[MAX_PROJECTILES];
extern Item items[MAX_ITEMS];
extern uint8_t num_items;
extern uint8_t easter_egg_count;
extern void showNotification(const uint8_t* text, const uint8_t* icon, uint8_t w, uint8_t h);
extern void spawnProjectile(float start_x, float start_y, float dir_x, float dir_y, uint8_t proj_type);
extern RenderState g_render;
extern uint8_t g_tick_counter;

//=============================================================================
// THROWABLE OBJECTS
//=============================================================================

Throwable throwables[MAX_THROWABLES];
uint8_t num_throwables = 0;

extern Entity* g_current_render_entity;

//=============================================================================
// ENTITY HEIGHT OFFSETS
// Y-position adjustment for different enemy types
//=============================================================================

const uint8_t ENTITY_HEIGHT_OFFSETS[] PROGMEM = {
    10, 10, 3, 10, 10, 17, 3, 3, 3, 3
};

const uint8_t ENEMY_PROPERTIES_TABLE[] PROGMEM = {
    0,                             // 0x00 - reserved
    0,                             // 0x01 - reserved
    0,                             // 0x02 - reserved
    MASK_SHOOTER,                  // 0x03 - E_NUB: hangs from the ceiling, shoots
    MASK_SHOOTER,                  // 0x04 - E_EASY: stands on the floor, shoots
    MASK_MELEE | MASK_FLYING,      // 0x05 - E_MEDIUM: flies, melee
    MASK_MELEE | MASK_FLYING,      // 0x06 - E_HARD: flies, melee
    MASK_MELEE | MASK_SHOOTER,     // 0x07 - E_BOSS_1: stands, shoots + melee
    MASK_MELEE | MASK_SHOOTER,     // 0x08 - E_BOSS_2: stands, shoots + melee
    MASK_MELEE | MASK_SHOOTER,     // 0x09 - E_BOSS_3: stands, shoots + melee
    MASK_MELEE,                    // 0x0A - E_BIGPOTATO: stands, no attack (melee as defense only)
};

//=============================================================================
// CREATE ENTITY
//=============================================================================

// Create a new enemy entity at the specified grid position
// Parameters:
//   type - enemy type (E_NUB, E_EASY, E_MEDIUM, etc.)
//   x, y - grid coordinates of the cell where enemy spawns
// Returns: initialized Entity structure ready for use in the game

Entity create_entity(uint8_t type, uint8_t x, uint8_t y)
{
    Entity e;

    // Place enemy exactly in the center of the grid cell
    // This ensures proper alignment with the raycasting grid
    e.pos.x = x + 0.5f;
    e.pos.y = y + 0.5f;
    
    // Set enemy type and initial state
    e.type = type;
    e.state = S_STAND;      // Start in idle state
    e.timer = 0;            // Reset animation timer
    e.shoot_timer = 0;      // Reset shooting cooldown
    
    // Calculate health based on enemy type and difficulty
    uint8_t base = type - 2;
    
    uint8_t diff = game.difficulty;
    uint8_t mult = (1 + diff) << 2;  
    
    e.health = base * mult;
    
    return e;
}

//=============================================================================
// CELL PASSABILITY CHECK
//=============================================================================

static uint8_t isCellPassable(uint8_t block, uint8_t bx, uint8_t by) {
    if (block >= E_WALL_TYPE_1) {
        if (block == E_DOOR) {
            Door* d = getDoorAt(bx, by);
            if (d != 0) {
                if (d->state == S_OPEN) {
                    return 1;
                }
            }
        }
        return 0;
    }
    return 1;
}

//=============================================================================
// ENEMY AI UPDATE
//=============================================================================

void updateEnemyAI(Entity* e) {
    extern int16_t i_speed;
    uint8_t est = e->state;
    
    if (S_DEAD == est) return;
    
    // Handle state timers (dying, damaged, attacking)
    if (S_DYING == est || S_DAMAGED == est || S_ATTACK == est) {
        uint8_t t = e->timer & 0x0F;
        if (t > 0) {
            e->timer = (e->timer & 0xF0) | (t - 1);
        }
        if (0 == t || 1 == t) {
            if (S_DYING == est) {
                e->state = S_DEAD;
            } else {
                e->state = S_MOVE;
                if (S_ATTACK == est) {
                    e->shoot_timer = 20;
                }
            }
            e->timer = 0;
        }
        return;
    }
    
    if (e->shoot_timer > 0) {
        e->shoot_timer--;
    }
    
    uint8_t type = e->type;
    uint8_t props = pgm_read_byte(&ENEMY_PROPERTIES_TABLE[type]);
    float dx = player.pos.x - e->pos.x;
    float dy = player.pos.y - e->pos.y;
    float distSq = dx * dx + dy * dy;
    
    e->state = S_MOVE;
    
    // Big potato is stationary
    if (type == E_BIGPOTATO) {
        e->state = S_STAND;
        return;
    }
    
    // Line of sight check
    if (0 == isVisible(e->pos.x, e->pos.y, player.pos.x, player.pos.y)) {
        e->state = S_STAND;
        return;
    }
    
    // Melee attack
    if ((props & MASK_MELEE) && ENEMY_MELEE_DIST_SQ >= distSq && 0 == e->shoot_timer) {
        e->state = S_ATTACK;
        e->timer = ENEMY_ATTACK_DURATION;
        i_speed = ENEMY_MELEE_KNOCKBACK;
        
        uint8_t power = type - 2;
        uint8_t mult = game.difficulty + 1;
        uint8_t dmg = power * mult;
        player.health = (player.health > dmg) ? (player.health - dmg) : 0;
        
        flash_screen = 2;
        playSound(snd_player_hurt);
        g_render.shake_timer = SHAKE_DURATION_DAMAGE;
        return;
    }
    
    // Ranged attack
    float inv_dist = 0.0f;
    uint8_t has_inv = 0;
    
    if ((props & MASK_SHOOTER) && 0 == e->shoot_timer) {
        float shootRangeSq = 400.0f;
        if (type >= E_BOSS_1) {
            shootRangeSq = 150.0f;
        } else if (type >= E_MEDIUM) {
            shootRangeSq = 200.0f;
        }
        
        if (shootRangeSq > distSq) {
            e->shoot_timer = (type >= E_BOSS_1) ? ENEMY_SHOOT_DELAY_BOSS : ENEMY_SHOOT_DELAY_NORMAL;
            
            if (distSq > 0.0001f) {
                inv_dist = 1.0f / sqrtf(distSq);
                has_inv = 1;
                float norm_dx = dx * inv_dist;
                float norm_dy = dy * inv_dist;
                float spawn_x = e->pos.x + norm_dx * 0.25f;
                float spawn_y = e->pos.y + norm_dy * 0.25f;
                spawnProjectile(spawn_x, spawn_y, norm_dx, norm_dy, type + 10);
                playSound(snd_shoot_blaster);
            }
        }
    }
    
    // Movement (flying enemies)
    if ((props & MASK_FLYING) && distSq > 0.5f) {
        if (0 == has_inv) {
            inv_dist = 1.0f / sqrtf(distSq);
        }
        
        float speed = (E_MEDIUM == type) ? 0.06f : 0.04f;
        float nx = e->pos.x + dx * inv_dist * speed;
        float ny = e->pos.y + dy * inv_dist * speed;
        
        uint8_t lvl = stateToLevel(game.state);
        const uint8_t *lvl_ptr = level_data[lvl];
        uint8_t bnx = (uint8_t)nx;
        uint8_t bny = (uint8_t)ny;
        uint8_t bex = (uint8_t)e->pos.x;
        uint8_t bey = (uint8_t)e->pos.y;
        
        // Try full movement
        uint8_t block = getBlockAt(lvl_ptr, bnx, bny);
        if (isCellPassable(block, bnx, bny) > 0) {
            e->pos.x = nx;
            e->pos.y = ny;
        } else {
            // Try X movement only
            block = getBlockAt(lvl_ptr, bnx, bey);
            if (isCellPassable(block, bnx, bey) > 0) {
                e->pos.x = nx;
            } else {
                // Try Y movement only
                block = getBlockAt(lvl_ptr, bex, bny);
                if (isCellPassable(block, bex, bny) > 0) {
                    e->pos.y = ny;
                }
            }
        }
    }
}

//=============================================================================
// THROWABLE OBJECT MANAGEMENT
//=============================================================================

static void deactivateThrowable(Throwable *t) {
    t->active = 0;
    if (num_throwables > 0) {
        num_throwables--;
    }
}

void throwHeldItem(void) {
    if (0 == player.ammo[WEAPON_FIST]) {
        return;
    }
    
    float p_x = player.pos.x;
    float p_y = player.pos.y;
    float d_x = player.dir.x;
    float d_y = player.dir.y;
    
    for (uint8_t i = 0; MAX_THROWABLES > i; i++) {
        if (1 > throwables[i].active) {
            throwables[i].x = p_x + d_x * 0.5f;
            throwables[i].y = p_y + d_y * 0.5f;
            throwables[i].vx = d_x * 0.45f;
            throwables[i].vy = d_y * 0.45f;
            throwables[i].type = E_METAL_BOX;
            throwables[i].active = 1;
            throwables[i].timer = 60;
            num_throwables++;
            player.ammo[WEAPON_FIST] = 0;
            playSound(snd_shoot_melee);
            break;
        }
    }
}

void updateThrowables(void) {
    uint8_t level_idx = stateToLevel(game.state);
    const uint8_t *current_level_ptr = level_data[level_idx];
    
    for (uint8_t i = 0; MAX_THROWABLES > i; i++) {
        Throwable *t = &throwables[i];
        if (1 > t->active) continue;
        
        t->timer--;
        if (0 == t->timer) {
            deactivateThrowable(t);
            continue;
        }
        
        t->x += t->vx;
        t->y += t->vy;
        
        uint8_t bx = (uint8_t)t->x;
        uint8_t by = (uint8_t)t->y;
        
        if (bx >= LEVEL_WIDTH || by >= LEVEL_HEIGHT) {
            deactivateThrowable(t);
            continue;
        }
        
        // Wall collision
        uint8_t block = getBlockAt(current_level_ptr, bx, by);
        uint8_t hit_solid = 0;
        
        if (block >= E_WALL_TYPE_1) {
            hit_solid = 1;
            if (block == E_DOOR) {
                Door *d = getDoorAt(bx, by);
                if (d != 0 && d->state == S_OPEN) {
                    hit_solid = 0;
                }
            }
        } else if (block == E_METAL_BOX) {
            hit_solid = 1;
        }
        
        if (hit_solid != 0) {
            deactivateThrowable(t);
            playSound(snd_wall_hit);
            addEffect(0, t->x, t->y);
            continue;
        }
        
        // Hit enemies
        for (uint8_t j = 0; num_entities > j; j++) {
            Entity *e = &entity[j];
            if (e->state >= S_DYING) continue;
            
            float dx = e->pos.x - t->x;
            float dy = e->pos.y - t->y;
            if (0.36f > (dx * dx + dy * dy)) {
                applyDamage(e, WEAPON_BOX);
                deactivateThrowable(t);
                break;
            }
        }
    }
}

//=============================================================================
// EFFECT SYSTEM
//=============================================================================

Effect effects[MAX_EFFECTS];
uint8_t num_effects = 0;

const uint8_t* const EFF_BMPS[] PROGMEM = {
    eff_splash_bmp,
    eff_blaster_bmp,
    eff_cutter_bmp,
    eff_bfg_bmp,
    eff_splash_bmp,
};

const uint8_t* const EFF_MSKS[] PROGMEM = {
    eff_splash_mask,
    eff_blaster_msk,
    eff_cutter_msk,
    eff_bfg_msk,
    eff_splash_mask,
};

void addEffect(uint8_t type, float x, float y) {
    for (uint8_t i = 0; MAX_EFFECTS > i; i++) {
        if (effects[i].active != 0) continue;
        
        effects[i].x = (int16_t)(x * 8.0f);
        effects[i].y = (int16_t)(y * 8.0f);
        effects[i].type = type;
        effects[i].timer = EFFECT_DURATION;
        effects[i].active = 1;
        num_effects++;
        break;
    }
}

void updateEffects(void) {
    for (uint8_t i = 0; MAX_EFFECTS > i; i++) {
        if (0 == effects[i].active) continue;
        
        effects[i].timer--;
        if (0 == effects[i].timer) {
            effects[i].active = 0;
            num_effects--;
        }
    }
}

void renderEffects(void) {
    float det = player.plane.x * player.dir.y - player.dir.x * player.plane.y;
    if (fabsf(det) < 0.0001f) return;
    float inv_det = 1.0f / det;
    
    for (uint8_t i = 0; MAX_EFFECTS > i; i++) {
        if (0 == effects[i].active) continue;
        
        float wx = (float)effects[i].x * 0.125f;
        float wy = (float)effects[i].y * 0.125f;
        
        float dx = wx - player.pos.x;
        float dy = wy - player.pos.y;
        
        float ty = inv_det * (-player.plane.y * dx + player.plane.x * dy);
        if (0.2f >= ty || ty > 10.0f) continue;
        
        float tx = inv_det * (player.dir.y * dx - player.dir.x * dy);
        if (fabsf(tx) > ty * 1.5f) continue;
        
        float inv_ty = 1.0f / ty;
        int16_t sx = 64 + (int16_t)(64.0f * tx * inv_ty);
        
        uint8_t z_idx = (sx >= 0 && 128 > sx) ? ((uint8_t)sx >> 5) : 0;
        uint8_t z_val = (uint8_t)(ty * 8.0f);
        if (z_val > 1) z_val -= 2;
        if (ZBUFFER_SIZE > z_idx && z_val > zbuffer[z_idx]) continue;
        
        uint8_t eff_type = effects[i].type;
        if (eff_type > 4) eff_type = 0;
        
        const uint8_t *bmp_ptr = (const uint8_t*)pgm_read_word(&EFF_BMPS[eff_type]);
        const uint8_t *msk_ptr = (const uint8_t*)pgm_read_word(&EFF_MSKS[eff_type]);
        uint8_t tex_size = (eff_type == EFFECT_BFG_EXPLOSION) ? 24 : 16;
        
        uint8_t progress = EFFECT_DURATION - effects[i].timer;
        int16_t size = (int16_t)((float)(16 + (progress << 1)) * inv_ty);
        size = size << 1;
        
        if (eff_type == EFFECT_BFG_EXPLOSION) {
            size += (size >> 1);
        }
        if (4 > size) continue;
        if (size > 64) size = 64;
        
        int16_t screen_y = 32 - (size >> 1);
        drawSprite(sx - (size >> 1), screen_y, bmp_ptr, msk_ptr, tex_size, tex_size, size, size, 0, 0);
    }
}