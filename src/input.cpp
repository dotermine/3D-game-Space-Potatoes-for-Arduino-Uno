// input.cpp - Input handling for game controls
// This file implements button state reading for the game's input system
// using direct port manipulation for fast and efficient polling.

#include "input.h"

//=============================================================================
// INPUT SETUP
// Configure pins as inputs with internal pull-ups enabled
//=============================================================================

void input_setup(void) {
    // Port D pins: Left(PD2), Up(PD3), Down(PD4), Right(PD5), Fire(PD7)
    // Set as inputs with pull-ups enabled
    PORTD |= K_LEFT_BIT | K_UP_BIT | K_DOWN_BIT | K_RIGHT_BIT | K_FIRE_BIT;
    DDRD &= ~(K_LEFT_BIT | K_UP_BIT | K_DOWN_BIT | K_RIGHT_BIT | K_FIRE_BIT);
    
    // Port B pin: Use(PB0)
    // Set as input with pull-up enabled
    PORTB |= K_USE_BIT;
    DDRB &= ~K_USE_BIT;
}

//=============================================================================
// BUTTON STATE FUNCTIONS
// All functions return true if button is pressed (active low)
//=============================================================================

bool input_left(void) {
    return IS_PRESSED(PIND, K_LEFT_BIT);
}

bool input_right(void) {
    return IS_PRESSED(PIND, K_RIGHT_BIT);
}

bool input_up(void) {
    return IS_PRESSED(PIND, K_UP_BIT);
}

bool input_down(void) {
    return IS_PRESSED(PIND, K_DOWN_BIT);
}

bool input_fire(void) {
    return IS_PRESSED(PIND, K_FIRE_BIT);
}

bool input_use(void) {
    return IS_PRESSED(PINB, K_USE_BIT);
}