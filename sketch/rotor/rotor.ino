// -*- Mode:c -*-
/*
  Antenna Az/El Rotor Controller
 
 Copyright 2013 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/
//#include <avr/io.h>

#define serBufferMax 40

// Board pin connections
int az_pwr_pin = 4;
int el_pwr_pin = 5;
int az_dir_pin = 2;
int el_dir_pin = 3;

// Analog Inputs
int az_pos_pin = A0;
int el_pos_pin = A1;

// Serial input stuff
int SERbfsz;
int cmdsz;
char *bufCpy;
char SERbuffer[serBufferMax];
char CMDbuffer[serBufferMax];

//static unsigned char az_moving;
//static unsigned char el_moving;
//static unsigned char az_dir;
//static unsigned char el_dir;

// Position variables
int az_position = 0;
int el_position = 0;
int az_target = 0;
int el_target = 0;
float az_degrees = 0.0;
float el_degrees = 0.0;
float az_target_degrees = 0.0;
float el_target_degrees = 0.0;
float az_degrees_per_count = 0.0;
float el_degrees_per_count = 0.0;
int az_commanded = 0;
int el_commanded = 0;

// Position calibration variables
// TODO move to eeprom storage
int az_low_end  = 206;
int az_high_end = 817;
int el_low_end = 359;
int el_high_end = 668; // for 180 degrees

#define NUM_SAMPLES 10
int az_pos[NUM_SAMPLES];
int el_pos[NUM_SAMPLES];
int sample_number = 0;

int ii = 0;

int azMoving(void)
{
  return digitalRead(az_pwr_pin);
}

int elMoving(void)
{
  return digitalRead(el_pwr_pin);
}

int azLeft(void)
{
  return !digitalRead(az_dir_pin);
}

int azRight(void)
{
  return digitalRead(az_dir_pin);
}

int elUp(void)
{
  return digitalRead(el_dir_pin);
}

int elDown(void)
{
  return !digitalRead(el_dir_pin);
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
      digitalWrite(az_pwr_pin, LOW);
      delay(20); // Long enough to hit a zero cross
    }
  return;
}

void stopEl(void)
{
  if (elMoving())
    {
      digitalWrite(el_pwr_pin, LOW);
      delay(20); // Long enough to hit a zero cross
    }
  return;
}

void setAzLeft(void)
{
  if (azLeft())
      return
  stopAz();
  digitalWrite(az_dir_pin, LOW);
}

void setAzRight(void)
{
  if (azRight())
      return
  stopAz();
  digitalWrite(az_dir_pin, HIGH);
}

void setElDown(void)
{
  if (elDown())
      return
  stopEl();
  digitalWrite(el_dir_pin, LOW);
}

void setElUp(void)
{
  if (elUp())
      return
  stopEl();
  digitalWrite(el_dir_pin, HIGH);
}

void sendAz(void)
{
  Serial.print("AZ");
  Serial.println(az_degrees, 1);
}

void sendEl(void)
{
  Serial.print("EL");
  Serial.println(el_degrees, 1);
}

void sendAzEl(void)
{
  Serial.print("AZ");
  Serial.print(az_degrees, 1);
  Serial.print(" EL");
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
  // THIS IS BROKEN
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
	  Serial.println("az low end:");
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
	  Serial.println("az high end:");
	  Serial.println(val);
	  break;
	} else {
	  oldval = val;
	}
        delay(500);
    }
}

void handleSerial(int inbyte)
{
  //
  // If there is a character available on the serial port, accumulate it into
  // a buffer. When completed by a newline, or carriage return, process it.
  //
  if (inbyte == '\n' || inbyte == '\r') {
    SERbuffer[SERbfsz++] = 0;
    cmdsz = SERbfsz - 1;
    SERbfsz = 0;

    // Assume ascii and make all letters uppercase
    for (ii=0; ii < cmdsz; ii++)
      if ((SERbuffer[ii] >= 97) && (SERbuffer[ii] <=122))
	SERbuffer[ii]  &= ~(0x20);

    // First check for the only 'command' that is longer than two characters.
    // It's a combination of az and el position requests, and must be responded
    // to with a single line, although this restriction may be an implementation
    // consequence of hamlib

    if (strncmp(SERbuffer, "AZ EL", 5) == 0) {
      // position request
      sendAzEl();
      return;
    }
    
    // within a command line, there can be one or more commands, separated by spaces
    // Separate them on space boundaries, and process each
    bufCpy = &SERbuffer[0];
    while (*bufCpy != 0) {
      ii = 0;
      while (1) {
	  if (*bufCpy == 0) {
	    // end of serial input buffer
	    CMDbuffer[ii++] = 0;
	    break;
	  } else if (*bufCpy == ' ') {
	    // process what we've copied so far
	    CMDbuffer[ii++] = 0;
	    bufCpy++;
	    break;
	  } else {
	    CMDbuffer[ii++] = *bufCpy++;
	  }
      }

      cmdsz = strlen(CMDbuffer);
      // commands are exactly two characters long
      if (cmdsz < 2) {
	//Serial.println("short");
	return;
      }

      //if (strncmp(CMDbuffer, "CALIBRATE", 9) == 0) {
      //  calibrate();
      //  return;
      //}

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
	continue;
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
	continue;
      }
      else if (strncmp(CMDbuffer, "UP", 2) == 0) {
	// Uplink Freq (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "DN", 2) == 0) {
	// Downlink Freq (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "DM", 2) == 0) {
	// Downlink Mode (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "UM", 2) == 0) {
	// Uplink Mode (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "DR", 2) == 0) {
	// Downlink Radio (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "UR", 2) == 0) {
	// Uplink Radio (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "ML", 2) == 0) {
	// Move Left
	setAzLeft();
	startAz();
	az_commanded = 0;
	continue;
      }
      else if (strncmp(CMDbuffer, "MR", 2) == 0) {
	// Move Right
	setAzRight();
	startAz();
	az_commanded = 0;
	continue;
      }
      else if (strncmp(CMDbuffer, "MU", 2) == 0) {
	// Move Up
	setElUp();
	startEl();
	el_commanded = 0;
	continue;
      }
      else if (strncmp(CMDbuffer, "MD", 2) == 0) {
	// Move Down
	setElDown();
	startEl();
	el_commanded = 0;
	continue;
      }
      else if (strncmp(CMDbuffer, "SA", 2) == 0) {
	// Stop Azimuth Moving
	stopAz();
	az_commanded = 0;
	continue;
      }
      else if (strncmp(CMDbuffer, "SE", 2) == 0) {
	// Stop Elevation Moving
	stopEl();
	el_commanded = 0;
	continue;
      }
      else if (strncmp(CMDbuffer, "AO", 2) == 0) {
	// AOS (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "LO", 2) == 0) {
	// LOS (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "OP", 2) == 0) {
	// Set Output number (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "IP", 2) == 0) {
	// Read an input (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "AN", 2) == 0) {
	// Read Analog Input (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "ST", 2) == 0) {
	// Set Time (ignore)
	continue;
      }
      else if (strncmp(CMDbuffer, "VE", 2) == 0) {
	// Version
	Serial.println("VE001");
	continue;
      }
    }
  }
  else {
    // accumulate the available character
    if (SERbfsz < (serBufferMax-1))
      SERbuffer[SERbfsz++] = inbyte;
    else {
      // buffer overrun, just clear buffer and start over
      //Serial.println("clear");
      SERbfsz = 0;
    }
    return;
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

  // make some one-time floating point calcs
  az_degrees_per_count = 360.0/(float(az_high_end)-float(az_low_end));
  el_degrees_per_count = 180.0/(float(el_high_end)-float(el_low_end));
}

void loop() {
  int numser;
  
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

  az_degrees = az_degrees_per_count * float(az_position-az_low_end);
  el_degrees = el_degrees_per_count * float(el_position-el_low_end);

  if (az_commanded)
    {
      if (azLeft() && (az_position <= az_target)) {
	stopAz();
	az_commanded = 0;
	//Serial.print(az_position);
	//Serial.print("  ");
	//Serial.println(az_target);
      } else if  (azRight() && (az_position >= az_target)) {
	stopAz();
	az_commanded = 0;
	//Serial.print(az_position);
	//Serial.print("  ");
	//Serial.println(az_target);
      }
    }

  if (el_commanded)
    {
      if (elDown() && (el_position <= el_target)) {
	stopEl();
	el_commanded = 0;
	//Serial.print(el_position);
	//Serial.print("  ");
	//Serial.println(el_target);
      } else if  (elUp() && (el_position >= el_target)) {
	stopEl();
	el_commanded = 0;
	//Serial.print(el_position);
	//Serial.print("  ");
	//Serial.println(el_target);
      }
    }

  // Check for serial data
  while(Serial.available()) {
    handleSerial(Serial.read());
  }

  //Serial.print(az_degrees);
  //Serial.print("  ");
  //Serial.println(el_degrees);

  //Serial.print(az_position);
  //Serial.print("  ");
  //Serial.println(el_position);

  //delay(20); // Long enough to hit a zero cross

  //delay(500);
  //Serial.println("Hello World!");
}

