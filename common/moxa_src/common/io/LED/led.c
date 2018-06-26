/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    led.c

    This file includes functions of TI AM335X GPIO controller.
    The GPIO functions are also included in it.
    
    2014-02-18	Sun Lee
*/
//R00 2017-10-20 Kennith Hsu.  modified for UC5100
#include <common.h>
#include <command.h>
#include <asm/gpio.h>
#include <bios.h>
#include <model.h>
#include "moxa_lib.h"
#include "types.h"
#include "led.h"
#include <i2c.h>
#include <pca953x.h>
#include "uart.h"


#define LED_MAX	12

int do_sd_led_func(unsigned int on)
{
	int ret = 0;

	ret = gpio_direction_output(PIO_LED_SD, on);
	
	return ret;

}

int do_zig_led_func(unsigned int on)
{
	int ret = 0;

	ret = gpio_direction_output(PIO_LED_ZIGBEE, on);

	return ret;
}

int do_usb_plug_led_func(unsigned int on)
{
	int ret = 0;

	ret = gpio_direction_output(PIO_LED_USB, on);

	return ret;
}

int do_cellular_led_func(unsigned int val)
{
	int led1 = gpio_get_value(PIO_LED_CEL_1);
	int led2 = gpio_get_value(PIO_LED_CEL_2);
	//int led3 = gpio_get_value(PIO_LED_CEL_3);
	int led = 0;
	int ret = 0;


	if ((val & 0x1) == 0x1)
		led = 1;
	else
		led = 0;

cel_led1:	
        ret |= gpio_direction_output(PIO_LED_CEL_1, led);

	if (ret != 0)
                goto EXIT;

	if ((val & 0x2) == 0x2)
		led = 1;
	else
		led = 0;
cel_led2:
        ret |= gpio_direction_output(PIO_LED_CEL_2, led);

	if (ret != 0){
	        led = led1;
		goto cel_led1;
	}

	if ((val & 0x4) == 0x4)
		led = 1;
	else
		led = 0;

        ret |= gpio_direction_output(PIO_LED_CEL_3, led);

	if (ret != 0){
	        led = led2;
		goto cel_led2;
	}

EXIT:
	return ret;

}

int do_diagnostic_led_func(unsigned int val)
{

	int led1 = gpio_get_value(PIO_LED_DIA_1);
	int led2 = gpio_get_value(PIO_LED_DIA_2);
	//int led3 = gpio_get_value(PIO_LED_DIA_3);
	int led = 0;
	int ret = 0;


	if ((val & 0x1) == 0x1)
		led = 1;
	else
		led = 0;

dia_led1:	
        ret |= gpio_direction_output(PIO_LED_DIA_1, led);

	if (ret != 0)
                goto EXIT;

	if ((val & 0x2) == 0x2)
		led = 1;
	else
		led = 0;
dia_led2:
        ret |= gpio_direction_output(PIO_LED_DIA_2, led);

	if (ret != 0){
	        led = led1;
		goto dia_led1;
	}

	if ((val & 0x4) == 0x4)
		led = 1;
	else
		led = 0;

        ret |= gpio_direction_output(PIO_LED_DIA_3, led);

	if (ret != 0){
	        led = led2;
		goto dia_led2;
	}

EXIT:
	return ret;

}

int do_user_led_func(unsigned int val)
{
	int ret = 0;
	int led = 0;

	ret = i2c_set_bus_num(2);

	ret |= pca953x_set_dir(PCA953X_ADDR2, 0x0700, ~0x0700); // R00 set out put for uer led port 

	if (ret != 0)
		goto EXIT;

	if ((val & 0x1) == 0x1)
		led = PIO_LED_USR_1;
	else
		led = 0;
	
	ret = pca953x_set_val(PCA953X_ADDR2, PIO_LED_USR_1 , led);

	if (ret != 0)
		goto EXIT;

	if ((val & 0x2) == 0x2)
		led = PIO_LED_USR_2;
	else
		led = 0;

	ret = pca953x_set_val(PCA953X_ADDR2, PIO_LED_USR_2 , led);
	
	if (ret != 0)
		goto EXIT;

        if ((val & 0x4) == 0x4)
                led = PIO_LED_USR_3;
	else
		led = 0;

	ret = pca953x_set_val(PCA953X_ADDR2, PIO_LED_USR_3 , led);

	if (ret != 0)
		goto EXIT;
	
	ret = i2c_set_bus_num(0);

EXIT:
	return ret;
}

