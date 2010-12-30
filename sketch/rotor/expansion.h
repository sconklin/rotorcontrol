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

#ifndef _EXPANSION_H_
#define _EXPANSION_H_

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

void expansion_dir(int);
void gpio_write(int, int);
unsigned int expansion_read(void);

#endif /* _EXPANSION_H_ */
