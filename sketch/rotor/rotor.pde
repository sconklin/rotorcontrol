// -*- Mode:c -*-
/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/
#include <LiquidCrystal.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "rotor.h"
#include "encoders.h"
#include "expansion.h"

#define INIT_TIMER_COUNT 6
#define RESET_TIMER2 TCNT2 = INIT_TIMER_COUNT

// globals for input
unsigned char lastinput, thisinput;

// Possible inputs
#define L_PRESS 0x01
#define R_PRESS 0x02
#define R_CW    0x04
#define R_CCW   0x08
#define L_CW    0x10
#define L_CCW   0x20

/*
void display_error(char *errstr) {
  // TODO
  //lcd.setCursor(0, 1);
  //lcd.print(errstr);
  return;
}
*/

volatile int milliseconds = 0;
volatile int seconds = 0;

LiquidCrystal lcd(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

// Aruino runs at 16 Mhz, so we have 1000 Overflows per second…
// 1/ ((16000000 / 64) / 256) = 1 / 1000
ISR(TIMER2_OVF_vect) {
    RESET_TIMER2;
    milliseconds++;
    if (milliseconds == 1000) {
	seconds+=1;
	milliseconds = 0;
    }
}

unsigned char get_input_events(void) {
    unsigned char in_events = 0;
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

void setup() {
  
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

  // Timer2 Settings: Timer Prescaler /64,
  TCCR2A |= (1<<CS22);
  TCCR2A &= ~((1<<CS21) | (1<<CS20));
  // Use normal mode
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
  // Use internal clock – external clock not used in Arduino
  ASSR |= (0<<AS2);
  // Timer2 Overflow Interrupt Enable
  TIMSK2 |= (1<<TOIE2) | (0<<OCIE2A);
  sei();
  // End timer2 setup
}

void loop() {
  int numser;
  static unsigned char ui_events;
  
  // Check for serial data
  /*
  numser = Serial.available();
  if (numser) {
    // TODO check for a high number here, don't want overflow
    // inbyte = Serial.read()
  }
*/

  // Check to see if we got any input events
  ui_events = get_input_events();
  if (ui_events) {
      Serial.print(seconds, DEC);
      Serial.print(".");
      Serial.print(milliseconds, DEC);
      Serial.println(" seconds");
  }
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  
  //delay(500);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  //Serial.println("Hello World!");
}

