/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <usb.h>
#include <gps.h>
#include <asm/gpio.h>
#include <environment.h>
#include <bios.h> 
#include "cmd_bios.h"         
#include <model.h>
#include "types.h"
#include <cli.h>
#include <rf.h>

int PRE_USB_INIT = NONE;

extern char usb_start_port;
extern char usb_stop_port;
extern int usb_init_state;

int check_modules_init_time(const char model_name[]){
	
	int time = 0;

	if(strcmp(model_name, "MC8305") == 0)
		time = 5;
	else if (strcmp(model_name, "MC7304") == 0)
		time = 10;
	else if (strcmp(model_name, "MC7354") == 0)
		time = 10;
	else if (strcmp(model_name, "MC9090") == 0)
		time = 10;
	else if (strcmp(model_name, "MC8090") == 0)
		time = 10;
	else if (strcmp(model_name, "MODEM-LTE") == 0)
		time = 30;
	else
		time = 10;

	return time;

}

int usb_signal_init(struct usb_device *dev, int init_type)
{

	int cnt = 0;
	int time = 0;
	int ret = 0;	
        //int delay = 13;

	if(init_type == USB_CELLULAR) {
		usb_start_port = 0;
		usb_stop_port = 1;
	} else {
		usb_start_port = 1;
		usb_stop_port = 2;
	}

	if((init_type != PRE_USB_INIT) || (init_type == USB_CELLULAR) || (usb_get_dev_index(0) == NULL)) {
		
		if(init_type == USB_CELLULAR){
			/*for (cnt = 0; cnt < delay; cnt++){
			        printf("wait Cellular module initial. %02d s\r", delay - cnt - 1);
				udelay(1000*1000);
			}
			printf("\n");*/

//                        do{
                                run_command("usb reset", 0);
//                        } while (usb_get_dev_index(1) == NULL || (usb_stor_exist() == 0 && usb_get_dev_index(2) == NULL)) ;
		}else{
                        run_command("usb reset", 0);
                        if (init_type == USB_FLASH){
                                PRE_USB_INIT = init_type;
                                if (usb_stor_exist() == 0)
                                        return 0;
                                else
                                        return -1;
                        }
	        }

                if(init_type == USB_UART){
                        if (usb_get_dev_index(1)){
                                *dev = *usb_get_dev_index(1);
                        }else{
                                return -1;
                        }
                }else{
                        if (usb_get_dev_index(0)){
                                *dev = *usb_get_dev_index(0);
                        }else{
                                return -1;
                        }
                }


		if(init_type == USB_CELLULAR)
			printf("Waiting for Module Init...");

		if (init_type == USB_CELLULAR)
		        time = check_modules_init_time(dev->prod);	
		else
			time = 0;
		
		for (cnt = 0; cnt < time; cnt++)
			udelay(1000*1000);
	
		if (init_type == USB_CELLULAR)
			printf("Ready!\n");

		PRE_USB_INIT = init_type;
	}

        if(init_type == USB_UART){
                if (usb_get_dev_index(1)){
                        *dev = *usb_get_dev_index(1);
                }else{
                        return -1;
                }
        }else{
                if (usb_get_dev_index(0)){
                        *dev = *usb_get_dev_index(0);
                }else{
                        return -1;
                }
        }

	if(dev == NULL){
		ret = -1;
		goto EXIT;
	}
	
	if(dev->devnum == 0){
		run_command("usb start", 0);
		printf("Waiting for Module Init...");
		
		time = check_modules_init_time(dev->prod);
		
		for( cnt = 0; cnt < time; cnt++)
			udelay(1000*1000);
		
		printf("Ready!\n");
		PRE_USB_INIT = init_type;
	}

        if(init_type == USB_UART){
                *dev = *usb_get_dev_index(1);
        }else
                *dev = *usb_get_dev_index(0);


	if(dev == NULL) {
		ret = -1;
		printf("USB DEV GET FAIL\n");
	}
        
	printf("\n");

EXIT:
	return ret;
}
