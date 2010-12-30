/*
  Antenna Rotor Controller
  I2C Expansion code
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

  This was helpful for me:
  http://www.neufeld.newton.ks.us/electronics/?p=241
*/

#include <Wire.h>
#include "expansion.h"

void init_expansion(void) {
  Wire.begin();
  expansion_dir(EXPIOSET);
  return;
}

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

unsigned int expansion_read(void) {
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

