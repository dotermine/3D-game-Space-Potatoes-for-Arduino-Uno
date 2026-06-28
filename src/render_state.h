// render_state.h - добавить функции-помощники

#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#include "constants.h"
#include "types.h"



struct RenderState {
    // Raycasting
    float rayDirX;
    float rayDirY;
    float delta_x;
    float delta_y;
    float perpWallDist;
    float wall_x;
    float inv_dist;
    
    // Common
    float inv_det;
    int16_t v_offset;
    
    // Texture mapping
    float doorOffset;
    uint8_t wall_type;
    bool flipped;
    
    // Sprite rendering
    uint16_t distance;
    bool flip_x;
    
    int8_t shake_x;      // тряска камеры по X
    int8_t shake_y;      // тряска камеры по Y
    uint8_t shake_timer; // таймер тряски

};

extern RenderState g_render;

// Inline helper для настройки спрайта
inline void setupSpriteRender(uint16_t distance, bool flip_x) {
    g_render.distance = distance;
    g_render.flip_x = flip_x;
}

#endif