// -*- Mode:c -*-
/*
  Antenna Az/El Rotor Controller
 
 Copyright 2013 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/
#include <Wire.h>
#include <LiquidTWI.h>
#include <MemoryFree.h>

#define serBufferMax 40
#define JOG_DELAY 50
#define IDLE_TIMEOUT_COUNTER 30000

// Board pin connections
int az_pwr_pin = 4;
int el_pwr_pin = 5;
int az_dir_pin = 2;
int el_dir_pin = 3;

// Analog Inputs
int az_pos_pin = A0;
int el_pos_pin = A1;

// Serial input stuff
int SERbfwp;
int cmdsz;
char *bufCpy;
char SERbuffer[serBufferMax];
char CMDbuffer[serBufferMax];

// Position variables
int az_position = 0;
int el_position = 0;
int az_target = 0;
int el_target = 0;
float az_degrees = 0.0;
float el_degrees = 0.0;
//float az_target = 0.0;
//float el_target = 0.0;
float az_target_degrees = 0.0;
float el_target_degrees = 0.0;
float az_degrees_per_count = 0.0;
float el_degrees_per_count = 0.0;
int az_commanded = 0;
int el_commanded = 0;
int az_countdown = 0;
int el_countdown = 0;

// Position calibration variables
// TODO move to eeprom storage
// NOTE - Elevation end stops are 2 counts past 0 and 180 degrees
int az_low_end  = 204;
int az_high_end = 817;
int el_low_end = 361;
int el_high_end = 666; // for 180 degrees

// Beyond these limits, we assume an analog reading is a sensor failure
int low_limit = 100;
int high_limit = 900;

#define NUM_SAMPLES 10
int az_pos[NUM_SAMPLES];
int el_pos[NUM_SAMPLES];
int sample_number = 0;

int ii = 0;

int hang = 0;

LiquidTWI lcd(0);

void haltText(String message1, String message2, String message3, String message4)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message1);
  lcd.setCursor(0, 1);
  lcd.print(message2);
  lcd.setCursor(0, 2);
  lcd.print(message3);
  lcd.setCursor(0, 3);
  lcd.print(message4);
  while (true)
    hang++;

}

int azMoving(void)
{
  return bitRead(PORTD,az_pwr_pin);
}

int elMoving(void)
{
  return bitRead(PORTD,el_pwr_pin);
}

int azLeft(void)
{
  return !bitRead(PORTD,az_dir_pin);
}

int azRight(void)
{
  return bitRead(PORTD,az_dir_pin);
}

int elUp(void)
{
  return bitRead(PORTD,el_dir_pin);
}

int elDown(void)
{
  return !bitRead(PORTD,el_dir_pin);
}

void startAz(void)
{
  digitalWrite(az_pwr_pin, HIGH);
}

void startEl(void)
{
  digitalWrite(el_pwr_pin, HIGH);
}

void stopAz(void)
{
  if (azMoving())
    {
      //Serial.println(F("Stopping Az"));
      digitalWrite(az_pwr_pin, LOW);
      delay(20); // Long enough to hit a zero cross
    }
  return;
}

void stopEl(void)
{
  if (elMoving())
    {
      //Serial.println(F("Stopping El"));
      digitalWrite(el_pwr_pin, LOW);
      delay(20); // Long enough to hit a zero cross
    }
  return;
}

void setAzLeft(void)
{
  if (azLeft()) {
    //Serial.println(F("moving left"));
    return;
  }
  stopAz();
  digitalWrite(az_dir_pin, LOW);
}

void setAzRight(void)
{
  if (azRight()) {
    //Serial.println(F("moving right"));
    return;
  }
  stopAz();
  digitalWrite(az_dir_pin, HIGH);
}

void setElDown(void)
{
  if (elDown()) {
    //Serial.println(F("moving down"));
    return;
  }
  stopEl();
  digitalWrite(el_dir_pin, LOW);
}

void setElUp(void)
{
  if (elUp()) {
    //Serial.println(F("moving up"));
    return;
  }
  stopEl();
  digitalWrite(el_dir_pin, HIGH);
}

void sendAz(void)
{
  Serial.print(F("AZ"));
  Serial.println(az_degrees, 1);
}

void sendEl(void)
{
  Serial.print(F("EL"));
  Serial.println(el_degrees, 1);
}

void sendAzEl(void)
{
  Serial.print(F("AZ"));
  Serial.print(az_degrees, 1);
  Serial.print(F(" EL"));
  Serial.println(el_degrees, 1);
}

int degrees2Az(float deg)
{
  return (int(deg / az_degrees_per_count)+ az_low_end);
}

int degrees2El(float deg)
{
  return (int(deg / el_degrees_per_count)+ el_low_end);
}

void calibrate(void)
{
  int val, oldval;
  stopAz();
  stopEl();
  // move left until we no longer go any further
  setAzLeft();
  startAz();
  oldval = 0;
  while (1)
    {
      val = analogRead(az_pos_pin);
      //val = val >> 1; // remove noise
        Serial.println(val);
	if (val == oldval) {
	  // done
	  Serial.println(F("az low end:"));
	  Serial.println(val);
	  break;
	} else {
	  oldval = val;
	}
	delay(500);
    }
  
  // move right until we no longer go any further
  setAzRight();
  startAz();
  oldval = 0;
  while (1)
    {
      val = analogRead(az_pos_pin);
      //val = val >> 1; // remove noise
        Serial.println(val);
	if (val == oldval) {
	  // done
	  Serial.println(F("az high end:"));
	  Serial.println(val);
	  break;
	} else {
	  oldval = val;
	}
        delay(500);
    }
}

void processCommand()
{
  //lcd.setCursor(0, 3);
  //lcd.print(F("                    "));
  //lcd.setCursor(0, 3);
  //lcd.print(CMDbuffer);
  // now process it
  if (strncmp(CMDbuffer, "AZ", 2) == 0) {
    // AZ
    if (cmdsz == 2) {
      // If only two characters, just print current az
      sendAz();
    } else {
      az_target_degrees = atof(&CMDbuffer[2]);
      az_target = degrees2Az(az_target_degrees);
      // check range
      if ((az_target >= az_low_end) && (az_target <= az_high_end))
	{
	  if (az_position < az_target) {
	    setAzRight();
	    startAz();
	    az_commanded = 1;
	  } else if (az_position > az_target) {
	    setAzLeft();
	    startAz();
	    az_commanded = 1;
	  }
	  //sendAz();
	}
    }
    return;
  }
  else if (strncmp(CMDbuffer, "EL", 2) == 0) {
    // EL
    if (cmdsz == 2) {
      // If only two characters, just print current az
      sendEl();
    } else {
      el_target_degrees = atof(&CMDbuffer[2]);
      el_target = degrees2El(el_target_degrees);
      // check range
      if ((el_target >= el_low_end) && (el_target <= el_high_end))
	{
	  if (el_position < el_target) {
	    setElUp();
	    startEl();
	    el_commanded = 1;
	  } else if (el_position > el_target) {
	    setElDown();
	    startEl();
	    el_commanded = 1;
	  }
	  //sendEl();
	}
    }
    return;
  }
  else if (strncmp(CMDbuffer, "JU", 2) == 0) {
    // Jog up
    setElUp();
    startEl();
    delay(JOG_DELAY);
    stopEl();
    setElDown(); // turn off relay to prevent current draw
    return;
  }
  else if (strncmp(CMDbuffer, "XXX", 3) == 0) {
    // Ignore
    return;
  }
  else if (strncmp(CMDbuffer, "JD", 2) == 0) {
    // Jog down
    setElDown();
    startEl();
    delay(JOG_DELAY);
    stopEl();
    return;
  }
  else if (strncmp(CMDbuffer, "JL", 2) == 0) {
    // Jog left
    setAzLeft();
    startAz();
    delay(JOG_DELAY);
    stopAz();
    return;
  }
  else if (strncmp(CMDbuffer, "JR", 2) == 0) {
    // Jog right
    setAzRight();
    startAz();
    delay(JOG_DELAY);
    stopAz();
    setAzLeft(); // turn off relay to prevent current draw
    return;
  }
  else if (strncmp(CMDbuffer, "UP", 2) == 0) {
    // Uplink Freq (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "DN", 2) == 0) {
    // Downlink Freq (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "DM", 2) == 0) {
    // Downlink Mode (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "UM", 2) == 0) {
    // Uplink Mode (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "DR", 2) == 0) {
    // Downlink Radio (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "UR", 2) == 0) {
    // Uplink Radio (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "ML", 2) == 0) {
    // Move Left
    setAzLeft();
    startAz();
    az_commanded = 0;
    return;
  }
  else if (strncmp(CMDbuffer, "MR", 2) == 0) {
    // Move Right
    setAzRight();
    startAz();
    az_commanded = 0;
    return;
  }
  else if (strncmp(CMDbuffer, "MU", 2) == 0) {
    // Move Up
    setElUp();
    startEl();
    el_commanded = 0;
    return;
  }
  else if (strncmp(CMDbuffer, "MD", 2) == 0) {
    // Move Down
    setElDown();
    startEl();
    el_commanded = 0;
    return;
  }
  else if (strncmp(CMDbuffer, "SA", 2) == 0) {
    // Stop Azimuth Moving
    stopAz();
    az_commanded = 0;
    setAzLeft(); // turn relay off
    return;
  }
  else if (strncmp(CMDbuffer, "SE", 2) == 0) {
    // Stop Elevation Moving
    stopEl();
    el_commanded = 0;
    setElDown(); // turn relay off
    return;
  }
  else if (strncmp(CMDbuffer, "AO", 2) == 0) {
    // AOS (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "LO", 2) == 0) {
    // LOS (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "OP", 2) == 0) {
    // Set Output number (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "IP", 2) == 0) {
    // Read an input (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "AN", 2) == 0) {
    // Read Analog Input
    ii = atoi(&CMDbuffer[2]);
    if ((ii < 0) || (ii > 5)) {
      // out of range, ignore it
      return;
    }
    Serial.print(F("AN"));
    Serial.print(ii);
    Serial.print(",");
    Serial.println(analogRead(ii+14));
    return;
  }
  else if (strncmp(CMDbuffer, "ST", 2) == 0) {
    // Set Time (ignore)
    return;
  }
  else if (strncmp(CMDbuffer, "VE", 2) == 0) {
    // Version
    Serial.println(F("VE001"));
    return;
  }
  else {
    haltText("Bad Cmd",CMDbuffer, "", ""); 
    return;
  }
}

void handleSerial(int inbyte)
{
  char *s;
  char *d;
  //
  // If there is a character available on the serial port, accumulate it into
  // a buffer. When completed by a newline, or carriage return, process it.
  //
  if (inbyte == '\n' || inbyte == '\r') {
    SERbuffer[SERbfwp++] = 0;
    cmdsz = SERbfwp - 1;
    SERbfwp = 0;

    // Assume ascii and make all letters uppercase
    s = &SERbuffer[0];
    while (*s) {
      if ((*s >= 97) && (*s <=122))
	*s &= ~(0x20);
      s++;
    }

    // First check for the only 'command' that is longer than two characters.
    // It's a combination of az and el position requests, and must be responded
    // to with a single line, although this restriction may be an implementation
    // consequence of hamlib. Subsequent commands on the same line are discarded.

    if (strncmp(SERbuffer, "AZ EL", 5) == 0) {
      // position request
      sendAzEl();
      return;
    }
    
    // cheap and dirty park implementation
    //  Subsequent commands on the same line are discarded.
    if (strncmp(SERbuffer, "PARK", 4) == 0) {
      strcpy(SERbuffer, "AZ180 EL90");
    }

    // "AZ240.4 EL26.1 UP000 XXX DN000 XXX"
    // within a command line, there can be one or more commands, separated by spaces
    // Separate them on space boundaries, and process each
    s = &SERbuffer[0];
    while (*s) {
      d = &CMDbuffer[0];
      // copy a string into cmd buffer until space or end
      while (*s && (*s != ' ')) {
	*d++ = *s++;
      }
      // if it's a space, advance past it
      if (*s == ' ')
	s++;

      *d++ = 0; // terminate the command buffer string, source points just past space or still at NULL
      cmdsz = strlen(CMDbuffer);

      // commands are exactly two characters long
      if (cmdsz < 2) {
	//Serial.println("short");
	return;
      }
      /*
      if (strncmp(CMDbuffer, "CALIBRATE", 9) == 0) {
        calibrate();
        return;
      }
      */
      processCommand();
    }
  } else {
    // accumulate the available character
    if (SERbfwp < (serBufferMax-1))
      SERbuffer[SERbfwp++] = inbyte;
    else {
      // buffer overrun, just clear buffer and start over
      //Serial.println("clear");
      SERbfwp = 0;
      SERbuffer[SERbfwp] = 0;
    }
  }
  return;
}

