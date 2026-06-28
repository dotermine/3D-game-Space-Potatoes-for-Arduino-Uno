// input.h - Input handling header
// This file defines the pin mappings and functions for reading
// game controller inputs using direct port manipulation.

#ifndef _input_h
#define _input_h

#include <avr/io.h>

//=============================================================================
// PIN DEFINITIONS
//=============================================================================

#define K_LEFT_PIN      2   // PD2 - Left button
#define K_RIGHT_PIN     5   // PD5 - Right button
#define K_UP_PIN        3   // PD3 - Up button
#define K_DOWN_PIN      4   // PD4 - Down button
#define K_FIRE_PIN      7   // PD7 - Fire button
#define K_USE_PIN       8   // PB0 - Use/Interact button

//=============================================================================
// BIT MASKS
//=============================================================================

#define K_LEFT_BIT      (1<<2)   // PD2
#define K_RIGHT_BIT     (1<<5)   // PD5
#define K_UP_BIT        (1<<3)   // PD3
#define K_DOWN_BIT      (1<<4)   // PD4
#define K_FIRE_BIT      (1<<7)   // PD7
#define K_USE_BIT       (1<<0)   // PB0

//=============================================================================
// MACROS
//=============================================================================

// Button is pressed when pin reads LOW (pull-up enabled)
#define IS_PRESSED(port, bit) (!(port & bit))

//=============================================================================
// FUNCTIONS
//=============================================================================

// Initialize all input pins with pull-ups enabled
void input_setup(void);

// Read individual button states
bool input_left(void);     // Move left (turn)
bool input_right(void);    // Move right (turn)
bool input_up(void);       // Move forward
bool input_down(void);     // Move backward
bool input_fire(void);     // Attack/Shoot
bool input_use(void);      // Use/Interact (doors, weapon change)

#endif