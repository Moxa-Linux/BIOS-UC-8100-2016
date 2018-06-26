/*  Copyright (C) MOXA Inc. All rights reserved.

	This software is distributed under the terms of the
	MOXA License.  See the file COPYING-MOXA for details.
 */
/*
   test.c

   Test functions.

   2013-10-03	Donald Hsu.			Create it.
 */
#include <command.h>
#include <bios.h>
#include <model.h>
#include "../cmd_bios.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "test.h"
#include "wdt_diag.h"
#include <i2c.h>
#include <ns16550.h>
#include <serial.h>

#define CMD_232_SOH				0x01
#define CMD_232_EOT				0xFF
#define CMD_232_MAGIC				0x04
#define CMD_232_ACK				0x80
#define CMD_232_NAK				0x40

#define CMD_232_ALIVE				0x00
#define CMD_232_SET_SERIAL			0x01
#define CMD_232_SET_MAC				0x02
#define CMD_232_GET_SERIAL			0x03
#define CMD_232_GET_MAC				0x04
#define CMD_232_SET_CUSTOMER_SERIAL		0x05
#define CMD_232_GET_CUSTOMER_SERIAL		0x06
#define CMD_232_SET_FULL_SERIAL                 0x07
#define CMD_232_SET_MODEL_NAME                  0x08
#define CMD_232_GET_MODEL_NAME                  0x09
#define CMD_232_COPY				0x3D
#define CMD_232_SAVE				0x3E
#define CMD_232_RESTART				0x3F
#define MAC_GEN_MODE				0
#define CV_ID_MODE				1

DECLARE_GLOBAL_DATA_PTR;

struct frame232 {
	uchar	soh;
	uchar	magic;
	uchar	cmd;
	uchar	data[60];
	uchar	eot;
}__attribute__((packed));

typedef struct frame232 *frame232_t;

static struct frame232	Frame;
static struct _EEPROMFORMAT EEPROMFORMAT;


static int get_1_frame(void)
{
	uchar *ptr;
	int len = 0;
	frame232_t f;

	f = &Frame;
	ptr = (uchar *)&Frame;

	while(1){
		if(serial_tstc() == 0)
			continue;

		*ptr = serial_getc();
		
		if(*ptr == CMD_232_SOH)
			break;
	}

	ptr++;

	while(1){

		if((serial_tstc()) == 0) 
			continue;

		*ptr = serial_getc();

		if(*ptr != CMD_232_MAGIC)
			return 0;
		else
			break;

	}

	ptr++;

	while (1) {
		if(serial_tstc() == 0)
			continue;

		*ptr = serial_getc();
		len++;
		ptr++;

		if (len >= (int)(sizeof(Frame)-2))
			break;

	}

	if(f->eot != CMD_232_EOT)
		return 0;
	else
		return 1;

}

