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


#define SRS_IDLE 0
#define SRS_PARTIAL_COMMAND1 1
#define SRS_PARTIAL_COMMAND2 2
#define SRS_NEED_HEADING 3
//#define SRS_HAVE_HEADING 4
#define SRS_NEED_TERMINATOR 5
#define SRS_UNSYNCED 6

//
// Serial functions
//
int init_serial(void);
int handle_serial_input(void);

#endif /* _RSERIAL_H_ */
