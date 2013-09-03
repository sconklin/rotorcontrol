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

#define serBufferMax 32

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
char SERbuffer[serBufferMax];

//static unsigned char az_moving;
//static unsigned char el_moving;
//static unsigned char az_dir;
//static unsigned char el_dir;

// Position variables
int az_position = 0;
int el_position = 0;
float az_degrees = 0.0;
float el_degrees = 0.0;
float az_degrees_per_count = 0.1;
float el_degrees_per_count = 0.1;

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
  // a buffer. When completed by a newline, carriage return, or space, process it.
  //
  if (inbyte == '\n' || inbyte == '\r' || inbyte == ' ') {
    SERbuffer[SERbfsz++] = 0;
    cmdsz = SERbfsz - 1;
    SERbfsz = 0;

    // commands are exactly two characters long
    if (cmdsz < 2) {
      Serial.println("short");
      return;
    }

    //if (strncmp(SERbuffer, "Calibrate", 9) == 0) {
    //  calibrate();
    //  return;
    //}
    // It's safe to assume that we have two ascii characters at the beginning of the buffer
    // because if we don't it won't match any commands anyway and we'll reject it.
    // so, convert to upper case
    SERbuffer[0] &= ~(0x20);
    SERbuffer[1] &= ~(0x20);

    // now process it
    if (strncmp(SERbuffer, "AZ", 2) == 0) {
      // AZ
      // If only two characters, just print current az
      // TODO AZ
      return;
    }
    else if (strncmp(SERbuffer, "EL", 2) == 0) {
      // EL
      // TODO EL
      return;
    }
    else if (strncmp(SERbuffer, "UP", 2) == 0) {
      // Uplink Freq (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "DN", 2) == 0) {
      // Downlink Freq (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "DM", 2) == 0) {
      // Downlink Mode (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "UM", 2) == 0) {
      // Uplink Mode (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "DR", 2) == 0) {
      // Downlink Radio (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "UR", 2) == 0) {
      // Uplink Radio (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "ML", 2) == 0) {
      // Move Left
      setAzLeft();
      startAz();
      return;
    }
    else if (strncmp(SERbuffer, "MR", 2) == 0) {
      // Move Right
      setAzRight();
      startAz();
      return;
    }
    else if (strncmp(SERbuffer, "MU", 2) == 0) {
      // Move Up
      setElUp();
      startEl();
      return;
    }
    else if (strncmp(SERbuffer, "MD", 2) == 0) {
      // Move Down
      setElDown();
      startEl();
      return;
    }
    else if (strncmp(SERbuffer, "SA", 2) == 0) {
      // Stop Azimuth Moving
      stopAz();
      return;
    }
    else if (strncmp(SERbuffer, "SE", 2) == 0) {
      // Stop Elevation Moving
      stopEl();
      return;
    }
    else if (strncmp(SERbuffer, "AO", 2) == 0) {
      // AOS (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "LO", 2) == 0) {
      // LOS (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "OP", 2) == 0) {
      // Set Output number (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "IP", 2) == 0) {
      // Read an input (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "AN", 2) == 0) {
      // Read Analog Input (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "ST", 2) == 0) {
      // Set Time (ignore)
      return;
    }
    else if (strncmp(SERbuffer, "VE", 2) == 0) {
      // Version
      Serial.println("VE001");
      return;
    }
  }
  else {
    // accumulate the available character
    if (SERbfsz < (serBufferMax-1))
      SERbuffer[SERbfsz++] = inbyte;
    else {
      // buffer overrun, just clear buffer and start over
      Serial.println("clear");
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

  // Check for serial data
  while(Serial.available()) {
    handleSerial(Serial.read());
  }

  Serial.print(az_degrees);
  Serial.print("  ");
  Serial.println(el_degrees);

  //Serial.print(az_position);
  //Serial.print("  ");
  //Serial.println(el_position);

  //delay(20); // Long enough to hit a zero cross

  //delay(500);
  //Serial.println("Hello World!");
}

