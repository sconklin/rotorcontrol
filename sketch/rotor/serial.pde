// -*- Mode:c -*-
/*
  Serial handler code
 
 Copyright 2011 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#include "serial.h"
#include "misc.h"



// Globals for serial handler code

static unsigned char serial_state;

static char inbuffer[10];

static int heading_memory[10];

static unsigned char state;
static unsigned char heading_digits;

int init_serial(void) {
  state = SRS_IDLE;
  heading_digits = 0;
  Serial.begin(9600);
}


// DCU-1 protocol:
// (Max of 9 characters in a command)
// ; - stops all rotation
// AIn; - reads heading, returns three digits leading zeroes "xxx;"
// STn; - stops all rotation
// APnxxx(.y); - Sets rotation degrees to xxx degrees and optionslly y tenths for next AMn; command
// AMn; send rotor to last APn setting

void process_serial_command(void) {
    int memno;
    // We have a complete command in the inbuffer, do something with it
    if (inbuffer[0] == 'S') {
	// Begin debug
	sprintf(textbuff, "Stop %d", inbuffer[2] - 48);
	display_twolines("Command:", textbuff);
	// End debug
	stop_motion();
	return;
    }
    memno = inbuffer[2] - 48;
    if (inbuffer[1] == 'M') {
	int heading = heading_memory[memno];
	// Begin debug
	sprintf(textbuff, "Move %d: %d", inbuffer[2] - 48, heading);
	display_twolines("Command:", textbuff);
	// End debug
	// move to a memory
	move_to(heading);
    }
    else if (inbuffer[1] == 'P') {
	int frac = 0;
	int azimuth = 0;
	// set a memory
	// round any fractional degrees
	if (inbuffer[6] == '.') {
	    frac = inbuffer[7]-48;
	}
	azimuth = (100 * (inbuffer[3]-48)) + (10 * (inbuffer[4]-48)) + (inbuffer[5]-48);
	if ((frac > 4) && (frac <= 9))
	    azimuth++;
	heading_memory[memno] = azimuth;
	// Begin debug
	sprintf(textbuff, "Save %d: %d", inbuffer[2] - 48, azimuth);
	display_twolines("Command:", textbuff);
	// End debug
    }
    else {
	// must be a read heading command 'I'
	int az;
	az = get_azimuth();
	sprintf(textbuff, "%03d;", az);
	Serial.print(textbuff);
	// Begin debug
	sprintf(textbuff, "Query %d: %d", inbuffer[2] - 48, az);
	display_twolines("Command:", textbuff);
	// End debug
    }
}

int handle_serial_input(void) {
    int inbyte = 0;

    // Read whatever characters are available
    inbyte = Serial.read();
    if (state == SRS_IDLE) {
	// see if the first character is a valid command start
	if (inbyte == ';') {
	    // if we get this while idle, stop all motion
	    stop_motion();
	}
	else if ((inbyte == 'A') || (inbyte == 'S')) {
	    state = SRS_PARTIAL_COMMAND1;
	    inbuffer[0] = inbyte;
	}
    }
    else if (state == SRS_PARTIAL_COMMAND1) {
	// see if the second character is valid
	if (inbuffer[0] == 'A') {
	    if ((inbyte == 'I') || (inbyte == 'P') || (inbyte == 'M')) {
		state = SRS_PARTIAL_COMMAND2;
		inbuffer[1] = inbyte;
	    }
	    else
		// Error
		state = SRS_UNSYNCED;
	}
	else if (inbuffer[0] == 'S') {
	    if (inbyte == 'T') {
		state = SRS_PARTIAL_COMMAND2;
		inbuffer[1] = inbyte;
	    }
	    else
		// Error
		state = SRS_UNSYNCED;
	}
	else
	    state = SRS_UNSYNCED;
    }
    else if (state == SRS_PARTIAL_COMMAND2) {
	// we're expecting a single digit
	if ((inbyte >= '0') && (inbyte <= '9')) {
	    inbuffer[2] = inbyte;
	    if (inbuffer[1] == 'P') {
		state = SRS_NEED_HEADING;
		heading_digits = 0;
	    }		    
	    else {
		state = SRS_NEED_TERMINATOR;
	    }
	}
	else
	    state = SRS_UNSYNCED;
    }
    else if (state == SRS_NEED_HEADING) {
	if (heading_digits == 4) {
	    if (inbyte == '.') {
		inbuffer[6] = inbyte;
		heading_digits++;
	    }
	    else if (inbyte == ';') {
		state = SRS_IDLE;
		process_serial_command();
	    }
	}
	if (((inbyte >= '0') && (inbyte <= '9')) && (heading_digits < 5)) {
	    inbuffer[3 + heading_digits++] = inbyte;
	    if (heading_digits == 5) {
		    state = SRS_NEED_TERMINATOR;
		}
	}
	else
	    state = SRS_UNSYNCED;
    }
    else if ((state == SRS_NEED_TERMINATOR) && (inbyte == ';')) {
	    state = SRS_IDLE;
	    process_serial_command();
    }
    else if (state == SRS_UNSYNCED) {
	// we are lost - just wait for a ';'
	if (inbyte == ';')
	    state == SRS_IDLE;
    }
    else {
	rotor_error(ERR_SERIAL_STATE);
    }
}

