/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

http://www.neufeld.newton.ks.us/electronics/?p=241
*/

#include <LiquidCrystal.h>
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// Board pin connections
#define LCDRS 7
#define LCDEN 8
#define LCDD4 9
#define LCDD5 10
#define LCDD6 11
#define LCDD7 12

// Other constants

// For expansion port PCA9555 IC
#define EXPADDR (0x4 << 3 | 0x0) // expansion chip I2C address (chip jumpered for 0)
#define REGISTER_INPUT0 (0)   // address of input register 0
#define REGISTER_INPUT1 (1)   // address of input register 1
#define REGISTER_OUTPUT0 (2)  // address of output register 0
#define REGISTER_OUTPUT1 (3)  // address of output register 1
#define REGISTER_INVERST0 (4) // address of polarity inversion register 0
#define REGISTER_INVERST1 (5) // address of polarity inversion register 1
#define REGISTER_CONFIG0 (6)  // address of config register 0
#define REGISTER_CONFIG1 (7)  // address of config register 1

// These outputs are on port 0 of the expansion chip
#define ACOUT 1  // output to swich SSR controlling AC and solenoid
#define CCWOUT 2 // Counterclockwise SSR 
#define CWOUT 4  // Clockwise SSR
// Bits 3-7 unused

// These inputs are on port 1 of the expansion chip
#define RBIN 1  // right encoder push button
#define LBIN 2  // left encode push button
#define RENA 4  // Right encoder A
#define RENB 8  // Right encoder B
#define RENSHIFT 2
#define LENA 16  // Left encoder A
#define LENB 32  // Left encoder B
#define LENSHIFT 4
// 6 and 7 unused

#define EXPIOSET (~(ACOUT|CCWOUT|CWOUT))

// For LCD
#define LCDWIDTH 16
#define LCDHEIGHT 2

// globals for testing
byte lastinput, thisinput;

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

  /*
   * State table (For each encoder):
   *
   * '*' shouldn't happen, as only encoder value changes are processed
   *
   * State Name | New encoder Val | Next State
   * -----------|-----------------|-----------
   * REST       |       0         | WF3 (wait for 3)
   *            |       1         | CW1
   *            |       2         | CCW1
   *            |       3         | REST
   * -----------|-----------------|-----------
   * WF3        |       0         | WF3
   *            |       1         | WF3
   *            |       2         | WF3
   *            |       3         | REST
   * -----------|-----------------|-----------
   * CW1        |       0         | CW2
   *            |       1         | CW1 *
   *            |       2         | WF3
   *            |       3         | REST
   * -----------|-----------------|-----------
   * CW2        |       0         | CW2 *
   *            |       1         | WF3
   *            |       2         | CW3
   *            |       3         | REST
   * -----------|-----------------|-----------
   * CW3        |       0         | WF3
   *            |       1         | WF3
   *            |       2         | CW3 *
   *            |       3         | REST (CW++)
   * -----------|-----------------|-----------
   * CCW1       |       0         | CCW2
   *            |       1         | WF3
   *            |       2         | CCW1 *
   *            |       3         | REST
   * -----------|-----------------|-----------
   * CCW2       |       0         | CCW2 *
   *            |       1         | CCW3
   *            |       2         | WF3
   *            |       3         | REST
   * -----------|-----------------|-----------
   * CCW3       |       0         | WF3
   *            |       1         | CCW3 *
   *            |       2         | WF3
   *            |       3         | REST (CCW++)
   * -----------|-----------------|-----------
   */

// Globals for rotary encoder state machine
static byte lenc_state, renc_state;
const static byte next-state[RS_NUM_INPUT_VALUES][RS_MAX_STATES] = {
    RS_WF3,  RS_CW1,  RS_CCW1, RS_REST,                // RS_REST
    RS_WF3,  RS_WF3,  RS_WF3,  RS_REST,                // RS_WF3
    RS_CW2,  RS_CW1,  RS_WF3,  RS_REST,                // RS_CW1
    RS_CW2,  RS_WF3,  RS_CW3,  RS_REST,                // RS_CW2
    RS_WF3,  RS_WF3,  RS_CW3,  RS_REST | RS_INC_CW,    // RS_CW3
    RS_CCW2, RS_WF3,  RS_CCW1, RS_REST,                // RS_CCW1
    RS_CCW2, RS_CCW3, RS_WF3,  RS_REST,                // RS_CCW2
    RS_WF3,  RS_CCW3, RS_WF3,  RS_REST | RS_INC_CCW    // RS_CCW3
};