void setup() {
  pinMode(az_pwr_pin, OUTPUT);
  digitalWrite(az_pwr_pin, LOW);
  pinMode(el_pwr_pin, OUTPUT);
  digitalWrite(el_pwr_pin, LOW);
  pinMode(az_dir_pin, OUTPUT);
  pinMode(el_dir_pin, OUTPUT);
  
  // Seed the buffers with readings
  for (ii=0; ii<NUM_SAMPLES; ii++) {
    az_pos[ii] = analogRead(az_pos_pin);
    el_pos[ii] = analogRead(el_pos_pin);
  }
  Serial.begin(9600);

  // Initialize the LCD display
  lcd.begin(20, 4);
  // Print a message to the LCD.
  //lcd.print(F("AI4QR"));
  //lcd.autoscroll();

  // make some one-time floating point calcs
  az_degrees_per_count = 360.0/(float(az_high_end)-float(az_low_end));
  el_degrees_per_count = 180.0/(float(el_high_end)-float(el_low_end));
}

void loop() {
  // read the positions, and average
  az_pos[sample_number] = analogRead(az_pos_pin);
  el_pos[sample_number] = analogRead(el_pos_pin);
  sample_number++;
  sample_number = sample_number % NUM_SAMPLES;
  az_position = 0;
  el_position = 0;
  for (ii = 0; ii< NUM_SAMPLES; ii++) {
    az_position = az_position + az_pos[ii];
    el_position = el_position + el_pos[ii];
  }
  // NOTE, this truncates (rounds down)
  az_position = az_position / 10;
  el_position = el_position / 10;

  // Check for sensor failure
//  if ((az_position < low_limit) || (az_position > high_limit))
//    stopAz();
//  if ((el_position < low_limit) || (el_position > high_limit))
//    stopEl();

  // TODO check for stuck sensor (moving, not at target, and reading not changing)

  az_degrees = az_degrees_per_count * float(az_position-az_low_end);
  el_degrees = el_degrees_per_count * float(el_position-el_low_end);

  if (az_countdown) {
    if (!--az_countdown) {
      setAzLeft(); // turn off relay
    }
  }

  if (el_countdown) {
    if (!--el_countdown) {
      setElDown(); // turn relay off
    }
  }

  if (az_commanded) {
    az_countdown = IDLE_TIMEOUT_COUNTER;
    if (azLeft() && (az_position <= az_target)) {
      stopAz();
      az_commanded = 0;
    } else if  (azRight() && (az_position >= az_target)) {
      stopAz();
      az_commanded = 0;
    }
  }

  if (el_commanded) {
    el_countdown = IDLE_TIMEOUT_COUNTER;
    if (elDown() && (el_position <= el_target)) {
      stopEl();
      el_commanded = 0;
    } else if  (elUp() && (el_position >= el_target)) {
      stopEl();
      el_commanded = 0;
    }
  }

  // Check for serial data
  while(Serial.available()) {
    handleSerial(Serial.read());
  }

  // Print debug output to LCD
  // There are four lines.
  // Top two reserved for operational info
  // Bottom two for debugging
  //
  //   0123456789001234567890
  // 0 TGT: AZ:nnn.n EL nnn.n
  // 1 POS: AZ:nnn.n EL nnn.n
  // 2 
  // 3 
  //
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  lcd.print(F("PA "));
  lcd.print(az_degrees, 0);
  lcd.print(F(" E "));
  lcd.print(el_degrees, 0);
  lcd.print(F("  "));

  lcd.setCursor(0, 1);
  lcd.print(F("TA "));
  lcd.print(az_target_degrees, 0);
  lcd.print(F(" E "));
  lcd.print(el_target_degrees, 0);
  lcd.print(F("  "));
  
  lcd.setCursor(0, 2);
  lcd.print(F("PA "));
  lcd.print(az_position);
  lcd.print(F(" E "));
  lcd.print(el_position);
  lcd.print(F("  "));

  lcd.setCursor(0, 3);
  lcd.print(F("TA "));
  lcd.print(az_target);
  lcd.print(F(" E "));
  lcd.print(el_target);
  lcd.print(F("  "));
  
  lcd.setCursor(18, 0);
  if (az_commanded) {
    lcd.print(F("1"));
  } else {
    lcd.print(F("0"));
  }

  lcd.setCursor(18, 1);
  if (el_commanded) {
    lcd.print(F("1"));
  } else {
    lcd.print(F("0"));
  }


  //delay(500);
}

