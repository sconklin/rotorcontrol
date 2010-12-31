// -*- Mode:c -*-
/*
  Antenna Rotor Controller
  Timers code
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#include "timers.h"
extern volatile int milliseconds;
extern volatile int seconds;

struct timerinfo {
    unsigned int millis;
    unsigned int seconds;
    unsigned char inuse;
    unsigned char expired;
};

static struct timerinfo timers[MAX_TIMERS];

void init_timers(void) {
    unsigned char i;
    for (i=0; i<MAX_TIMERS; i++) {
	timers[i].inuse = 0;
	timers[i].expired = 0;
    }
    return;
}

void set_timer(int timer_num, int timeout) {
    unsigned int now_millis, now_seconds;
    now_seconds = seconds;
    now_millis = milliseconds;
    // avoid timer rollover
    if (now_millis < 100)
	now_seconds = seconds;

    timers[timer_num].millis =  (timeout % 1000) + now_millis;
    timers[timer_num].seconds = (timeout / 1000) + now_seconds;
    timers[timer_num].inuse = 1;
    timers[timer_num].expired = 0;
}

unsigned char get_timers(void) {
    unsigned int now_millis, now_seconds, tmp;
    unsigned char i;
    unsigned char result = 0;
    now_seconds = seconds;
    now_millis = milliseconds;
    // avoid timer rollover
    if (now_millis < 100)
	now_seconds = seconds;

    for (i=0; i<MAX_TIMERS; i++) {
	if ((timers[i].inuse) && (!timers[i].expired)) {
	    // has it expired?
	    tmp = now_seconds - timers[i].seconds;
	    if ((tmp >= 0) && (tmp < 6000)) {
		// arbitrary max six minutes that timers will work
		if (tmp > 0) {
		    timers[i].expired = 1;
		    result |= (1 << i);
		} else if (timers[i].millis >= now_millis) {
		    // the seconds match - expired maybe?
		    timers[i].expired = 1;
		    result |= (1 << i);
		} 
	    }
	}
    }
    return result;
}



