/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
   usbh.c

    This file includes functions of TI AM335X USB controller.
    
    2017-10-02	Hsien Wen Tsai
*/

#include <common.h>
#include <command.h>
#include <usb.h>
#include "../cmd_bios.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "bios.h"
#include "model.h"
#include "usbh.h"
#include "usb_signal_init.h"
/*
 *	USB Host test ;
*/
#define USB_MEM_TEST_SIZE	0x400000
#define USB_TEST_ADDR1		0x81000000
#define USB_TEST_ADDR2		0x82000000
#define USB_TEST_PATTERN1	0x01234567
#define USB_TEST_PATTERN2	0xAA5555FF
#define USB_TEST_BIN_NAME	"test.bin"

extern char usb_start_port;
extern char usb_stop_port;
extern int usb_init_state;


int do_usb_get_info(struct usb_device *dev)
{

	int ret = 0;
        
	ret = usb_signal_init(dev, USB_FLASH);

	if (ret) {
		printf("USB INIT FAIL\n");
		goto EXIT;
	}

	ret = run_command("usb dev 0", 0);
EXIT:
	return ret;

}


#ifdef  USBH_MP
void USB_HOST_MPTEST(UINT8 led)
{
	unsigned long i;
	int ret = 0;
	char s[64] = {0};	
	int usb_num = 0;

	printf("\r\n");
	printf("[0x%04x]USB Host Test...\r\n", USBH_MP);

	usb_start_port = 1;
	usb_stop_port = 2;

	usb_init_state = 1;

	run_command("usb reset", 0);

	ret = run_command("usb dev 0", 0);

	if (ret != DIAG_OK ) {
		printf("Failed to attach the usb device. \r\n");
		goto FAIL;
	}

	printf("*******USB Storage Write Data(0x01234567) Test*******\n");

	for(i = 0; i < USB_MEM_TEST_SIZE; i += 4)
		VPlong(USB_TEST_ADDR1 + i) = USB_TEST_PATTERN1;
	
	sprintf(s, "fatwrite usb %d:1 0x%x %s 0x%x", usb_num, USB_TEST_ADDR1, USB_TEST_BIN_NAME, USB_MEM_TEST_SIZE);

	run_command(s, 0);
	
	udelay(1000 * 1000);
	
	sprintf(s, "fatload usb %d:1 0x%x %s 0x%x", usb_num, USB_TEST_ADDR2, USB_TEST_BIN_NAME, USB_MEM_TEST_SIZE);
	run_command(s, 0);
	
	udelay(100 * 1000);

	for (i = 0; i < USB_MEM_TEST_SIZE; i += 4) {
		if (VPlong(USB_TEST_ADDR2 + i) != USB_TEST_PATTERN1) {
			printf("USB Storage Read/Write data(0x01234567) Test..Fail\n");
			printf("test.bin [addr1=0x%lx,val=0x%lx]\n",(USB_TEST_ADDR2 + i), VPlong(USB_TEST_ADDR2 + i));
			goto FAIL;
		}
	}

	printf("*******USB Storage Write Data(0xAA5555FF) Test*******\n");

	for(i = 0; i < USB_MEM_TEST_SIZE; i += 4)
		VPlong(USB_TEST_ADDR1+i) = USB_TEST_PATTERN2;

	sprintf(s, "fatwrite usb %d:1 0x%x %s 0x%x", usb_num, USB_TEST_ADDR1, USB_TEST_BIN_NAME, USB_MEM_TEST_SIZE);
	run_command(s, 0);
	
	udelay(1000*1000);
	
	sprintf(s, "fatload usb %d:1 0x%x %s 0x%x", usb_num, USB_TEST_ADDR2, USB_TEST_BIN_NAME, USB_MEM_TEST_SIZE);
	run_command(s, 0);
	
	udelay(100*1000);

	run_command("fatls usb 0:1", 0);

	for(i = 0; i < USB_MEM_TEST_SIZE; i += 4) {
		if (VPlong(USB_TEST_ADDR2 + i) != USB_TEST_PATTERN2) {
			printf("USB Storage Read/Write data(0xAA5555FF) Test..Fail\n");
			printf("test.bin [addr1 = 0x%lx, val = 0x%lx]\n",(USB_TEST_ADDR2 + i), VPlong(USB_TEST_ADDR2 + i));
			goto FAIL;
		}
	}

	printf("USB Storage Read/Write testing ** OK ** \r\n");
	return;
FAIL:

	printf("USB Storage Read/Write testing ** FAIL ** \r\n");
	halt(led, HALT_MODE1);
	return;

}
#endif  //USBH_MP
#if 0
//R02 - Start
#ifdef  USB_CELLULAR_MP
void USB_CELLULAR_MPTEST(UINT8 led)
{
	printf("\r\n");
	printf("[0x%04x]USB Cellular and SIM Card Test...\r\n",USB_CELLULAR_MP);

	if (diag_do_check_sim_card()) {
		printf("\rUSB Cellular and SIM Card Test ** FAIL ** \r\n");
		halt(led, HALT_MODE1);
	} else {
		printf("\rUSB Cellular and SIM Card Test ** OK ** \r\n");
	}
}
#endif  //USB_CELLULAR_MP
#endif
int do_usb_detect_test(board_infos *sys_info)
{
	struct usb_device *dev = NULL;
	unsigned long i;
	int ret = 0;

	ret = do_usb_get_info(dev);

	if (ret != DIAG_OK ) {
		printf("Failed to attach the usb device. \r\n");
		ret = -1;
        	goto EXIT;
	}

	printf("*******USB Storage Write Data(USB_TEST_PATTERN1) Test*******\n");

	for (i = 0; i < USB_MEM_TEST_SIZE; i += 4)
		VPlong(USB_TEST_ADDR1 + i) = USB_TEST_PATTERN1;	

	run_command("fatwrite usb 0:1 0x81000000 test.bin 0x400000", 0);
	udelay(1000*1000);                       
	run_command("fatload usb 0:1 0x82000000 test.bin 0x400000", 0);
	udelay(100*1000);                       
	
	for (i = 0; i < USB_MEM_TEST_SIZE; i+= 4){
		if (VPlong(USB_TEST_ADDR2 + i) != USB_TEST_PATTERN1) {
			printf("USB Storage Read/Write data(USB_TEST_PATTERN1) Test..Fail\n");
			printf("test.bin [addr1=0x%lx,val=0x%lx]\n",(USB_TEST_ADDR2 + i),VPlong(USB_TEST_ADDR2 + i));
			printf("USB Storage Read/Write testing ** FAIL ** \r\n"); //R03
			ret = -1;
        		goto EXIT;
		}
			
	}
	
	printf("*******USB Storage Write Data(USB_TEST2_PATTERN) Test*******\n");	
	for(i = 0; i < USB_MEM_TEST_SIZE; i += 4)
		VPlong(USB_TEST_ADDR1 + i) = USB_TEST_PATTERN2;

	run_command("fatwrite usb 0:1 0x81000000 test.bin 0x400000", 0);
	udelay(1000*1000);                       
	run_command("fatload usb 0:1 0x82000000 test.bin 0x400000", 0);
	udelay(100*1000);                       
	run_command("fatls usb 0:1", 0);
	
	for (i = 0; i < USB_MEM_TEST_SIZE; i += 4) {
		if (VPlong(USB_TEST_ADDR2 + i) != USB_TEST_PATTERN2) {
			printf("USB Storage Read/Write data(USB_TEST2_PATTERN) Test..Fail\n");
			printf("test.bin [addr1=0x%lx,val=0x%lx]\n",(USB_TEST_ADDR2 + i), VPlong(USB_TEST_ADDR2 + i));
			printf("USB Storage Read/Write testing ** FAIL ** \r\n"); //R03
			ret = -1;
        		goto EXIT;
		}
	}

EXIT:
	return ret;

}

