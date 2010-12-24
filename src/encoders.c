/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

http://www.neufeld.newton.ks.us/electronics/?p=241
*/

/*
#include <LiquidCrystal.h>
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/io.h>
*/
#include "encoders.h"

/*
 * State table (For each encoder):
 *
 * '*' shouldn't happen, as only encoder value changes are processed
 *
 * State Name | New encoder Val | Next State
 * -----------|-----------------|-----------
 * REST       |       0         | WF3 (wait for 3)
 *            |       1         | CW1
 *            |       2         | CCW1
 *            |       3         | REST
 * -----------|-----------------|-----------
 * WF3        |       0         | WF3
 *            |       1         | WF3
 *            |       2         | WF3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * CW1        |       0         | CW2
 *            |       1         | CW1 *
 *            |       2         | WF3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * CW2        |       0         | CW2 *
 *            |       1         | WF3
 *            |       2         | CW3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * CW3        |       0         | WF3
 *            |       1         | WF3
 *            |       2         | CW3 *
 *            |       3         | REST (CW++)
 * -----------|-----------------|-----------
 * CCW1       |       0         | CCW2
 *            |       1         | WF3
 *            |       2         | CCW1 *
 *            |       3         | REST
 * -----------|-----------------|-----------
 * CCW2       |       0         | CCW2 *
 *            |       1         | CCW3
 *            |       2         | WF3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * CCW3       |       0         | WF3
 *            |       1         | CCW3 *
 *            |       2         | WF3
 *            |       3         | REST (CCW++)
 * -----------|-----------------|-----------
 */

// Globals for rotary encoder state machine

static unsigned char enc_state[NUM_ENCODERS];

const static unsigned char next_state[RS_NUM_INPUT_VALUES][RS_MAX_STATES] = {
    RS_WF3,  RS_CW1,  RS_CCW1, RS_REST,                // RS_REST
    RS_WF3,  RS_WF3,  RS_WF3,  RS_REST,                // RS_WF3
    RS_CW2,  RS_CW1,  RS_WF3,  RS_REST,                // RS_CW1
    RS_CW2,  RS_WF3,  RS_CW3,  RS_REST,                // RS_CW2
    RS_WF3,  RS_WF3,  RS_CW3,  RS_REST | RS_INC_CW,    // RS_CW3
    RS_CCW2, RS_WF3,  RS_CCW1, RS_REST,                // RS_CCW1
    RS_CCW2, RS_CCW3, RS_WF3,  RS_REST,                // RS_CCW2
    RS_WF3,  RS_CCW3, RS_WF3,  RS_REST | RS_INC_CCW    // RS_CCW3
};

int init_encoders(void) {
    unsigned char i;
    for (i=0; i<NUM_ENCODERS; i++)
	enc_state[i] = RS_REST;
}

int do_enc_state(unsigned char newval, unsigned char encoder) {
    unsigned char oldstate, newstate;
    int count = 0;

    if (encoder >= NUM_ENCODERS)
	// error
	return 0;
  
    oldstate = enc_state[encoder];

    newstate = next_state[oldstate][newval];
    // Check the high bits to see whether we completed a click
    if (newstate & RS_INC_CCW) {
	count--;
    } else if (newstate & RS_INC_CW) {
	count++;
    }

    // get the clean state
    newstate &= RS_MASK;

    enc_state[encoder] = newstate;

    return count;
}
