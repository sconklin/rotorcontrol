/*
  Antenna Rotor Controller
  Error code

 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

#ifndef _ERROR_H_
#define _ERROR_H_


#define ERR_SERIAL_STATE 1
#define ERR_SOMETHING_ELSE 2

void rotor_error(int errnum);

#endif /* _ERROR_H_ */
