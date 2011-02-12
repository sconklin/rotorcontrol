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
#include "error.h"

// DCU-1 protocol:
// (Max of 9 characters in a command)
// ; - stops all rotation
// AIn; - reads heading, returns three digits leading zeroes "xxx;"
// STn; - stops all rotation
// APnxxx(.y); - Sets rotation degrees to xxx degrees and optionslly y tenths for next AMn; command
// AMn; send rotor to last APn setting


// Globals for serial handler code

static unsigned char serial_state;

static int inbuffer[12];

int init_serial(void) {
  Serial.begin(9600);
}

#define SRS_IDLE = 0
#define SRS_PARTIAL_COMMAND1 = 1
#define SRS_PARTIAL_COMMAND2 = 2
#define SRS_NEED_HEADING = 3
//#define SRS_HAVE_HEADING = 4
#define SRS_NEED_TERMINATOR = 5
#define SRS_UNSYNCED = 6

void process_serial_command(void) {
}

int handle_serial_input(void) {
    int inbyte = 0;
    static unsigned char state = SRS_IDLE;
    static unsigned char heading_digits = 0

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
		process_serial_command()
	    }
	}
	if (((inbyte >= '0') && (inbyte <= '9')) && (heading_digits < 5)) {
	    inbuffer[3 + heading_digits++] = inbyte;
	    if heading_digits == 5 {
		    state = SRS_NEED_TERMINATOR
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
	rotor_error(ERR_SERIAL_STATE)
    }
}

