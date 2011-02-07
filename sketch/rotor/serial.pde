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

// DCU-1 protocol:
// (Max of 9 characters in a command)
// ; - stops all rotation
// AIn; - reads heading, returns three digits leading zeroes "xxx;"
// STn; - stops all rotation
// APnxxx(.y); - Sets rotation degrees to xxx degrees and optionslly y tenths for next AMn; command
// AMn; send rotor to last APn setting


// Globals for serial handler code

static unsigned char serial_state;

static unsigned char inbuffer[12];

int init_serial(void) {
}

int handle_serial_input(void) {
}

