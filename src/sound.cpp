// sound.cpp - Sound and music system for Arduino
// This file implements PWM-based sound generation and music playback
// using Timer1 for audio output and Timer0 for timing.

#define IN_SOUND_CPP
#include "sound.h"
#include "game_state.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//=============================================================================
// MUSIC NOTE FREQUENCY TABLE
// Index 0 = silence, 1+ = note frequencies in Hz (timer counter values)
//=============================================================================

const uint16_t scale[] PROGMEM = {
    0,      // Silence
    7644,   // C
    6811,   // C#
    6068,   // D
    5727,   // D#
    5104,   // E
    4544,   // F
    4048,   // F#
    3822,   // G
    3406,   // G#
    3034,   // A
    2864,   // A#
    2552,   // B
    2272,   // C (high)
    2024,   // C# (high)
    1911,   // D (high)
    1703,   // D# (high)
    1517,   // E (high)
    1432,   // F (high)
    1276,   // F# (high)
    1136,   // G (high)
    1012,   // G# (high)
    956,    // A (high)
    851,    // A# (high)
    750,    // B (high)
    640     // C (high)
};

//=============================================================================
// SOUND EFFECT DATA
// Each sound is a sequence of volume/duration values terminated by 0xFF
//=============================================================================

const uint8_t SND_DATA[] PROGMEM = {
    // Melee shoot sound
    0xF0, 0xFD, 0xFE, 0xFF,
    // Blaster shoot sound
    0x30, 0x36, 0x40, 0x4C, 0xFF,
    // Laser shoot sound
    0x18, 0x19, 0x18, 0x19, 0x20, 0xFF,
    // Plasma shoot sound
    0x70, 0x60, 0x55, 0x60, 0x70, 0xFF,
    // Door open sound
    0xA0, 0x90, 0x80, 0x70, 0x60, 0xFF,
    // Item pickup sound
    0x36, 0x2B, 0x24, 0x1B, 0xFF,
    // Player hurt sound
    0x78, 0x75, 0x72, 0xFF,
    // Enemy hurt sound
    0x50, 0x45, 0x40, 0xFF,
    // Enemy death sound
    0xFE, 0xA0, 0xFD, 0x60, 0xFC, 0x30, 0xFF,
    // Wall hit sound
    0x65, 0x60, 0x65, 0xFF
};

//=============================================================================
// MUSIC MELODY DATA
// Format: [note, duration] pairs, -1 = rest, 1/2/4/8 = note duration
//=============================================================================

const int8_t melody_data[] PROGMEM = {
    // Main theme - Level 1 music
    13,4, 15,4, 17,2, 17,4, 15,4, 13,2, 12,4, 13,4, 15,2,
    13,4, 12,4, 10,1, 13,4, 15,4, 17,2, 18,4, 17,4, 15,2,
    17,4, 18,4, 20,2, 17,4, 15,4, 13,1, -1,4,
    15,4, 17,4, 15,4, 13,4, 14,4, 15,4, 13,2,
    12,4, 13,4, 14,4, 12,4, 13,4, 14,4, 15,2,
    15,4, 17,4, 18,4, 17,2, 15,4, 17,4, 15,4,
    14,4, 15,4, 13,4, 12,4, 10,1, -1,4,
    17,4, 16,4, 15,2, 14,4, 15,4, 14,2,
    13,4, 14,4, 13,2, 12,4, 13,4, 12,1,
    13,4, 14,4, 15,2, 16,4, 15,4, 14,2,
    13,4, 12,4, 13,4, 14,4, 15,4, 13,4, 12,1, -1,4,
    
    // Bridge section
    14,8, 15,8, 16,8, 17,4, 16,8, 15,8, 14,4,
    15,8, 14,8, 13,8, 12,4, 13,8, 14,8, 15,4,
    16,8, 17,8, 18,8, 17,4, 16,8, 15,8, 14,4,
    15,8, 16,8, 17,8, 14,4, 12,4, 10,1, -1,4,
    
    // Climax section
    17,8, 18,8, 19,8, 17,4, 18,8, 19,8, 20,4,
    19,8, 18,8, 17,2, 16,8, 17,8, 18,8, 16,4,
    17,8, 18,8, 19,4, 17,8, 18,8, 17,1, -1,4,
    
    // Resolution
    17,2, 15,4, 13,4, 12,1,
    15,2, 13,4, 12,4, 10,1,
    13,2, 12,4, 10,4, 8,1,
    10,4, 8,4, 6,4, 4,4, 6,2, 4,2, 2,1, -1,4,
    
    // Outro
    15,4, 17,4, 18,4, 19,2,
    19,8, 20,8, 22,2,
    20,4, 19,4, 18,4, 19,2,
    15,4, 17,4, 18,4, 19,4,
    20,4, 22,4, 19,4, 20,4,
    22,4, 24,2, 19,1, -1,4
};

