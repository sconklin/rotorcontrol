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
#include "timers.h"

#define INIT_TIMER_COUNT 6
#define RESET_TIMER2 TCNT2 = INIT_TIMER_COUNT

static unsigned char going_right;
static unsigned char out_port;

// Globals so that the timer functions can read them
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

void print_direction() {
  lcd.setCursor(0,1);
  if (going_right)
    lcd.print("right");
  else
    lcd.print("left ");
  }

void bit_on(unsigned char fbit) {
  unsigned char foo;
  foo = out_port | fbit;
  if ((foo & CWOUT) && (foo & CCWOUT)) {
      //Serial.println("Error - attempt to drive both directions");
    return;
  }
  out_port |= fbit;
  gpio_write(EXPADDR, out_port);
  return;
}

void bit_off(unsigned char fbit) {
  out_port &= ~fbit;
  gpio_write(EXPADDR, out_port);
}

void motor_off(void) {
  out_port &= ~(CWOUT | CCWOUT);
  gpio_write(EXPADDR, out_port);
}

void setup() {
  
  out_port = 0;

  init_expansion();
  gpio_write(EXPADDR, 0);

  init_encoders();
  init_timers();
  init_serial();

  lcd = LiquidCrystal(LCDRS, LCDEN, LCDD4, LCDD5, LCDD6, LCDD7);

  lcd.begin(LCDWIDTH, LCDHEIGHT);

  lcd.print("conklinhouse.com");
  going_right = 0;
  print_direction();

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
  static unsigned char motor_on;
  unsigned char tcheck;
  
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
  /*
  if (ui_events) {
      Serial.print(seconds, DEC);
      Serial.print(".");
      Serial.print(milliseconds, DEC);
      Serial.println(" seconds");
  }
  */
  // Testing - left encoder selects direction
  if ((ui_events & L_CCW) || (ui_events & L_CW)) {
    going_right = !going_right;
    print_direction();
  }

  if (ui_events & L_PRESS) {
    if (motor_on) {
	//Serial.println("Refusing to turn motor on twice");
    } else {
      // Start rotor motor on left press
      // Engage lock
      //Serial.println("brake disengaged");
      bit_on(ACOUT);
      // Engage motor
      if (going_right) {
	  //Serial.println("motor on right");
        bit_on(CWOUT);
      } else {
	  //Serial.println("motor on left");
        bit_on(CCWOUT);
      }
      motor_on = 1;
    }
  }

  if (ui_events & R_PRESS) {
    // Turn off motor and set timer
    //Serial.println("Motor off - start timer");
    motor_off();
    set_timer(0, 5000);
  }
    
  tcheck = get_timers();
  if (tcheck) {
      //Serial.println("timer expired - engage brake");
    // engage lock
    bit_off(ACOUT);
    motor_on = 0;
  }
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  
  //delay(500);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  //Serial.println("Hello World!");
}