void GetSN_MAC(int mode)
{
	frame232_t f;
	PEEPROMFORMAT moxa_info;
	ulong val = 0;
	ulong serial;
	uchar m[18],mem[6];
	uchar mac[20], buf[256];
	uchar *ptr1;
	int i, k;
	f = &Frame;
	moxa_info = &EEPROMFORMAT;
	
	I2C_SET_BUS (0);

	if (mode == 1) {
		moxa_board_info(mode);
		printf("Waitting for serial and MAC address...\r\n");
	} else if(mode == 2) {
		printf("Waitting for Customer ID...\r\n");
	} else if(mode == 3) {
                printf("Waitting for Model Name...\r\n");
        }

	I2C_SET_BUS (0);
	while(1){
		if(get_1_frame() == 0)
			continue;
		
		switch(f->cmd){
			case CMD_232_ALIVE :
				f->cmd |= CMD_232_ACK;
				break;
			case CMD_232_SET_SERIAL :
				f->cmd |= CMD_232_NAK;
				break;
			case CMD_232_SET_MAC :
				k = 0;

				memset(m, 0, sizeof(m));

				for(i = 0; i < 6; i++){
					if(!_is_hex(f->data[i*2]) || !_is_hex(f->data[i*2+1])) {
						f->cmd |= CMD_232_NAK;
						break;
					}
					m[k] = f->data[(i*2)];
					k++;
					m[k] = f->data[(i*2)+1];
					k++;
					if(i < 5){
						m[k] = ':';
						k++;
					}

					mem[i] = (_from_hex(f->data[i*2]) * 16) + _from_hex(f->data[i*2+1]);
				}

				setenv("ethaddr", (char *)m);
                                val = ((mem[3] << 16) | (mem[4] << 8) | mem[5]) + 1;
                                mem[3] = (UINT8)((val >> 16) & 0xFF);
                                mem[4] = (UINT8)((val >> 8) & 0xFF);
                                mem[5] = (UINT8)(val & 0xFF);
				for (i = 0, k = 0; i < 6; i++) {                                        
					sprintf((char*)(&m[(k)]), "%02X", mem[i]);
					k = k + 2;
					if (i < 5) {
						m[k] = ':';
						k++;
					}
				}

				setenv("eth1addr", (char *)m);
				f->cmd |= CMD_232_ACK;				
				break;

			case CMD_232_GET_SERIAL :
				memset (buf, 0, sizeof (buf));
				ptr1 = (uchar *)getenv ("serialnumber");
				ptr1 += 8;

				for(i = 0; i < 4; i++) {
					buf[i] = *ptr1;
					ptr1++;
				}
				
				serial = (ulong)Atol((char*)buf);
				buf[0] = (u8) serial & 0xff ;
				buf[1] = (u8) (serial>>8) & 0xff;
				buf[2] = (u8) (serial>>16) & 0xff;
				buf[3] = (u8) (serial>>24) & 0xff;
				
				memcpy(f->data, buf, 4);
				f->cmd |= CMD_232_ACK;
				break;

			case CMD_232_GET_MAC :
				ptr1 = (uchar *)getenv ("ethaddr");
				for (i = 0, k = 0; i < 17; i++) {
					if (((i+1)%3) != 0) {
						mac[k] = *ptr1;
						k++;
					}
					ptr1++;
				}
				memcpy(f->data, mac, 12);

				f->cmd |= CMD_232_ACK;
				break;
			case CMD_232_GET_CUSTOMER_SERIAL :
				memset (buf, 0, sizeof (buf));
				ptr1 = (uchar *)getenv ("customer_id");

				for (i=0;i<12;i++) {
					buf[i] = *ptr1;
					ptr1++;
				}

				memcpy(f->data, buf, 12);
				f->cmd |= CMD_232_ACK;
			case CMD_232_SET_CUSTOMER_SERIAL :
				memcpy(moxa_info->szCusUID,f->data,strlen((char *)f->data));
				setenv ("customer_id",(char *)moxa_info->szCusUID);  // set customer id variable
				f->cmd |= CMD_232_ACK;
				break;
			case CMD_232_SET_FULL_SERIAL :
//		                diag_set_env_conf_u32("line", __LINE__);
				memcpy(moxa_info->szSerialNumber,f->data, strlen((char *)f->data));
				setenv ("serialnumber", (char *)moxa_info->szSerialNumber);
				f->cmd |= CMD_232_ACK;
//		                diag_set_env_conf_u32("line", __LINE__);
				break;
                        case CMD_232_SET_MODEL_NAME :
                                memcpy(moxa_info->szModelName, f->data,strlen((char *)f->data));
                                setenv ("modelname", (char *)moxa_info->szModelName);
                                f->cmd |= CMD_232_ACK;
                                break;
			case CMD_232_GET_MODEL_NAME :
				memset (buf, 0, sizeof (buf));
				ptr1 = (uchar *)getenv ("modelname");
				i = 0;
				
				while(1){
					buf[i] = *ptr1;
					if(*ptr1 == '\0'){
						break;
					}
					i++;
					ptr1++;
				}
                                
				memcpy(f->data, buf, strlen((char *)buf)+1);
				f->cmd |= CMD_232_ACK;
				break;
			case CMD_232_SAVE :
				f->cmd |= CMD_232_ACK;
				break;

			case CMD_232_COPY :
				f->cmd |= CMD_232_ACK;
				break;

			case CMD_232_RESTART :
				f->cmd |= CMD_232_ACK;
				break;

			default :
				f->cmd |= CMD_232_NAK;
				break;
		}

		k = sizeof (Frame);
		ptr1 = (uchar *)(&Frame);

		for (i = 0; i < k; i++) {
			NS16550_putc((NS16550_t)(CONFIG_SYS_NS16550_COM1), (char)(*ptr1));
			ptr1++;
		}


		if (f->cmd == (CMD_232_RESTART | CMD_232_ACK)) {

			val = diag_get_env_conf_u32("mm_flags");

			if (mode == 0) { 
				break;
			} else if (mode == 1) {
				ptr1 = (uchar *)getenv ("serialnumber");

				eeprom_write(EEPROM_I2C_ADDR, SN_BASE_ADDR, (unsigned char *)ptr1, MAX_SN_SIZE);
				if(strncmp((char *)ptr1, (char *)moxa_info->szSerialNumber, strlen((char *)ptr1)) != 0){
					printf("Check Serial Number Fail!\r\n");
					halt(0, HALT_MODE1);
				} else {
					printf("S/N:%s\r\n\r\n",moxa_info->szSerialNumber);
				}		
				diag_set_env_conf_u32("mm_flags", (val | T1_1_FLAG));

			} else if (mode == 2) {

				memset (buf, 0, sizeof (buf));
				
				if(eeprom_write(0x50, 114, (uchar *)moxa_info->szCusUID, 20) != 0) {
					printf("Write Customer ID to EEPROM Fail!\r\n");
					halt(0, HALT_MODE1);
				}

				eeprom_read(0x50, 114, buf, 20);

				if(strncmp((char *)buf,(char *)moxa_info->szCusUID, strlen((char *)buf)) != 0) {
					printf("Check Customer ID Fail!\r\n");
					halt(0, HALT_MODE1);
				} else {
					printf("Customer ID:%s\r\n\r\n",moxa_info->szCusUID);
				}

				diag_set_env_conf_u32("mm_flags", (val | T3_6_FLAG));
			} else if (mode == 3) {
				memset (buf, 0, sizeof (buf));
				if(eeprom_write(0x50,4,(unsigned char *)moxa_info->szModelName,80) != 0){
					printf("Write Model Name to EEPROM Fail!\r\n");
					halt(0, HALT_MODE1);
				}

				eeprom_read(0x50,4,buf,80);

				if(strncmp((char *)buf,(char *)moxa_info->szModelName,strlen((char *)buf)) != 0){
					printf("Check Model Name Fail!\r\n");
					halt(0, HALT_MODE1);
				} else {
					printf("MODEL NAME:%s\n",moxa_info->szModelName);
				}


				diag_set_env_conf_u32("mm_flags", (val | T3_2_FLAG));
			}
			halt (PIO_LED_SD, HALT_MODE3);
		}
	}
//	uart_test_set_mode (0, 0);		//Setting to RS-232 mode
//	uart_test_set_mode (1, 0);

	return;
}        