int do_debug_led_func(unsigned int val)
{

	int led1 = gpio_get_value(PIO_LED_MP_1);
	int led2 = gpio_get_value(PIO_LED_MP_2);
	int led3 = gpio_get_value(PIO_LED_MP_3);
	int led4 = gpio_get_value(PIO_LED_MP_4);
	int led = 0;
	int ret = 0;


	if ((val & 0x1) == 0x1)
		led = 1;
	else
		led = 0;

dbg_led1:	
        ret |= gpio_direction_output(PIO_LED_MP_0, led);

	if (ret != 0)
                goto EXIT;

	if ((val & 0x2) == 0x2)
		led = 1;
	else
		led = 0;
dbg_led2:
        ret |= gpio_direction_output(PIO_LED_MP_1, led);

	if (ret != 0){
	        led = led1;
		goto dbg_led1;
	}

	if ((val & 0x4) == 0x4)
		led = 1;
	else
		led = 0;
dbg_led3:
        ret |= gpio_direction_output(PIO_LED_MP_2, led);

	if (ret != 0){
	        led = led2;
		goto dbg_led2;
	}

	if ((val & 0x8) == 0x8)
		led = 1;
	else
		led = 0;
dbg_led4:
        ret |= gpio_direction_output(PIO_LED_MP_3, led);

	if (ret != 0){
	        led = led3;
		goto dbg_led3;
	}

	if ((val & 0x10) == 0x10)
		led = 1;
	else
		led = 0;

        ret |= gpio_direction_output(PIO_LED_MP_4, led);

	if (ret != 0){
	        led = led4;
		goto dbg_led4;
	}

EXIT:
	return ret;

}


/*
 *	LED test ;
*/
int LED_MPTEST(u8 led)
{
	int i = 0;
	int count = 500;
	int ret = 0;

        printf("\r\n");

	printf("[0x%04x]LED Test...\r\n",led);		
	printf("wait sw button to pass test...\r\n");		
	do_sd_led_func(0);
	do_zig_led_func(0);
	do_usb_plug_led_func(0);
	do_cellular_led_func(0);
	do_diagnostic_led_func(0);
	//do_user_led_func(0);

	while(1){
		for( i = 0; i < LED_MAX; i++){
			switch(i){
				case LED_SD :
				        do_sd_led_func(1);
					udelay(count*1000);
				        do_sd_led_func(0);
					udelay(count*1000);
					break;
				case LED_ZIG:
				        do_zig_led_func(1);
					udelay(count*1000);
				        do_zig_led_func(0);
					udelay(count*1000);
					break;
				case LED_CEL_1:
					do_cellular_led_func(1);
					udelay(count*1000);
					do_cellular_led_func(0);
					udelay(count*1000);
					break;
				case LED_CEL_2:
					do_cellular_led_func(2);
					udelay(count*1000);
					do_cellular_led_func(0);
					udelay(count*1000);
					break;
				case LED_CEL_3:
					do_cellular_led_func(4);
					udelay(count*1000);
					do_cellular_led_func(0);
					udelay(count*1000);
					break;
				case LED_USB:
				        do_usb_plug_led_func(1);
					udelay(count*1000);
				        do_usb_plug_led_func(0);
                                        udelay(count*1000);
					break;
				case LED_DIA_1:
				        do_diagnostic_led_func(1);
                                        udelay(count*1000);
				        do_diagnostic_led_func(0);
                                        udelay(count*1000);
					break;
				case LED_DIA_2:
				        do_diagnostic_led_func(2);
                                        udelay(count*1000);
				        do_diagnostic_led_func(0);
                                        udelay(count*1000);
					break;
				case LED_DIA_3:
				        do_diagnostic_led_func(4);
                                        udelay(count*1000);
				        do_diagnostic_led_func(0);
                                        udelay(count*1000);
					break;
				default:
					break;
			}
		}

		if (GetJpStatus(PIO_SW_BUTTON) == 0) {
			while(1){
				if (GetJpStatus(PIO_SW_BUTTON) == 1) {
					udelay(1000);
					break;
				}
			}

			udelay(1000);
			break;
		}
	}

	return ret;
}

