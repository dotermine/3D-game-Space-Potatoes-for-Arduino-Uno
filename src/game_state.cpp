// game_state.cpp - Game state management
// This file implements the core game state tracking, including
// level progression, difficulty settings, and global game flags.

#include "game_state.h"

//=============================================================================
// GLOBAL GAME STATE VARIABLES
//=============================================================================

struct GameState game;              // Main game state structure
uint8_t easter_egg_count = 0;       // Number of easter egg books collected

//=============================================================================
// INITIALIZE GAME STATE
//=============================================================================

void initGameState(void) {
    game.state = STATE_INTRO;       // Start at title screen
    game.difficulty = 1;            // Normal difficulty (0=easy, 1=normal, 2=hard)
    game.music_on = 0;              // Music disabled by default
    game.is_secret_level = 0;       // Not in secret level
    game.isVictory = 0;             // Not victory state
    game.level_initialized = 0;     // Level not yet initialized
    game.reserved = 0;              // Reserved for future use
    
    easter_egg_count = 0;
}

//=============================================================================
// LEVEL TO STATE CONVERSION
//=============================================================================

uint8_t levelToState(uint8_t level, uint8_t is_secret) {
    if (is_secret) {
        return STATE_SECRET;
    }
    
    switch(level) {
        case 0: return STATE_LEVEL_1;
        case 1: return STATE_LEVEL_2;
        case 2: return STATE_LEVEL_3;
        default: return STATE_LEVEL_1;
    }
}