/*
  Antenna Rotor Controller
  
  Movement code

 Copyright 2011 Steve Conklin AI4QR <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef _MOVEMENT_H_
#define _MOVEMENT_H_


void move_to(int azimuth);
void stop_motion(void);
int get_azimuth(void);
int get_target(void);

#endif /* _MOVEMENT_H_ */
