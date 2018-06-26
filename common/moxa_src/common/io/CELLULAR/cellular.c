/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <usb.h>
#include <asm/gpio.h>
#include <environment.h>
#include <cli.h>
#include <bios.h> 
#include <model.h>
#include <rf.h>
#include <serial.h>
#include "types.h"
#include "cmd_bios.h"         
#include "moxa_console.h"
#include "sys_info.h"
#include "usb_signal_init.h"
#include "moxa_lib.h"
#include "moxa_gpio.h"

#if 0
struct _rf_info rf_info[32]={
// vendor,   product,   at_out_bulk, at_in_bulk, gps_out_bulk, gps_in_bulk,   product_Name
  {0x1199,    0x9011,         0x3,       0x5,        0x1,           0x1,        "MC8305"},
  {0x1199,    0x68C0,         0x4,       0x5,        0x1,           0x1,        "MC8090"},
  {0x1199,    0x9011,         0x3,       0x4,        0x2,           0x2,        "MC9090"},
  {0x1199,    0x68C0,         0x3,       0x4,        0x2,           0x2,        "MC7304"},
  {0x1199,    0x68C0,         0x3,       0x4,        0x2,           0x2,        "MC7354"},
  {0x1546,    0x1141,         0x2,       0x3,        0x5,           0x7,        "MODEM-LTE"},
  {0x0bc3,    0x0001,         0x1,       0x1,        0x1,           0x1,        "IPWireless Modem"},
  {0x0,       0x0,            0x0,       0x0,        0x0,           0x0,        "Unknown"}
};
#endif

static int cell_init = 0;

static int do_check_sim_card(struct usb_device *dev, struct _rf_info *rf_info)
{
	int alen;
	int res = 0;
	int i = 0;
	int timeout;
	int num = 0;
	int ok_flag = 0;
	char model[MAX_SIZE_64BYTE] = {0};
	char msg[MAX_SIZE_64BYTE] = {0};

	#define USB_BUFSIZ	512
	ALLOC_CACHE_ALIGN_BUFFER(char, reply, USB_BUFSIZ);
	ALLOC_CACHE_ALIGN_BUFFER(char, cmd, USB_BUFSIZ);
	char ATcmd[]={"+CPIN: READY"};
	//char ATcmd[]={"+CPIN: SIM PIN"};
	
	printf("\n");
	sprintf(model, "%s", dev->prod);
	printf("MODEL:%s\n",dev->prod);

	while(1) {
		if (rf_info[num].vendor == 0)
			return DIAG_FAIL;
		
		sprintf(msg, "%s", rf_info[num].modelName);
		res = 0;

		for (i = 0;i < strlen((char *)model);i++) {
			if (model[i]!= msg[i]) {
				res = 1;
				num++;					
				break;
			}
		}
			
		if(res == 0)
			break;
	}
		
	if(cell_init == 0) {
		if ((num == 0) || (num == 2)) {
			res =  3;
		} else {
			if (dev->descriptor.idVendor == 0x1546)
				res = 6;
			else
				res = 1;
		}
	
		memset (cmd, 0, USB_BUFSIZ);
		sprintf(cmd, "ATE0\r\n");
		usb_bulk_msg (dev, usb_sndbulkpipe (dev, rf_info[num].out_bulk), cmd, strlen ((char *)cmd), &alen, 5000);
		printf ("\n");
		udelay(1000*1000);
			
		for(i = 0; i < res; i++) {
			memset(reply, 0, USB_BUFSIZ);
			usb_bulk_msg (dev, usb_rcvbulkpipe (dev, rf_info[num].in_bulk), reply, USB_BUFSIZ, &alen, 5000);
			printf("%s(RX Count=%d)(RX Len=%d)\n", reply,i,alen);
		}

		cell_init = 1;
	}
		
	printf ("CMD: \n");

	memset (cmd, 0, USB_BUFSIZ);
	sprintf(cmd, "AT+CPIN?\r\n");
	usb_bulk_msg (dev, usb_sndbulkpipe (dev, rf_info[num].out_bulk), cmd, strlen ((char *)cmd), &alen, 5000);
	printf ("\n");
		
	if(dev->descriptor.idVendor == 0x1546)
		timeout = 2;
	else
		timeout = 1;

	while (timeout--) {
		memset (reply, 0, USB_BUFSIZ);
		udelay(1000*1000);
		usb_bulk_msg (dev, usb_rcvbulkpipe (dev, rf_info[num].in_bulk), reply, USB_BUFSIZ, &alen, 5000);

		sprintf(cmd, "%s",reply);
	
		printf ("%s(RX Count=%d)(RX Len=%d)\n", reply, timeout, alen);
			
		if (ok_flag == 0) {
			for(res = 0; res < alen; res++) {
				if (ATcmd[i] == cmd[res])
					i++;
				else
					i = 0;				
				
				if (i == strlen((char *)ATcmd))
					ok_flag = 1;
			}					
		}
	}
		
	if (ok_flag) {
		printf("SIM Card Test OK!\n");
		return DIAG_OK;
	} else {
		printf("SIM Card Test Fail!\n");
	}

	return DIAG_FAIL;			
}

