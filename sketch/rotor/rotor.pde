/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

http://www.neufeld.newton.ks.us/electronics/?p=241
*/
#include <Wire.h>
#include <LiquidCrystal.h>

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

#define NUM_ENCODERS 2

// define encoder indices
#define L_ENC 0
#define R_ENC 1

//
// Begin encoder defines
//



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

// Globals for rotary encoder state machine

static unsigned char enc_state[NUM_ENCODERS];

const static unsigned char next_state[RS_MAX_STATES][RS_NUM_INPUT_VALUES] = {
//const static unsigned char next_state[RS_NUM_INPUT_VALUES][RS_MAX_STATES] = {
    RS_WF3,  RS_CCW1, RS_CW1,  RS_REST,                // 0 RS_REST
    RS_WF3,  RS_WF3,  RS_WF3,  RS_REST,                // 1 RS_WF3
    RS_CCW2, RS_CCW1, RS_WF3,  RS_REST,                // 2 RS_CCW1
    RS_CCW2, RS_WF3,  RS_CCW3, RS_REST,                // 3 RS_CCW2
    RS_WF3,  RS_WF3,  RS_CCW3, RS_REST | RS_INC_CCW,   // 4 RS_CCW3
    RS_CW2,  RS_WF3,  RS_CW1,  RS_REST,                // 5 RS_CW1
    RS_CW2,  RS_CW3,  RS_WF3,  RS_REST,                // 6 RS_CW2
    RS_WF3,  RS_CW3,  RS_WF3,  RS_REST | RS_INC_CW     // 7 RS_CW3
};

//
// End encoder defines
//

#define EXPIOSET (~(ACOUT|CCWOUT|CWOUT))

// For LCD
#define LCDWIDTH 16
#define LCDHEIGHT 2

// globals for testing
unsigned char lastinput, thisinput;

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
  unsigned char data = 0;

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

/*
void display_error(char *errstr) {
  // TODO
  //lcd.setCursor(0, 1);
  //lcd.print(errstr);
  return;
}
*/

//
// Begin encoder functions
//

int init_encoders(void) {
    unsigned char i;
    for (i=0; i<NUM_ENCODERS; i++)
	enc_state[i] = RS_REST;
}

int do_enc_state(unsigned char newval, unsigned char encoder) {
    unsigned char oldstate, newstate;
    int count = 0;

    if (encoder >= NUM_ENCODERS)
	// TODO error
	return 0;
  
    oldstate = enc_state[encoder];

    newstate = next_state[oldstate][newval];

    // Check the high bits to see whether we completed a click
    if (newstate & RS_INC_CCW) {
	count--;
    } else if (newstate & RS_INC_CW) {
	count++;
    }

    // get the clean state
    newstate &= RS_MASK;

    enc_state[encoder] = newstate;

    return count;
}

//
// End encoder functions
//

LiquidCrystal lcd(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

void setup() {
  int i, j;
  
  Serial.begin(9600);

  Wire.begin();

  expansion_dir(EXPIOSET);

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

