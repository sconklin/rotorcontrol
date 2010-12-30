/*
  Encoder handler code
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef _ENCODER_H_
#define _ENCODER_H_

/*
 * State table (For each encoder):
 *
 * 3 is rest position
 * 01 00 10 11 left 1 0 2 3
 * 10 00 01 11 right  2 0 1 3
 *
 * '*' shouldn't happen, as only encoder value changes are processed
 *
 * State Name | New encoder Val | Next State
 * -----------|-----------------|-----------
 * 0 REST     |       0         | WF3 (wait for 3)
 *            |       1         | CCW1
 *            |       2         | CW1
 *            |       3         | REST
 * -----------|-----------------|-----------
 * 1 WF3      |       0         | WF3
 *            |       1         | WF3
 *            |       2         | WF3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * 2 CCW1     |       0         | CCW2
 *            |       1         | CCW1 *
 *            |       2         | WF3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * 3 CCW2     |       0         | CCW2 *
 *            |       1         | WF3
 *            |       2         | CCW3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * 4 CCW3     |       0         | WF3
 *            |       1         | WF3
 *            |       2         | CCW3 *
 *            |       3         | REST (CCW++)
 * -----------|-----------------|-----------
 * 5 CW1      |       0         | CW2
 *            |       1         | WF3
 *            |       2         | CW1 *
 *            |       3         | REST
 * -----------|-----------------|-----------
 * 6 CW2      |       0         | CW2 *
 *            |       1         | CW3
 *            |       2         | WF3
 *            |       3         | REST
 * -----------|-----------------|-----------
 * 7 CW3      |       0         | WF3
 *            |       1         | CW3 *
 *            |       2         | WF3
 *            |       3         | REST (CW++)
 * -----------|-----------------|-----------
 */

// States for Rotary encoders
#define RS_REST 0
#define RS_WF3  1
#define RS_CCW1 2
#define RS_CCW2 3
#define RS_CCW3 4
#define RS_CW1  5
#define RS_CW2  6
#define RS_CW3  7
#define RS_MAX_STATES 8

#define RS_NUM_INPUT_VALUES 4
#define RS_INC_CW  0x80
#define RS_INC_CCW 0x40
#define RS_MASK    0x0F

#define NUM_ENCODERS 2

//
// Encoder functions
//
int init_encoders(void);
int do_enc_state(unsigned char, unsigned char);

#endif /* _ENCODER_H_ */
