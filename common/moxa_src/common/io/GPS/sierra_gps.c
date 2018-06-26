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
#include "sys_info.h"
#include "usb_signal_init.h"

extern char usb_start_port;
extern char usb_stop_port;
extern int usb_init_state;

#if 0
struct _gps_info
{
	unsigned short          vendor;
	unsigned short          product;
	unsigned char           out_bulk;
	unsigned char           in_bulk;
	unsigned char           gps_out_bulk;
	unsigned char           gps_in_bulk;
	char                    modelName[32];
}__attribute__((packed));

struct _gps_info gps_info[32]={
  {0x1199,	0x9011,		0x3, 	     0x5,	    0x1,	    0x1,	"MC8305"},
  {0x1199, 	0x68C0,		0x4, 	     0x5,	    0x1,	    0x1,	"MC8090"},
  {0x1199, 	0x9011,		0x3, 	     0x4,	    0x2,	    0x2,	"MC9090"},
  {0x1199, 	0x68C0,		0x3,	     0x4,	    0x2,	    0x2,	"MC7304"},
  {0x1199, 	0x68C0, 	0x3,	     0x4,	    0x2,	    0x2,	"MC7354"},
  {0x1546, 	0x1141, 	0x2,	     0x3,	    0x5,	    0x7, 	"MODEM-LTE"},	
  {0x0bc3, 	0x0001, 	0x1,	     0x1,	    0x1,	    0x1, 	"IPWireless Modem"},
  {0x0, 	0x0, 		0x0, 	     0x0,	    0x0,	    0x0,	"Unknown"}	
// vendor,	product,   at_out_bulk,  at_in_bulk,  	gps_out_bulk,  	gps_in_bulk,   product_Name
};
#endif
static int check_gps_gpgga_info(struct gps_gpgga_info *gps_gpgga_infos, char gps_info[], int size)
{
	int i = 0;
	int head = 0;
	int tail = 0;	
	int dot_num = 0;
	char *ptr = gps_info;

	if ((gps_info[6] == ',') && (gps_info[7] == ','))
		return 0;

	for (i = 0; i < size; i++) {

		if (gps_info[i] == ',')
			dot_num++;

		if ((dot_num == 2) && (head == 0)){
			head = i+1;
			continue;
		}

		if ((dot_num == 3) && (tail == 0)) {
			tail = i;
			break;
		}
	}
	
	if ((tail - head) <= 1)
		return 0;
	
	ptr += head;

	for (i = 0; i < (tail - head); i++)
		gps_gpgga_infos->gps_lat[i] = ptr[i];

	if (strncmp(gps_gpgga_infos->gps_lat, "2503", 4) != 0)
		return 0;

	ptr = gps_info;
	dot_num = 0;
	head = 0;
	tail = 0;	

	for (i = 0; i < size; i++){

		if (gps_info[i] == ',')
			dot_num++;

		if ((dot_num == 4) && (head == 0)){
			head = i + 1;
			continue;
		}

		if ((dot_num == 5) && (tail == 0)) {
			tail = i;
			break;
		}
	}
	
	
	if ((tail - head) <= 1)
		return 0;
	
	ptr += head;

	for (i = 0; i < (tail - head); i++)
		gps_gpgga_infos->gps_lon[i] = ptr[i];

	if (strncmp(gps_gpgga_infos->gps_lon, "12127", 5) != 0)
		return 0;

	return 1;

}

