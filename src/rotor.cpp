/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

http://www.neufeld.newton.ks.us/electronics/?p=241
*/
#include <wiring.h> // with this defined, Wire not right, without defined, digitalRead not defined
#include "Wire.h"
#include "LiquidCrystal.h"
#include <HardwareSerial.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <encoders.h>

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
#define RENSHIFT 2 // how many bits do we shift?
#define LENA 16  // Left encoder A
#define LENB 32  // Left encoder B
#define LENSHIFT 4
// 6 and 7 unused

// define encoder indices
#define L_ENC 0
#define R_ENC 1

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

//LiquidCrystal lcd(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

void setup() {
  Serial.begin(9600);

  Wire.begin();

  expansion_dir(EXPIOSET);

  init_encoders();

  //lcd = LiquidCrystal.LiquidCrystal(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

  //lcd.begin(LCDWIDTH, LCDHEIGHT);
  //Serial.println("conklinhouse.com");
  //lcd.print("conklinhouse.com");
  
  // init the input variables
  // TODO check for unexpected state here
  lastinput = expansion_read();
}

void loop() {
  unsigned char value;
  int numser;
  int encoder_moved;
  
  // Check for serial data
  numser = Serial.available();
  if (numser) {
    // TODO check for a high number here, don't want overflow
    // inbyte = Serial.read()
  }

  // Check to see if we got any input events
  if (digitalRead(2) == 0) {
    // Input changed since our last read
    unsigned char changes;
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
      unsigned char encval = (changes & (RENA | RENB)) >> RENSHIFT;
      Serial.print("right encoder ");
      Serial.print(encval, BIN);
      encoder_moved = do_enc_state(encval, R_ENC);
      Serial.println("");
      Serial.println("Right: ");
      Serial.print(encoder_moved, DEC);
      Serial.println("");
    }
    if (changes & (LENA | LENB)) {
      // left encoder changed
      unsigned char encval = (changes & (LENA | LENB)) >> LENSHIFT;
      Serial.print("left encoder");
      Serial.print(encval, BIN);
      encoder_moved = do_enc_state(encval, L_ENC);
      Serial.println("");
      Serial.println("Left: ");
      Serial.print(encoder_moved, DEC);
      Serial.println("");
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