//=============================================================================
// SCENE MUSIC OFFSETS
// Offsets into melody_data for each game state
//=============================================================================

static const uint16_t SCENE_OFFSETS[] PROGMEM = {
    0,      // Intro/Title
    52,     // Level 1
    108,    // Level 2
    160,    // Level 3
    216,    // Secret level
    260,    // Lose/Game Over
    308     // Win/Victory
};

//=============================================================================
// SOUND PLAYBACK STATE
//=============================================================================

static uint8_t snd_offset = 0xFF;   // Current sound effect offset
static uint8_t snd_idx;             // Current position in sound data
static uint8_t music_step;          // Current position in melody
static uint8_t music_timer;         // Timer for note duration

//=============================================================================
// UPDATE MUSIC (restart from beginning)
//=============================================================================

void update_music(void) {
    music_step = 0;
    music_timer = 0;
}

//=============================================================================
// PLAY SOUND EFFECT
//=============================================================================

void playSound(uint8_t offset) {
    snd_offset = offset;
    snd_idx = 0;
}

//=============================================================================
// SOUND SYSTEM INITIALIZATION
//=============================================================================

void sound_init(void) {
    DDRB |= 0x02;                   // Set PB1 (pin 9) as output for PWM
    TCCR1A = 0;                     // Clear timer1 control registers
    TCCR1B = (1 << WGM12) | (1 << CS11);  // CTC mode, prescaler = 8
    TIMSK0 |= (1 << OCIE0A);        // Enable Timer0 compare interrupt
}

//=============================================================================
// TIMER0 COMPARE INTERRUPT - Sound and music playback
//=============================================================================

ISR(TIMER0_COMPA_vect) {
    static uint8_t div = 0;
    
    // Divide timer frequency to reduce update rate
    if (++div & 0x07) return;
    
    // Play sound effect if active
    if (snd_offset != 0xFF) {
        uint8_t b = pgm_read_byte(&SND_DATA[snd_offset + snd_idx++]);
        if (b != 0xFF) {
            // Set PWM duty cycle
            TCCR1A |= (1 << COM1A0);
            OCR1A = ((uint16_t)b << 4) + ((uint16_t)b << 3);  // b * 24
            return;
        }
        snd_offset = 0xFF;
        music_timer = 0;
        return;
    }
    
    // Play background music if enabled
    if (game.music_on && game.state != 5) {
        if (music_timer > 0) {
            if (--music_timer == 1) {
                TCCR1A &= ~(1 << COM1A0);  // Stop note
            }
        } else {
            // Get current state's music data
            uint8_t state = game.state;
            if (state > 5) state--;
            if (state > 6) state = 0;
            uint16_t base = pgm_read_word(&SCENE_OFFSETS[state]);
            
            // Read next note
            int8_t note = (int8_t)pgm_read_byte(&melody_data[base + music_step]);
            if (note == -1) {
                // Restart melody
                music_step = 0;
                note = (int8_t)pgm_read_byte(&melody_data[base]);
            }
            uint8_t dur = pgm_read_byte(&melody_data[base + music_step + 1]);
            music_step += 2;
            
            // Calculate duration
            const uint8_t base_spd = 18;
            if (dur == 1) {
                music_timer = base_spd << 3;
            } else if (dur == 2) {
                music_timer = base_spd << 2;
            } else if (dur == 4) {
                music_timer = base_spd << 1;
            } else {
                music_timer = base_spd;
            }
            
            // Play note
            if (note > 0) {
                uint16_t f = pgm_read_word(&scale[note]);
                OCR1A = f + (f >> 1);  // 1.5x frequency
                TCCR1A |= (1 << COM1A0);
            } else {
                TCCR1A &= ~(1 << COM1A0);
            }
        }
    } else {
        // Music off or game over state
        TCCR1A &= ~(1 << COM1A0);
    }
}