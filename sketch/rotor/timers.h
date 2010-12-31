/*
  Antenna Rotor Controller
  Timers code

  Note: Not intended to be perfectly accurate.
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef _TIMERS_H_
#define _TIMERS_H_


#define TIMER_LOCK 1
#define TIMER_MENU 2
#define MAX_TIMERS 2 // Keep this updated

#define TIMER_LOCK_BIT 0x01
#define TIMER_MENU_BIT 0x02

void init_timers(void);
void set_timer(int timer_num, int timeout);
unsigned char get_timers(void);

#endif /* _TIMERS_H_ */
