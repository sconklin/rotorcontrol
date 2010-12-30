/*
  Antenna Rotor Controller
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/
#ifndef _rotor_h_
#define _rotor_h_

// Board pin connections
#define LCDRS 7
#define LCDEN 8
#define LCDD4 9
#define LCDD5 10
#define LCDD6 11
#define LCDD7 12

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

// define encoder indices
#define L_ENC 0
#define R_ENC 1

#define EXPIOSET (~(ACOUT|CCWOUT|CWOUT))

// For LCD
#define LCDWIDTH 16
#define LCDHEIGHT 2

#endif /* _rotor_h_ */
