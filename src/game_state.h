// game_state.h - Game state management header
// This file declares the core game state structures and functions
// used for tracking game progression, difficulty, and global flags.

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include "constants.h"

//=============================================================================
// GAME STATE STRUCTURE
//=============================================================================

struct GameState {
    uint8_t state;                  // Current game state (STATE_*)
    uint8_t difficulty;             // Difficulty level (0=easy, 1=normal, 2=hard)
    uint8_t music_on : 1;           // 1 if music is enabled
    uint8_t is_secret_level : 1;    // 1 if currently in secret level
    uint8_t isVictory : 1;          // 1 if victory achieved
    uint8_t level_initialized : 1;  // 1 if current level is initialized
    uint8_t reserved : 4;           // Reserved for future use
};

//=============================================================================
// GLOBAL EXTERNAL DECLARATIONS
//=============================================================================

extern struct GameState game;
extern uint8_t easter_egg_count;
extern uint8_t messages_read_flags;

//=============================================================================
// MACROS
//=============================================================================

// Check if the given state is a gameplay state
#define IS_GAMEPLAY(state) ((state) >= STATE_LEVEL_1 && (state) <= STATE_SECRET)

//=============================================================================
// FUNCTIONS
//=============================================================================

// Initialize the game state to default values
void initGameState(void);

// Convert a level index and secret flag to a game state
uint8_t levelToState(uint8_t level, uint8_t is_secret);

#endif