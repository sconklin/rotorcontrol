// -*- Mode:c -*-
/*
  Antenna Rotor Controller
  Error code
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#include "error.h"
void rotor_error(int errnum) {
    while (1) {
	// clear the screen
	lcd.clear();
	delay(500);
	// display the error
	lcd.setCursor(0,0);
	lcd.print("Error:");
	lcd.setCursor(0,1);
	lcd.print(errnum);
	delay(500);
    }
}

