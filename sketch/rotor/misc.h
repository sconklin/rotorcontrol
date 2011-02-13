/*
  Antenna Rotor Controller
  
  Miscellaneous things

 Copyright 2011 Steve Conklin AI4QR <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef _MISC_H_
#define _MISC_H_

// Error codes
#define ERR_SERIAL_STATE 1
#define ERR_SOMETHING_ELSE 2

// Globals
extern char textbuff[];

void rotor_error(int errnum);

void display_twolines(char *one, char *two);

#endif /* _MISC_H_ */