void expansion_dir(int dir) {
  //  Send config register address
  Wire.beginTransmission(EXPADDR);
  Wire.send(REGISTER_CONFIG0);

  //  Connect to device and send two bytes
  Wire.send(0xff & dir);  //  low byte
  Wire.send(dir >> 8);    //  high byte

  Wire.endTransmission();
}

void gpio_write(int address, int data) {
  //  Send output register address
  Wire.beginTransmission(address);
  Wire.send(REGISTER_OUTPUT0);

  //  Connect to device and send two bytes
  Wire.send(0xff & data);  //  low byte
  Wire.send(data >> 8);    //  high byte

  Wire.endTransmission();
}

unsigned int expansion_read() {
  byte data = 0;

  //  Send input register address
  Wire.beginTransmission(EXPADDR);
  Wire.send(REGISTER_INPUT1);
  Wire.endTransmission();

  //  Connect to device and request two bytes
  Wire.beginTransmission(EXPADDR);
  Wire.requestFrom(EXPADDR, 1);

  if (Wire.available()) {
    data = Wire.receive();
  }

  Wire.endTransmission();

  return data;
}

char *display_error(char *errstr) {
  lcd.setCursor(0, 1);
  lcd.print(errstr);
}

static unsigned int missed_steps;

int init_encoders(void) {
    lenc_state = renc_state = RS_REST;
}

int do_enc_state(byte newval, byte right) {
    byte oldstate, newstate;
    int count = 0;
  
    if right
	oldstate = renc_state;
    else
	oldstate = lenc_state;

    newstate = next-state[state][newval];
    // Check the high bits to see whether we completed a click
    if (newstate & RS_INC_CCW) {
	count--;
    } else if (newstate & RS_INC_CW) {
	count++;
    }

    // get the clean state
    newstate &= RS_MASK;

    if right
	renc_state = newstate;
    else
	lenc_state = newstate;

    return count;
}

// Init the display
LiquidCrystal lcd(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

void setup() {
  Serial.begin(9600);

  Wire.begin();
  expansion_dir(EXPIOSET);

  init_encoders();

  lcd.begin(LCDWIDTH, LCDHEIGHT);
  //Serial.println("conklinhouse.com");
  lcd.print("conklinhouse.com");
  
  // init the input variables
  // TODO check for unexpected state here
  lastinput = expansion_read();
}

void loop() {
  byte value;
  int numser;
  int encoder_moved;
  
  // Check for serial data
  numser = Serial.available();
  if (numser) {
    // TODO check for a high number here, dpn't want overflow
    // inbyte = Serial.read()
  }

  // Check to see if we got any input events
  if (digitalRead(2) == 0) {
    // Input changed since our last read
    byte changes;
    thisinput = expansion_read();
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
      byte encval = (changes & (RENA | RENB)) >> RENSHIFT;
      Serial.print("right encoder ");
      Serial.print(encval, BIN);
      encoder_moved = do_enc_state(encval, 1);
      Serial.println("");
      Serial.println("Right: ");
      Serial.print(encoder_moved, DEC);
      Serial.println("");
    }
    if (changes & (LENA | LENB)) {
      // left encoder changed
      byte encval = (changes & (LENA | LENB)) >> LENSHIFT;
      Serial.print("left encoder");
      Serial.print(encval, BIN);
      encoder_moved = do_enc_state(encval, 0);
      Serial.println("");
      Serial.println("Left: ");
      Serial.print(encoder_moved, DEC);
      Serial.println("");
    }
    lastinput = thisinput;
  }
  
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  
  //delay(500);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  //Serial.println("Hello World!");
}

