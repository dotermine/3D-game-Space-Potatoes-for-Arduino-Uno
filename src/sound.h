// sound.h - Sound and music system header
// This file declares the sound effect and music playback functions
// for the game's audio system using PWM output.

#ifndef _sound_h
#define _sound_h

#include <avr/io.h>
#include <avr/pgmspace.h>

//=============================================================================
// SOUND EFFECT OFFSETS
// These offsets index into SND_DATA table
//=============================================================================

#define snd_shoot_melee     0   // Fist/melee attack sound
#define snd_shoot_blaster   4   // Blaster fire sound
#define snd_shoot_laser     9   // Laser fire sound
#define snd_shoot_plasma    15  // Plasma/BFG fire sound
#define snd_door_open       21  // Door opening sound
#define snd_item_pickup     27  // Item pickup sound
#define snd_player_hurt     32  // Player taking damage sound
#define snd_enemy_hurt      36  // Enemy taking damage sound
#define snd_enemy_death     40  // Enemy death sound
#define snd_wall_hit        47  // Projectile hitting wall sound

//=============================================================================
// GLOBAL EXTERNAL DECLARATIONS
//=============================================================================

extern const uint8_t SND_DATA[] PROGMEM;
extern const uint16_t scale[] PROGMEM;
extern const int8_t melody_data[] PROGMEM;

//=============================================================================
// FUNCTIONS
//=============================================================================

// Initialize the sound system (PWM and timers)
void sound_init(void);

// Play a sound effect by offset
void playSound(uint8_t offset);

// Restart background music from beginning
void update_music(void);

#endif