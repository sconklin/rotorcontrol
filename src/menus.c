/*
 
 Copyright 2010 Steve Conklin <steve@conklinhouse.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

*/

/*
 * Menus take all inputs from the front panel
 * Only one menu is active at a time
 * Each menu may contain multiple items
 * Each item is selected on a button press
 * Each item may call a function or start another menu
 */


struct menu {
    void *left_button_push();
    void *right_button_push();
    void *left_ccw();
    void *left_cw();
    void *right_ccw();
    void *right_cw();
};