void moxa_board_info(int mode)
{
        PEEPROMFORMAT moxa_info;
        uchar *moxa_info_ptr;

        moxa_info = &EEPROMFORMAT;
        moxa_info_ptr = (uchar *)&EEPROMFORMAT;

        memset (moxa_info, 0, sizeof (moxa_info));
        *moxa_info_ptr = 0x55;
        moxa_info_ptr++;
        *moxa_info_ptr = 0xAA;
        moxa_info_ptr++;
        *moxa_info_ptr = VersionMajor;
        moxa_info_ptr++;
        *moxa_info_ptr = VersionMinor;
}

void BIOS_SET_TEST_FLAG(int mm_flag)
{
	ulong val = 0;

	printf("Set MP FLAG TEST...\r\n");

	val = diag_get_env_conf_u32("mm_flags"); 		
	diag_set_env_conf_u32("mm_flags", (val | mm_flag));

	printf("MP-Test has Completed!\r\n");

	return;
}

void BIOS_MPTEST_OK(void)
{

	ulong   val=0;

	ShowLED(0);

	printf("Set MP FLAG TEST...\r\n");
	val = diag_get_env_conf_u32("mm_flags");
	diag_set_env_conf_u32("mm_flags", (val | T1_0_FLAG));
	printf("MP-Test has Completed!\r\n");

	/* Completed Ready LED  */

	while(1){
		//ReadyLed(PIO_LED_READY,1);
		udelay (500000);
		//ReadyLed(PIO_LED_READY,0);
		udelay (500000);
	}

}


