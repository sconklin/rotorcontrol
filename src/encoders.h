/*
  Encoder handling
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef ENCODERS_H_
#define ENCODERS_H_

// States for Rotary encoders
#define RS_REST 0
#define RS_WF3 1
#define RS_CW1 2
#define RS_CW2 3
#define RS_CW3 4
#define RS_CCW1 5
#define RS_CCW2 6
#define RS_CCW3 7
#define RS_MAX_STATES 8

#define RS_NUM_INPUT_VALUES 4
#define RS_INC_CCW 0x80
#define RS_INC_CW 0x40
#define RS_MASK 0x0F

// How many encoders do we have in the system
#define NUM_ENCODERS 2

int init_encoders(void);

int do_enc_state(byte,byte);

#endif /* ENCODERS_H_ */
