/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    uart.c

    This file includes functions of TI AM335X UART controller.
    The UART functions are also included in it.
    
    2017-10-02	HsienWen Tsai
*/
#include <stdlib.h>
#include <common.h>
#include <command.h>
#include <usb.h>
#include <asm/gpio.h>
#include <bios.h>
#include <model.h>
#include <serial.h>
#include "../cmd_bios.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "uart.h"
#include <i2c.h>
#include <pca953x.h>
#include "usb_signal_init.h"

DECLARE_GLOBAL_DATA_PTR;

extern char usb_start_port;
extern char usb_stop_port;
static struct usb_device dev;
#define buff_sz  150

int set_uart_rts (int port, int enable)
{
        int val = 0;
	int ret;

        usb_signal_init (&dev, USB_UART);

	if (enable)
		val = FTDI_SIO_SET_RTS_HIGH;
	else
		val = FTDI_SIO_SET_RTS_LOW;

        ret = usb_control_msg (&dev, 
                         usb_sndctrlpipe(&dev, 0),
                         FTDI_SIO_SET_MODEM_CTRL_REQUEST,
                         FTDI_SIO_SET_MODEM_CTRL_REQUEST_TYPE,
                         val, port,
                         NULL, 0, USB_CNTL_TIMEOUT);


        if (ret)
                printf("failed\n");
	return ret;
}

void enable_uart_rts (int port)
{
	set_uart_rts(port, 1);
	return;
}

void disable_uart_rts (int port)
{
	set_uart_rts(port, 0);
	return;
}

int get_uart_cts (int port)
{
        unsigned char buf[2];
        int ret ;

        usb_signal_init(&dev, USB_UART);

        ret = usb_control_msg(&dev,
                usb_rcvctrlpipe(&dev, 0),
                FTDI_SIO_GET_MODEM_STATUS_REQUEST,
                FTDI_SIO_GET_MODEM_STATUS_REQUEST_TYPE,
                0, port,
                buf, 2, USB_CNTL_TIMEOUT);


        if (ret < 1) {
            printf("failed to get modem status\n"); 
            return -1;
        }

        //printf("port%d modem status : %02x %02x\n", port, buf[1], buf[0]);
        
	return (buf[0] & FTDI_SIO_CTS_MASK) ? 1 : 0 ;
}

static struct usb_uart_info uartbulk[10] = {
//      in_bulk     out_bulk
        {0x01,      0x02},
        {0x03,      0x04},
        {0x05,      0x06},
        {0x07,      0x08},
};

static int usb_uart_test (uint8_t port1, uint8_t port2, uint8_t time)
{
        int i;
	int alen;
//	int ret;
	int len;
        int s = 0;
        uint8_t tmp = 0;
        int delay = 10 ;
	char reply[buff_sz];
	char cmd[buff_sz];

	//printf ("Port%d -> Port%d ... \n", port1 + 1, port2 + 1);

        //printf("time %d tmp = %d\n\n", time, tmp);
        while (tmp != time){
                if (tmp == 0)
                {
                        memset (reply, '\0', buff_sz);
                        usb_bulk_msg (&dev, usb_rcvbulkpipe (&dev, uartbulk[port2].in_bulk), reply, buff_sz, &alen, 5000);
                }

                tmp ++ ;

                len = 26;
                memset (cmd, '\0', buff_sz);

                for (i = 0; i < buff_sz; i++)
                        cmd[i] = (i % len) + 'a';  

                usb_bulk_msg (&dev, usb_sndbulkpipe (&dev, uartbulk[port1].out_bulk), cmd, buff_sz,&alen, 5000);
                udelay(delay * 1000);

                memset (reply, '\0', buff_sz);
                usb_bulk_msg (&dev, usb_rcvbulkpipe (&dev, uartbulk[port2].in_bulk), reply, buff_sz + 2, &alen, 5000);

                if (strncmp (&reply[2], cmd, buff_sz) != 0 ) {
                        //printf("xmit  : %s, len : %d\n", cmd, alen);
                        //printf("reply : %s, len : %d\n\n", &reply[2], alen - 2);
                        s++;
                }

                len = 10;

                
                memset (cmd, '\0', buff_sz);

                for ( i = 0; i < buff_sz; i++ ){
                        cmd[i] = (i % len) + '0';  
                }

                usb_bulk_msg (&dev, usb_sndbulkpipe (&dev, uartbulk[port1].out_bulk), cmd, buff_sz,&alen, 5000);
                
                udelay(delay * 1000);

                memset (reply, '\0', buff_sz);

                usb_bulk_msg (&dev, usb_rcvbulkpipe (&dev, uartbulk[port2].in_bulk), reply, buff_sz + 2, &alen, 5000);

                if (strncmp (&reply[2], cmd, buff_sz) != 0 ) {
                        //printf("xmit  : %s, len : %d\n", cmd, alen);
                        //printf("reply : %s, len : %d\n\n", &reply[2], alen - 2);
                        s++;
                }
        }
        if (s)
                printf("port%d -> port%d failed\n", port1+1, port2+1);
        return s > 0 ? 1 : 0;
}

