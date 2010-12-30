// -*- Mode:c -*-
/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

http://www.neufeld.newton.ks.us/electronics/?p=241
*/
//#include <Wire.h>
#include <LiquidCrystal.h>
#include "rotor.h"
#include "encoders.h"
#include "expansion.h"

// globals for testing
unsigned char lastinput, thisinput;

/*
void display_error(char *errstr) {
  // TODO
  //lcd.setCursor(0, 1);
  //lcd.print(errstr);
  return;
}
*/

LiquidCrystal lcd(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

void setup() {
  int i, j;
  
  Serial.begin(9600);

  init_expansion();

  init_encoders();

  lcd = LiquidCrystal(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

  lcd.begin(LCDWIDTH, LCDHEIGHT);
  Serial.println("conklinhouse.com");
  lcd.print("conklinhouse.com");
  
  // init the input variables
  // TODO check for unexpected state here
  lastinput = expansion_read();

}

void loop() {
  unsigned char value;
  int numser;
  int encoder_moved;
  
  // Check for serial data
  /*
  numser = Serial.available();
  if (numser) {
    // TODO check for a high number here, don't want overflow
    // inbyte = Serial.read()
  }
*/
  // Check to see if we got any input events
  if (digitalRead(2) == 0) {
    // Input changed since our last read
    unsigned char changes;
    thisinput = expansion_read();
    //Serial.println(thisinput, BIN);
    changes = thisinput ^ lastinput;
    if ((changes & RBIN) && (thisinput & RBIN)) {
      // right button changed
      Serial.println("right button press");
    }
    if ((changes & LBIN) && (thisinput & LBIN)) {
      // left button changed
      Serial.println("left button press");
    }
    if (changes & (RENA | RENB)) {
      // right encoder changed
      unsigned char encval = (thisinput & (RENA | RENB)) >> RENSHIFT;
      encoder_moved = do_enc_state(encval, R_ENC);
      if (encoder_moved) {
        Serial.print("Right ");
        Serial.println(encoder_moved, DEC);
      }
    }
    if (changes & (LENA | LENB)) {
      // left encoder changed
      unsigned char encval = (thisinput & (LENA | LENB)) >> LENSHIFT;
      encoder_moved = do_enc_state(encval, L_ENC);
      if (encoder_moved) {
        Serial.print("Left ");
        Serial.println(encoder_moved, DEC);
      }
    }
    lastinput = thisinput;
  }
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  
  //delay(500);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  //Serial.println("Hello World!");
}

