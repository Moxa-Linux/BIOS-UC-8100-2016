/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    led.h

    Definitions of TI AM335X UART controller.
    
    2014-02-18	Sun Lee	
*/

#ifndef _LED_H
#define _LED_H

#define CELLULAR_LED		1
#define DIAGNOSTIC_LED		1

enum LED_INFO{
        LED_SD = 0,
        LED_ZIG,
        LED_CEL_1,
        LED_CEL_2,
        LED_CEL_3,
        LED_USB,
        LED_DIA_1,
        LED_DIA_2,
        LED_DIA_3,

};

int LED_MPTEST(u8 led);
int do_user_led_func(unsigned int on);
int do_debug_led_func(unsigned int val);
int do_cellular_led_func(unsigned int val);
int do_diagnostic_led_func(unsigned int val);
int do_zig_led_func(unsigned int on);
int do_usb_plug_led_func(unsigned int on);
int do_sd_led_func(unsigned int on);
#endif