int diag_do_uart_232_RTS_CTS_test (void)
{
	int val = 1;
        int i ;
		
	        printf("start UART RTS CTS test ....\n");
		udelay(10*1000);

                for (i = UART1 ; i < UART1 +UART_NUM;i++)
                        enable_uart_rts(i);

                for (i = UART1 ; i < UART1 +UART_NUM;i++)
		        val &= get_uart_cts (i);

                if (val != 1) {
                        printf("RTS<-> CTS ..Test Fail!\n");
                        return 1;
                }
                
                val = 0;
                for (i = UART1 ; i < UART1 + UART_NUM; i++)
		        disable_uart_rts (i);

                for (i = UART1 ,val = 0; i < UART1 +UART_NUM;i++)
		        val |= get_uart_cts (i);


		if (val != 0) {
			printf("RTS<-> CTS ..Test Fail!\n");
			return 1;
                } else {
			printf("RTS<-> CTS ..Test OK!\n");
		}


	return 0;
}

void uart_mode_default(void)
{
	i2c_set_bus_num(2);
        pca953x_set_dir (PCA953X_ADDR1, 0xFFFF, 0);//set direction to output
	pca953x_set_val (PCA953X_ADDR1, 0xFFFF, 0x2222);//Default RS-232 Mode
	i2c_set_bus_num(0);

	udelay(100*1000);

	return;
}

/*
 *	UART test ;
*/

void uart_test_set_mode (int port, int mode)
{
        int shift = 0;

	i2c_set_bus_num(2);
	if (port == 0) {
            shift = PCA9535_UART_MODE_SHIFT_1;
	} else if (port == 1) {
            shift = PCA9535_UART_MODE_SHIFT_2;
	} else if (port == 2) {
            shift = PCA9535_UART_MODE_SHIFT_3;
	} else if (port == 3) {
            shift = PCA9535_UART_MODE_SHIFT_4;
	}

	if (mode == 0) /* RS-232 */
		pca953x_set_val (PCA953X_ADDR1, (0xFF << shift), (0x3 << shift));
	else if (mode == 1) /* RS-422 */
		pca953x_set_val (PCA953X_ADDR1, (0xFF << shift), (0x4 << shift));
	else if (mode == 2) /* RS-485-2W */
		pca953x_set_val (PCA953X_ADDR1, (0xFF << shift), (0x8 << shift));
        else
		pca953x_set_val (PCA953X_ADDR1, (0xFF << shift), (0x2 << shift));

	udelay (100);
	i2c_set_bus_num(0);
}

int do_usb_uart_test(uint8_t mode, uint8_t time)
{
        int ret = 0;
        //struct usb_device *udev = NULL ; 

        uart_mode_default();

        usb_signal_init(&dev, USB_UART);

	udelay(1000*1000);	//R24C
	uart_test_set_mode (0, mode);
	uart_test_set_mode (1, mode);
	uart_test_set_mode (2, mode);
	uart_test_set_mode (3, mode);

        ret = usb_uart_test (0, 1, time);
        ret |= usb_uart_test (1, 0, time) ;
        ret |= usb_uart_test (2, 3, time) ;
        ret |= usb_uart_test (3, 2, time) ;
    return ret;
}

