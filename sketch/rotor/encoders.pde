// -*- Mode:c -*-
/*
  Encoder handler code
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#include "encoders.h"

// Globals for rotary encoder state machine

static unsigned char enc_state[NUM_ENCODERS];

const static unsigned char next_state[RS_MAX_STATES][RS_NUM_INPUT_VALUES] = {
//const static unsigned char next_state[RS_NUM_INPUT_VALUES][RS_MAX_STATES] = {
    RS_WF3,  RS_CCW1, RS_CW1,  RS_REST,                // 0 RS_REST
    RS_WF3,  RS_WF3,  RS_WF3,  RS_REST,                // 1 RS_WF3
    RS_CCW2, RS_CCW1, RS_WF3,  RS_REST,                // 2 RS_CCW1
    RS_CCW2, RS_WF3,  RS_CCW3, RS_REST,                // 3 RS_CCW2
    RS_WF3,  RS_WF3,  RS_CCW3, RS_REST | RS_INC_CCW,   // 4 RS_CCW3
    RS_CW2,  RS_WF3,  RS_CW1,  RS_REST,                // 5 RS_CW1
    RS_CW2,  RS_CW3,  RS_WF3,  RS_REST,                // 6 RS_CW2
    RS_WF3,  RS_CW3,  RS_WF3,  RS_REST | RS_INC_CW     // 7 RS_CW3
};

// globals for input
static unsigned char lastinput;

int init_encoders(void) {
    unsigned char i;
    for (i=0; i<NUM_ENCODERS; i++)
	enc_state[i] = RS_REST;
    lastinput = expansion_read();
}

int do_enc_state(unsigned char newval, unsigned char encoder) {
    unsigned char oldstate, newstate;
    int count = 0;

    if (encoder >= NUM_ENCODERS)
	rotor_error(ERR_INVALID_ENCODER); // Does not return
  
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

unsigned char get_input_events(void) {
    unsigned char in_events = 0;
    unsigned char thisinput;
    int encoder_moved;
    // Check to see if we got any input events
    if (digitalRead(2) == 0) {
	unsigned char changes;
	thisinput = expansion_read();
	changes = thisinput ^ lastinput;
	if ((changes & RBIN) && (thisinput & RBIN)) {
	    in_events |= R_PRESS; // right button changed
	}
	if ((changes & LBIN) && (thisinput & LBIN)) {
	    in_events |= L_PRESS; // left button changed
	}
	if (changes & (RENA | RENB)) {
	    // right encoder changed
	    unsigned char encval = (thisinput & (RENA | RENB)) >> RENSHIFT;
	    encoder_moved = do_enc_state(encval, R_ENC);
	    if (encoder_moved == 1)
		in_events |= R_CW;
	    else if (encoder_moved == -1)
		in_events |= R_CCW;
	}
	if (changes & (LENA | LENB)) {
	    // left encoder changed
	    unsigned char encval = (thisinput & (LENA | LENB)) >> LENSHIFT;
	    encoder_moved = do_enc_state(encval, L_ENC);
	    if (encoder_moved == 1)
		in_events |= L_CW;
	    else if (encoder_moved == -1)
		in_events |= L_CCW;
	}
	lastinput = thisinput;
    }
    return in_events;
}
