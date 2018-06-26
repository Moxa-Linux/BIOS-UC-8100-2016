/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    uart.h

    Definitions of TI AM335X UART controller.
    
    2014-02-18	HsienWen Tsai
*/

#ifndef _UART_H
#define _UART_H

#define UART_CTS_REG		0x18
#define UART_RTS_REG		0x10

#define FTDI_SIO_RESET              0 /* Reset the port */
#define FTDI_SIO_MODEM_CTRLM_CTRL   1 /* Set the modem control register */
#define FTDI_SIO_SET_FLOW_CTRL      2 /* Set flow control register */
#define FTDI_SIO_SET_BAUD_RATE      3 /* Set baud rate */
#define FTDI_SIO_SET_DATA           4 /* Set the data characteristics of the port */
#define FTDI_SIO_GET_MODEM_STATUS   5 /* Retrieve current value of modemdem status register */

#define FTDI_SIO_SET_EVENT_CHAR     6 /* Set the event character */
#define FTDI_SIO_SET_ERROR_CHAR     7 /* Set the error character */
#define FTDI_SIO_SET_LATENCY_TIMER  9 /* Set the latency timer */
#define FTDI_SIO_GET_LATENCY_TIMER 10 /* Get the latency timer */

/* FTDI_SIO_MODEM_CTRLM_CTRL */
#define FTDI_SIO_SET_MODEM_CTRL_REQUEST_TYPE 0x40
#define FTDI_SIO_SET_MODEM_CTRL_REQUEST FTDI_SIO_MODEM_CTRLM_CTRL

#define FTDI_SIO_SET_RTS_MASK 0x2
#define FTDI_SIO_SET_RTS_HIGH ((FTDI_SIO_SET_RTS_MASK << 8) | 2)
#define FTDI_SIO_SET_RTS_LOW  ((FTDI_SIO_SET_RTS_MASK << 8) | 0)

/* FTDI_SIO_SET_FLOW_CTRL */
#define FTDI_SIO_SET_FLOW_CTRL_REQUEST_TYPE 0x40
#define FTDI_SIO_SET_FLOW_CTRL_REQUEST      FTDI_SIO_SET_FLOW_CTRL
#define FTDI_SIO_DISABLE_FLOW_CTRL          0x0
#define FTDI_SIO_RTS_CTS_HS                 (0x1 << 8)
#define FTDI_SIO_DTR_DSR_HS                 (0x2 << 8)
#define FTDI_SIO_XON_XOFF_HS                (0x4 << 8)

/* FTDI_SIO_GET_MODEM_STATUS */
#define FTDI_SIO_GET_MODEM_STATUS_REQUEST_TYPE  0xc0
#define FTDI_SIO_GET_MODEM_STATUS_REQUEST       FTDI_SIO_GET_MODEM_STATUS
#define FTDI_SIO_CTS_MASK                       0x10
#define FTDI_SIO_DSR_MASK                       0x20
#define FTDI_SIO_RI_MASK                        0x40
#define FTDI_SIO_RLSD_MASK                      0x80

#define WDR_TIMEOUT                         5000
#define UART_NUM                            4

struct _uart_info
{
	unsigned short          vendor;
	unsigned short          product;
	unsigned char           out_bulk;
	unsigned char           in_bulk;
	unsigned char           gps_out_bulk;
	unsigned char           gps_in_bulk;
	char                    modelName[32];
}__attribute__((packed));

struct usb_uart_info
{
        unsigned char           in_bulk;
        unsigned char           out_bulk;
}__attribute__((packed));

enum {
	RS232 = 0,
	RS422,
	RS4852W
};

enum {
	UART0 = 0,
	UART1,
	UART2,
	UART3,
	UART4,
	UART5,
	UART6,
};

int UART_MPTEST(u8 mode, u8 led);
void uart_test_set_mode (int uart_port, int mode);
int do_usb_uart_test(uint8_t mode, uint8_t time);
void uart_mode_default(void);
int set_uart_rts (int port, int enable);
int get_uart_cts (int port);
#endif	//_UART_H