static int do_cellular_for_rf_test(struct usb_device *dev, struct _rf_info *rf_info, int mode)
{
	int alen;
	int res = 0;
	int i = 0;
	int num = 0;
	int count = 0;
	uchar RfOk[] = "RF_OK";
	char model [MAX_SIZE_64BYTE] = {0};
	char msg [MAX_SIZE_64BYTE] = {0};

	#define USB_BUFSIZ	512
	ALLOC_CACHE_ALIGN_BUFFER(char, reply, USB_BUFSIZ);
	ALLOC_CACHE_ALIGN_BUFFER(char, cmd, USB_BUFSIZ);
	
	sprintf(model, "%s", dev->prod);
	printf("MODEL:%s\n",model);

	while(1){
		if (rf_info[num].vendor == 0)
			return DIAG_FAIL;

		sprintf(msg, "%s", rf_info[num].modelName);
		res = 0;
		
		for (i = 0; i < strlen((char *)model); i++) {
			if(model[i]!= msg[i]){
				res = 1;
				num++;					
				break;
			}
		}

		if(res == 0)
			break;
	}
	
	if(cell_init == 0) {
		if((num == 0) || (num == 2)) {
			res =  2;
		} else {
			if(dev->descriptor.idVendor == 0x1546)
				res = 6;
			else
				res = 1;
		}
	
		memset (cmd, 0, USB_BUFSIZ);
		sprintf(cmd, "ATE0\r\n");
		usb_bulk_msg (dev, usb_sndbulkpipe (dev, rf_info[num].out_bulk), cmd, strlen ((char *)cmd), &alen, 5000);
		
		udelay(1000*1000);
		
		for(i = 0;i < res; i++){
			memset (reply, 0, USB_BUFSIZ);
			usb_bulk_msg (dev, usb_rcvbulkpipe (dev, rf_info[num].in_bulk), reply, USB_BUFSIZ, &alen, 5000);
			printf ("%s(RX Count=%d)(RX Len=%d)\n", reply,i,alen);
		}	
		
		cell_init = 1;

	}

	printf ("CMD: \n");

	while(1){
		res = 0;
		memset (cmd, 0, USB_BUFSIZ);
		
		if (mode == 0){	//Debug port
			res = con_get_string(cmd, (USB_BUFSIZ-1));
		
			if ((res) == -1)
				return DIAG_FAIL;

			udelay(1*1000);	
				
			if (cmd[res] == 27) 		// ESC key
				return DIAG_FAIL;

			cmd[res] = 0x0d;	// "\r"
			res++;
			cmd[res] = 0x0a;	// "\n"
				
		}else{
			while (1) {
                                //eserial4_device.start ();
                                //eserial4_device.setbrg ();
                                if (eserial4_device.tstc() == 0) {
                                        continue;
                                }
                                cmd[res] = eserial4_device.getc();
                                eserial4_device.putc(cmd[res]);
                                /*if (tstc() == 0) {
                                        continue;
                                }
                                cmd[res] = getc();
                                putc (cmd[res]);*/

				if (cmd[res] == 0xa || cmd[res] == 0x0d)    //R24B "\n"
					break;

				if (cmd[res] == 27)		//R24B "\n"
					return -1;

				res++;
			}
		}
                if (mode == 0)
		        printf ("\n");

		usb_bulk_msg (dev, usb_sndbulkpipe (dev, rf_info[num].out_bulk), cmd, strlen ((char *)cmd), &alen, 5000);

		if (dev->descriptor.idVendor == 0x1546)
			count = 2;
		else
			count = 1;

		while (count--) {
			memset (reply, 0, USB_BUFSIZ);
			udelay(1000*1000);
			usb_bulk_msg (dev, usb_rcvbulkpipe (dev, rf_info[num].in_bulk), reply, USB_BUFSIZ, &alen, 5000);
			
			if ((strncmp(cmd,"ATE0", 4) == 0) || strncmp(cmd, "ATE1", 4) == 0)
				count = 0;

			if ((strncmp(cmd,"AT+UBANDSEL=", 12) == 0)) {
				if(cmd[12] != '?') {
					count = 0;
				}
			}
				
                        if (mode == 0)
			        printf ("%s(RX Count=%d)(RX Len=%d)\n", reply,count,alen);
                        else
			        //eserial4_device.puts(reply); //R65
			        puts(reply); //R65

			if ((strncmp(reply,"\r\nERROR", 7) == 0) || ((strncmp(reply,"\r\nOK", 4) == 0)) \
							|| ((strncmp(reply,"OK", 2) == 0)) ){
					count = 0;
			}

			for (res = 0, i = 0; res < alen; res++) {

				if (RfOk[i] == cmd[res])
					i++;
				else
					i = 0;

				if (i == strlen((char *)RfOk))
					return DIAG_OK;
			}
		}
	}		
			
	
	return DIAG_FAIL;
}