int UART_MPTEST(u8 mode, u8 led)
{
	__maybe_unused int ret ;
//	int p = 0, q = 0, r = 0, s = 0;
        __maybe_unused int i, retry = 2;
	printf("\r\n");       	

        uart_mode_default();

	/* set UART  */
	switch (mode) {
#ifdef RS232_MP		
	case RS232_MP:
		printf("[0x%04x]RS-232 Test...\r\n",led);		
	        for (i = 0; i < retry; i++){
                        ret = do_usb_uart_test(0, 10);

                        if(i)
                                break;
                        if(ret){
                                printf("test failed!.");
                                printf("Please check uart RS-232 cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
                        }
                }
                        

		printf("[0x%04x]RS-232 CTS RTS Test...\r\n",led);		
		if(diag_do_uart_232_RTS_CTS_test()){
			halt(0, HALT_MODE1);
		}
		break;
#endif	//RS232_MP
			
#ifdef RS422_MP		
	case RS422_MP:		
		printf("Please switch to RS-422 cable.\n"); //R03
		printf("If you're ready. Please Press SW button To RS-422 Test.\n"); //R03
		
		Wait_SW_Button();

		printf("[0x%04x]RS-422 Test...\r\n",led);
	        for (i = 0; i < retry; i++){
                        ret = do_usb_uart_test(1, 10);

                        if(i)
                                break;
                        if(ret){
                                printf("test failed!.");
                                printf("Please check uart RS-422 cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
                        }
                }

		break;
#endif	//RS422_MP		
#ifdef RS4852W_MP		
	case RS4852W_MP:
		printf("Please switch to RS-485-2W cable.\n"); //R03
		printf("If you're ready. Please Press SW button To RS-485-2W Test.\n"); //R03
		
		Wait_SW_Button();

		printf("[0x%04x]RS-485-2W Test...\r\n",led);
	        for (i = 0; i < retry; i++){
                        ret = do_usb_uart_test(2, 10);

                        if(i)
                                break;
                        if(ret){
                                printf("test failed!.");
                                printf("Please check uart RS-485-2W cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
                        }
                }

		break;
#endif	//RS4852W_MP		
		default:
			return -1;
	}
        

	gd->baudrate = CONFIG_BAUDRATE;
	
        /*p = ret & 0x01;
        q = ret & 0x02;
        r = ret & 0x04;
        s = ret & 0x08;*/

	switch(mode){
#ifdef RS232_MP		
	case RS232_MP:
		//if ((p == 0) && (q == 0) && (r == 0) && (s == 0)) {
		if (ret == 0){ 
			printf("RS-232 Test...OK\r\n");

		} else {
		/*	if (p > 0) {
				//if(gpio_get_value(PIO_UART_NUMBER))
					printf("RS-232 Test(P1->P2)...FAIL\r\n");    
				//else
				//	printf("RS-232 Test P1...FAIL\r\n");

			} 
			if(q > 0) {
				printf("RS-232 Test(P2->P1)...FAIL\r\n");			    	
			} 
			if(r > 0) {
				printf("RS-232 Test(P3->P4)...FAIL\r\n");			    	
			} 
			if(s > 0) {
				printf("RS-232 Test(P4->P3)...FAIL\r\n");			    	
			}*/
					
			halt(led, HALT_MODE1);
		}
				
		break;
#endif	//RS232_MP				
#ifdef RS422_MP		
	case RS422_MP:
		if (ret == 0){ 
			printf("RS-422 Test...OK\r\n");
		} else {
			halt(led, HALT_MODE1);
		}
				
		break;
#endif	//RS422_MP		
#ifdef RS4852W_MP		
	case RS4852W_MP:
		if (ret == 0){ 
			printf("RS-485-2W...OK\r\n");
		} else {
			halt(led, HALT_MODE1);
		}

		break;
#endif	//RS4852W_MP		
	default:
		break;
	}
	
	return 0;

}

