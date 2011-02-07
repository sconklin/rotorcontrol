/*
  Serial port handler code
 
 Copyright 2011 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef _RSERIAL_H_
#define _RSERIAL_H_

//
// Serial functions
//
int init_serial(void);
int handle_serial_input(void);

#endif /* _RSERIAL_H_ */