int gps_mptest (struct usb_device *dev, struct _rf_info *rf_info)
{
	#define USB_BUFSIZ	512
	struct gps_gpgga_info gps_gpgga_infos;
	int alen;
	int i = 0;
	int num = 0;
	const char gps_title[] = "$GPGGA";
	int ok = 0;
	int res = 0;
	char msg[USB_BUFSIZ] = {0};
	char at_cmd[USB_BUFSIZ] = {0};
	char buf[64] = {0};
	char cmdmsg[64] = {0};
	ulong t_cnt = 0;
	ulong cnt = 0;

	memset(&gps_gpgga_infos, 0, sizeof(gps_gpgga_infos));

	sprintf(buf, "%s", dev->prod);
	
	printf("MODEL:%s\n",dev->prod);
		
	if (strcmp( dev->prod, "IPWireless Modem") == 0)
		return CELLULAR_PEM;

	if (strcmp( dev->prod, "MODEM-LTE") == 0){
		printf("This module does not support GPS features.\n");
		return CELLULAR_UBLOX;
	}

	while(1){
		if (rf_info[num].vendor == 0)
			return DIAG_FAIL;

		sprintf(cmdmsg, "%s", rf_info[num].modelName);
		res = 0;

		for(i = 0; i < strlen((char *)buf); i++){
			if(buf[i]!= cmdmsg[i]) {
				res = 1;
				num++;
				break;
			}
		}

		if(res == 0)
			break;
	}

	memset (at_cmd, 0, USB_BUFSIZ);

	sprintf(at_cmd,"$GPS_START");
	usb_bulk_msg (dev, usb_sndbulkpipe (dev, rf_info[num].gps_out_bulk), at_cmd, strlen ((char *)at_cmd), &alen, 5000);

	udelay(1000*1000);
			
	cnt = get_timer(0);

	for(i = 0; i < 10000; i++) {
		memset (msg, 0, USB_BUFSIZ);

		usb_bulk_msg (dev, usb_rcvbulkpipe (dev, rf_info[num].gps_in_bulk), msg, USB_BUFSIZ, &alen, 5000);

		if (strncmp(gps_title, msg, 6) == 0) {
			ok = check_gps_gpgga_info(&gps_gpgga_infos, msg, alen);

			if (ok){
				printf("GPS_TEST_OK\n");
				break;	
			}
		}
			
		t_cnt = get_timer(cnt);

		if(t_cnt >= 120000)
			break;

		mdelay(5);
	}

	if(ok)
		return DIAG_OK;
	else
		return DIAG_FAIL;
		
}

static int gps_test(struct usb_device *dev, struct _rf_info *rf_info)
{
	#define USB_BUFSIZ	512
	struct gps_gpgga_info gps_gpgga_infos;
	int alen;
	int i = 0;
	int num = 0;
	const char gps_title[] = "$GPGGA";
	int ok = 0;
	int res = 0;
	char msg[USB_BUFSIZ] = {0};
	char at_cmd[USB_BUFSIZ] = {0};
	char buf[64] = {0};
	char cmdmsg[64] = {0};
	ulong t_cnt = 0;
	ulong cnt = 0;

	memset(&gps_gpgga_infos, 0, sizeof(gps_gpgga_infos));

	sprintf(buf, "%s", dev->prod);
	
	printf("MODEL:%s\n",dev->prod);
		
	if (strcmp( dev->prod, "IPWireless Modem") == 0)
		return CELLULAR_PEM;

	if (strcmp( dev->prod, "MODEM-LTE") == 0){
		printf("This module does not support GPS features.\n");
		return CELLULAR_UBLOX;
	}

	while(1){
		if (rf_info[num].vendor == 0)
			return DIAG_FAIL;

		sprintf(cmdmsg, "%s", rf_info[num].modelName);
		res = 0;

		for(i = 0; i < strlen((char *)buf); i++){
			if(buf[i]!= cmdmsg[i]) {
				res = 1;
				num++;
				break;
			}
		}

		if(res == 0)
			break;
	}

	memset (at_cmd, 0, USB_BUFSIZ);

	sprintf(at_cmd,"$GPS_START");
	usb_bulk_msg (dev, usb_sndbulkpipe (dev, rf_info[num].gps_out_bulk), at_cmd, strlen ((char *)at_cmd), &alen, 5000);

	udelay(1000*1000);
			
	cnt = get_timer(0);

	for(i = 0; i < 10000; i++) {
		memset (msg, 0, USB_BUFSIZ);

		usb_bulk_msg (dev, usb_rcvbulkpipe (dev, rf_info[num].gps_in_bulk), msg, USB_BUFSIZ, &alen, 5000);

		if (strncmp(gps_title, msg, 6) == 0) {
			ok = check_gps_gpgga_info(&gps_gpgga_infos, msg, alen);

			if (ok){
				printf("GPS_TEST_OK\n");
				break;	
			}
		}
			
		t_cnt = get_timer(cnt);

		if(t_cnt >= 120000)
			break;

		mdelay(5);
	}

	if(ok)
		return DIAG_OK;
	else
		return DIAG_FAIL;
		
}

int do_gps_test(board_infos *sys_info, struct _rf_info *rf_info)
{
	struct usb_device *dev = NULL;
	int ret = 0;

	ret = usb_signal_init(dev, USB_CELLULAR);

	if (ret) {
		printf("USB INIT FAIL\n");
		goto EXIT;
	}

//	dev = usb_get_dev_index(0);
//
//	if (dev == NULL) {
//		printf("Initial Fail\n");
//		goto EXIT;
//	}

	ret = gps_test(dev, rf_info);
EXIT:
	return ret;

}