int diag_do_cellular_for_rf_test(board_infos *sys_info, struct _rf_info *rf_info, int port)
{
	struct usb_device *dev = NULL;
	int ret = 0;

        dev = malloc( sizeof(struct usb_device) );

 //       do_pca9535_set_direction(2, 0x21, 1, 0);    //set power   direction as output
 //       do_pca9535_set_value(2, 0x21, 1, 1);
	ret = usb_signal_init(dev, USB_CELLULAR);

	if (ret) {
		printf("USB INIT FAIL\n");
		goto EXIT;
	}
	
//	dev = usb_get_dev_index(0);
	
//	if (dev == NULL) {
//		printf("Initial Fail\n");
//		goto EXIT;
//	}

	do_cellular_for_rf_test(dev, rf_info, port);
//        do_pca9535_set_value(2, 0x21, 1, 0);

EXIT:
	return ret;
}


int diag_do_check_sim_card(board_infos *sys_info, struct _rf_info *rf_info, int sim_side)
{
	struct usb_device *dev = NULL;
	int ret = 0;

        dev = malloc( sizeof(struct usb_device) );
	
//        do_pca9535_set_direction(2, 0x21, 1, 0);    //set power   direction as output
//        do_pca9535_set_direction(2, 0x21, 0, 0);    //set sim-pin direction as output
//
//        do_pca9535_set_value(2, 0x21, 0, 1 - sim_side); //set sim-pin direction
//        do_pca9535_set_value(2, 0x21, 1, 1);        //open the power
        ret = usb_signal_init(dev, USB_CELLULAR);

	if (ret) {
		printf("USB INIT FAIL\n");
		goto EXIT;
	}


	ret = do_check_sim_card(dev, rf_info);
//        do_pca9535_set_value(2, 0x21, 1, 0);    //close the power
EXIT:
	return ret;

}

#ifdef  USB_CELLULAR_MP

void USB_CELLULAR_MPTEST(board_infos *sys_info, struct _rf_info *rf_info, u8 led)
{
        unsigned int i;
        //char *s;
	printf("\r\n");
	printf("[0x%04x]USB Cellular and SIM Card Test...\r\n", USB_CELLULAR_MP);
        
        for (i = 0 ; i < SIM_NUM ;i++){
//                i == 0 ? (s = "A side") : (s = "B side");
//                printf ("test %s sim card\n", s);
//                printf ("insert sim card to %s and push the sw buttom\n", s);
//                Wait_SW_Button();

                if (diag_do_check_sim_card(sys_info, rf_info, i)) {
                        printf("\rUSB Cellular and SIM Card Test ** FAIL ** \r\n");
                        halt(led, HALT_MODE1);
                } else {
                        printf("\rUSB Cellular and SIM Card Test ** OK ** \r\n");
                }
        }
    //    sim_tester();
    //    run_command ("reset", 0);

}
#endif  //USB_CELLULAR_MP



