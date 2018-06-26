/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <net.h>
#include <usb.h>
#include <gps.h>
#include <i2c.h>
#include <tpm.h>
#include <spi.h>
#include <rf.h>
#include <asm/gpio.h>
#include <environment.h>
#include <pca953x.h>
#include <model.h>
#include <cli.h>
#include "moxa_bios/moxa_console.h"
#include "moxa_bios/moxa_lib.h"
#include "moxa_bios/moxa_boot.h"
#include "moxa_bios/moxa_upgrade.h"
#include "moxa_bios/moxa_mem.h"
#include "moxa_bios/sys_info.h"
#include "moxa_bios/moxa_gpio.h"
#include "moxa_bios/moxa_rtc.h"
#include "moxa_bios/ethernet.h"
#include "moxa_bios/tpm_mp.h"
#include "moxa_bios/tpm2_mp.h"
#include "moxa_bios/usbh.h"
#include "moxa_bios/sd.h"
#include "moxa_bios/uart.h"
#include "moxa_bios/led.h"
#include "moxa_bios/test.h"
#include "moxa_bios/wdt_diag.h"
#include "moxa_bios/ds1374_wdt.h"
#include "moxa_bios/cellular.h"
#include "moxa_bios/gps.h"
#include "moxa_bios/cpu_mp.h"
#if CONFIG_MOXA_DIO
#include "moxa_bios/dio.h"
#endif
#include "cmd_bios.h"
#include "mmc.h"
#include "fs.h"
#include "fat.h"
#include "cmd_bios.h"
#include "predefine_table.h"
#include "types.h"
#include "cmd_tpm2.h"

DECLARE_GLOBAL_DATA_PTR;

static char ReturnUboot = 0;

extern int diag_do_check_gps(void);
char CharBuf[256];
char ShortBuf[64];
char CmdMsg[64];
const char RUNBAR[4][8] = {"\\\r","| \r","/ \r","- \r"};
const char AbortString[20] = {"Abort this test!\r\n"};
const char ESCString[48] = {"Please press 'ESC' to Quit this testing ...\r\n"};
static int WDT_MODE = 0;
extern char usb_start_port;
extern char usb_stop_port;
data_pattern DataPattern; // That pattern that we use in all MOXA BIOS
unsigned char test_pattern_byte[6] = {0x00, 0x55, 0x5A, 0xAA, 0xA5, 0xFF};
unsigned short test_pattern_short[6] = {0x0000, 0x5555, 0x5A5A, 0xAAAA, 0xA5A5, 0xFFFF};
unsigned long test_pattern_long[6] = {0x00000000, 0x55555555, 0x5A5A5A5A, 0xAAAAAAAA, 0xA5A5A5A5, 0xFFFFFFFF};
int usb_init_state = 0;
extern unsigned  int fw_tftp_size;
extern int usb_signal_init(struct usb_device *dev, struct _rf_info *rf_info, int init_state);
void diag_do_nothing(void){printf("\r\nNot Support!\r\n");return;}
board_infos sys_info;

#define BIOS_ITEM_FOR_BASIC_FUNC	1
#define BIOS_ITEM_FOR_FULL_FUNC		1
#define MMC_MAX_DEVICE			3

#if 1
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


burnin_cmd burnin_cmd_value[] = {	//\\ 31 characters in each item.
	{1, 1, "Set/Clear Flag", diag_do_set_clear_flag_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Download/Upload", diag_do_download_upload_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Set SN/MAC", diag_do_sn_mac_config_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Set Burn in Time", diag_do_nothing, BIOS_ITEM_FOR_BASIC_FUNC},
//	{1, 1, "Module/ID Setting", diag_do_nothing, BIOS_ITEM_FOR_BASIC_FUNC},	
	{1, 1, "TFTP Settings", diag_do_tftp_setting_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Console/UART Test", diag_do_uart_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Ethernet Test", diag_do_eth_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "IO Test", diag_do_io_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Memory/Storage Test", diag_do_memory_storage_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "TPM2 Setting", diag_do_tpm2_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "RTC Test", diag_do_rtc_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "USB Host Test", diag_do_usb_host_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "SD Test", diag_do_sd_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "LED Test", diag_do_led_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "WDT Test", diag_do_wdt_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Set/Clear backup env", diag_do_set_clear_backup_env_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "QA Test", diag_do_qa_test_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{1, 1, "Run Kernel", diag_do_run_kernel_func, BIOS_ITEM_FOR_BASIC_FUNC},
 	{99, 'q', "UBoot Command Line", diag_do_uboot, BIOS_ITEM_FOR_BASIC_FUNC},	
	{-1,'*',"",0,0}
};

void diag_do_show_ip(void);
void diag_do_change_ip(void);

burnin_cmd BIOS_config_cmd_value [] = {
	{1, 1, "TPM2 Setting", diag_do_tpm2_config, 1},
	{1, 1, "SD Card Write Protect", diag_do_sd_protected_config, 0},
	{1, 1, "Extend USB Port Control", diag_do_extend_usb_port_control, 0},
	{1, 1, "Update Firmware from Tftp", diag_do_tftp_download_firmware, 1},
	{1, 1, "Go To OS", diag_do_run_mmc_func, BIOS_ITEM_FOR_BASIC_FUNC},
	{-1, '*', "", 0, 0}
};

void diag_do_extend_usb_port_control(void)
{
	unsigned int sd_protected;
	unsigned int set_or_clean;

	printf("\nCurrent Extend USB Port is ");
	sd_protected = diag_get_env_conf_u32("sd_protected");                
	
	if(sd_protected & extend_USB_port_crtl)
		printf("OFF.\n");
	else
		printf("ON.\n");

	if(diag_get_value("Change to ,0 - ON, 1 - OFF", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	if(set_or_clean)
		sd_protected |= extend_USB_port_crtl;
	else
		sd_protected &= ~extend_USB_port_crtl;


	diag_set_env_conf_u32("sd_protected", sd_protected);
	
}

void bios_print_item_msg(burnin_cmd *cmd_list)
{

	int i = 0, j = 0, k = 0;
	char *s;

	burnin_cmd *burnin_temp = cmd_list;

	printf("\r\n----------------------------------------------------------------------------");
	
	s = getenv("modelname");
	printf("\r\n  Model: %s",s);

	printf("\r\n  Boot Loader Version: ");
	s = getenv ("biosver");
	printf("%s         ", s);

	Check_CPU_Type(1);
	
	printf ("\r\n  Build date: " __DATE__ " - " __TIME__);	

	s = getenv("serialnumber");
	printf("   Serial Number: %s", s);

	s = getenv("ethaddr");
	printf("\r\n  LAN1 MAC: %s", s);

	s = getenv ("eth1addr");

	printf("          LAN2 MAC: %s", s);
	printf("\r\n----------------------------------------------------------------------------\r\n");

	while (burnin_temp->index >= 0) {
		if (burnin_temp->exist) {
			if ((burnin_temp->index == 99) || (burnin_temp->index == 98)) {
				printf("(%c)",burnin_temp->symbol);
			} else {
				if (k == 26)
					k++;

				if (k <= 9) {
					printf ("(%c)",k+'0');
					burnin_temp->symbol = k+'0';
				} else {
					printf ("(%c)",k-10+'a');
					burnin_temp->symbol = k-10+'a';
				}
			}

			printf(" %s",burnin_temp->string);
			
			for(j = strlen((char *)burnin_temp->string); j < 31; j++){
				printf(" ");
			}

			k++;
			i++;

			if((i % 2) == 0) {
				printf("\r\n");
			}
		}

		burnin_temp++;
	}

	if((i % 2) != 0) {
		printf("\r\n");
	}
}

int bios_parse_cmd (char* buffer)
{
	char seps[]  = " ";
	char *token;
	char cmd_buf[256];
	unsigned int i;

	token = strtok (buffer, seps);
	
	if (token == NULL)
		return (-1);

	strcpy (cmd_buf, token);
	
	if ((strcmp (cmd_buf, "q") == 0) ||(strcmp (cmd_buf, "Q") == 0))
		return (-2);

	i = atoi_simple (cmd_buf);

	return i;
}

char bios_parse_cmd_char(char* buffer)
{
	char seps[]  = " ";
	char *token;
	char cmd_buf[256];

	token = strtok(buffer, seps);

	if (token == NULL)
		return (-1);

	strcpy(cmd_buf, token);
	
	if (cmd_buf[0] >= 'A' && cmd_buf[0] <= 'Z')
		cmd_buf[0] += (0x20);

	printf("\r\n");

	return cmd_buf[0];
}

/**	\brief
 *
 *	Get input value from user and convert to the specfic format.
	mode:
		 DIAG_GET_VALUE_HEX_MODE
		 DIAG_GET_VALUE_DEC_MODE
		 DIAG_GET_VALUE_NO_ABORT
 *
*/
int diag_get_value(char *title, unsigned int *val, unsigned int min, unsigned int max, int mode)
{
	while(1) {

		printf("\r\n%s (", title);

		if (mode == DIAG_GET_VALUE_HEX_MODE)
			printf("0x%lX-0x%lX", (unsigned long)min, (unsigned long)max);
		else
			printf("%lu-%lu", (unsigned long)min, (unsigned long)max);

		if (mode & DIAG_GET_VALUE_NO_ABORT)
			printf("): ");
		else
			printf(",enter for abort): ");

		if (con_get_string(CharBuf,sizeof(CharBuf)) == -1){
			printf(AbortString);
			return DIAG_ESC;
		}

		if (CharBuf[0] == 0)
			return DIAG_FAIL;

		if ((bios_parse_num(CharBuf, val, 0, 0) || *val < min || *val > max ) == DIAG_FAIL) {
			printf("\r\n%d: Invalid number !", *val);
			continue;
		}

		break;
	}

	printf("\r\n");
	
	return DIAG_OK;

}

/**	\brief
 *
 *	BIOS Init function. 
 *
 */
void bios_init_global_value(void)
{
	//extern rtc_time_block RtcTime;
	// 1. DataPattern
	DataPattern.pat0 = DATA_PATTERN0;
	DataPattern.pat1 = DATA_PATTERN1;
	DataPattern.pat2 = DATA_PATTERN2;
	DataPattern.pat3 = DATA_PATTERN3;
	DataPattern.pat4 = DATA_PATTERN4;
	DataPattern.pat5 = DATA_PATTERN5;
	/*
	// 2. RTC Time block
	RtcTime.t_year = IXP_RTC_BASE_YEAR;
	RtcTime.t_mon = 1;
	RtcTime.t_mday = 1;
	*/
	return;
}

void diag_do_uboot(void)
{
	printf("\r\n\r\n");
	ReturnUboot = 1;
}

/**	\brief
 *
 *	Show the BIOS menu.
 *
 */
int bios_manual_testing(burnin_cmd *cmd_list)
{
	unsigned long ret = 0;

	while(1){
		burnin_cmd *burnin_temp = cmd_list;
		bios_print_item_msg(burnin_temp);
		printf("---------------------------------------------------------------------------\r\n");
		printf("Command>>");

		if(con_get_string(CmdMsg,sizeof(CmdMsg)) == -1){
			printf("\r\n");
			break;
		}

		ret = bios_parse_cmd_char(CmdMsg);
	
		if (ret == (-1)) {
			continue;
		} else if((ret == 'q' || ret == 'Q') && burnin_temp != burnin_cmd_value) {
			printf("\r\n");
			break;
		}

		while (burnin_temp->index != (-1)) {
			if ((ret == burnin_temp->symbol ) && (burnin_temp->exist)) {
				(*burnin_temp->burnin_routine)();
				break;
			}

			burnin_temp++;
		}

		if (ReturnUboot)
			break;
	}
	return ret;
}

int bios_manual_config(burnin_cmd *cmd_list)
{
	unsigned long ret = 0;

	while(1) {
		burnin_cmd *burnin_temp = cmd_list;
		bios_print_item_msg(burnin_temp);
		printf("---------------------------------------------------------------------------\r\n");
		printf("Command>>");

		if (con_get_string(CmdMsg,sizeof(CmdMsg)) == -1) {
			printf("\r\n");
			break;
		}

		ret = bios_parse_cmd_char(CmdMsg);

		if (ret == (-1)) {
			continue;
		} else if ((ret == 'q' || ret == 'Q') && burnin_temp!=BIOS_config_cmd_value) {
			printf("\r\n");
			break;
		}

		while(burnin_temp->index != (-1)) {
			if ((ret == burnin_temp->symbol) && (burnin_temp->exist)) {
				(*burnin_temp->burnin_routine)();
				break;
			}

			burnin_temp++;
		}

		if (ReturnUboot)
			break;
	}

	return ret;
}

unsigned int diag_get_mm_flag(unsigned int flag)
{
	unsigned int mm_flags;
	int ret = 0;

	mm_flags = diag_get_env_conf_u32("mm_flags");                

	if (mm_flags & flag) {
		printf("set.\n");
		ret = 1;
	} else {
		printf("cleared.\n");
		ret = 0;
	}

	return ret;
}

void diag_set_mm_flag(unsigned long flag)
{
	unsigned int mm_flags;
	unsigned int set_or_clean;
	
	mm_flags = diag_get_env_conf_u32("mm_flags"); 		

	if (diag_get_value("0 - clean, 1 - set", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	if (set_or_clean)
		mm_flags |= flag;
	else
		mm_flags &= ~flag;
	
	diag_set_env_conf_u32("mm_flags", mm_flags);			
	
	if (set_or_clean == 0)
		diag_do_clear_backup_env();

//	if (set_or_clean)
//		diag_do_save_backup_env();	
//	else
//		diag_do_clear_backup_env();
	return;	
}

void diag_set_boot_priority_flag(void)
{
	unsigned int set_or_clean;
	
	printf("Current OS boot Priority = %x\n", diag_get_env_conf_u32("os_boot_priority"));

	if (diag_get_value("0: Auto, 1: Linux, 2: WinCE", &set_or_clean, 0, 2, DIAG_GET_VALUE_DEC_MODE))
		return;

	diag_set_env_conf_u32("os_boot_priority", set_or_clean);			

	return;	
}

void  diag_do_t0_0_flag(void)
{
	printf("\nNow T0 Flag is ");

	diag_get_mm_flag(T0_0_FLAG);
	diag_set_mm_flag(T0_0_FLAG);

	return;

}

void  diag_do_t0_1_flag(void)
{
	printf("\nNow T0-1 Flag is ");

	diag_get_mm_flag(T0_1_FLAG);
	diag_set_mm_flag(T0_1_FLAG);

	return;

}

/* ----- Set/Clear FLAG MENU - START ----- */
/* T0 MENU - START */
burnin_cmd t0_flag_cmd_value[] = {
	{0, '0', "T0() Flag", diag_do_t0_0_flag, 1},
	{1, '1', "T0-1() Flag", diag_do_t0_1_flag, 1},
	{2, '2', "T0-2(OS Type)", diag_set_boot_priority_flag, 0},	
	{-1, '*', "",0,0}
};

void diag_do_t0_flag(void)
{
	burnin_cmd *burnin_temp = t0_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* T0 MENU - END */

/* T1 MENU - START */
void diag_do_t1_0_flag(void)
{
	printf("\nNow T1(MP) Flag is ");
	diag_get_mm_flag(T1_0_FLAG);
	diag_set_mm_flag(T1_0_FLAG);

	return;
}

void diag_do_t1_1_flag(void)
{
	printf("\nNow T1-1(MAC) Flag is ");
	diag_get_mm_flag(T1_1_FLAG);
	diag_set_mm_flag(T1_1_FLAG);

	return;
}

void diag_do_t1_2_flag(void)
{
	printf("\nNow T1-2(RFnum) Flag is ");
	diag_get_mm_flag(T1_2_FLAG);
	diag_set_mm_flag(T1_2_FLAG);
	return;
}

void diag_do_t1_3_flag(void)
{
	printf("\nNow T1-3(GPStest) Flag is ");
	diag_get_mm_flag(T1_3_FLAG);
	diag_set_mm_flag(T1_3_FLAG);
	return;
}

burnin_cmd t1_flag_cmd_value[] = {
	{0, '0', "T1(MP) Flag", diag_do_t1_0_flag, 1},
	{1, '1', "T1-1(MAC) Flag", diag_do_t1_1_flag, 1},
	{2, '2', "T1-2(RFnum) Flag", diag_do_t1_2_flag, 1},
	{3, '3', "T1-3(GPStest) Flag", diag_do_t1_3_flag, 1},
	{-1, '*', "",0,0}
};

void diag_do_t1_flag(void)
{
	burnin_cmd *burnin_temp = t1_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* T1 MENU - END */

/* T2 MENU - START */
void diag_do_t2_0_flag(void)
{
	printf("\nNow T2 Flag is ");
	diag_get_mm_flag(T2_FLAG);
	diag_set_mm_flag(T2_FLAG);

	return;
}

burnin_cmd t2_flag_cmd_value[] = {
	{0, '0', "T2 (Burn in) Flag", diag_do_t2_0_flag, 1},
	{-1, '*', "", 0, 0}
};

void diag_do_t2_flag(void)
{
	burnin_cmd *burnin_temp = t2_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}

void diag_do_t3_0_flag (void)
{
	printf("\nNow T3-0(K) Flag is ");
	diag_get_mm_flag(T3_0_FLAG);
	diag_set_mm_flag(T3_0_FLAG);

	return ;
}

void diag_do_t3_1_flag(void)
{
	printf("\nNow T3-1(GSM) Flag is ");
	diag_get_mm_flag(T3_1_FLAG);
	diag_set_mm_flag(T3_1_FLAG);
	
	return;
}

void diag_do_t3_2_flag(void)
{
	printf("\nNow T3-2(MN) Flag is ");
	diag_get_mm_flag(T3_2_FLAG);
	diag_set_mm_flag(T3_2_FLAG);

	return;
}

void diag_do_t3_3_flag(void)
{
	printf("\nNow T3-3(RF) Flag is ");
	diag_get_mm_flag(T3_3_FLAG);
	diag_set_mm_flag(T3_3_FLAG);
	
	return;
}

void diag_do_t3_4_flag(void)
{
	printf("\nNow T3-4(GPS) Flag is ");
	diag_get_mm_flag(T3_4_FLAG);
	diag_set_mm_flag(T3_4_FLAG);
	
	return;
}

void diag_do_t3_5_flag(void)
{
	printf("\nNow T3-5(FG) Flag is ");
	diag_get_mm_flag(T3_5_FLAG);
	diag_set_mm_flag(T3_5_FLAG);
	
	return;
}

void diag_do_t3_6_flag(void)
{
	printf("\nNow T3-6(CVID) Flag is ");
	diag_get_mm_flag(T3_6_FLAG);
	diag_set_mm_flag(T3_6_FLAG);
	
	return;
}

burnin_cmd t3_flag_cmd_value[] = {
	{0, '0', "T3(K) Flag", diag_do_t3_0_flag, 1},
	{1, '1', "T3-1(GSM) Flag", diag_do_t3_1_flag, 1},
	{2, '2', "T3-2(MN) Flag", diag_do_t3_2_flag, 1},
	{3, '3', "T3-3(RF) Flag", diag_do_t3_3_flag, 1},
	{4, '4', "T3-4(GPS) Flag", diag_do_t3_4_flag, 1},
	{5, '5', "T3-5(FG) Flag", diag_do_t3_5_flag, 1},
	{6, '6', "T3-6(CVID) Flag", diag_do_t3_6_flag, 1},
	{-1,'*', "",0,0}
};

void diag_do_t3_flag(void)
{
	burnin_cmd *burnin_temp = t3_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* T3 MENU - END */

/* EOT MENU - START */
void diag_do_eot_flag_setting(void)
{
	printf("\nNow EOT Flag is ");
	diag_get_mm_flag(EOT_FLAG);
	diag_set_mm_flag(EOT_FLAG);
	
	return;
}

burnin_cmd eot_flag_cmd_value[] = {
	{0, '0', "EOT Flag", diag_do_eot_flag_setting, 1},
	{1, '1', "Model-T Flag", diag_do_nothing, 1},
	{-1, '*',"",0,0}
};

void diag_do_eot_flag(void)
{
	burnin_cmd *burnin_temp = eot_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* EOT MENU - END */

/* MODULE MENU - START */
burnin_cmd module_flag_cmd_value[] = {
	{0, '0', "Module T0 Flag", diag_do_nothing, 1},
	{1, '1', "Module T1 Flag", diag_do_nothing, 1},
	{-1,'*', "",0,0}
};

void diag_do_module_flag(void)
{
	burnin_cmd *burnin_temp = module_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* MODULE MENU - END */

/* Set/Clear ALL MENU - START */
void diag_do_clear_all_flag(void)
{
	diag_set_env_conf_u32("mm_flags", 0);

	diag_do_clear_backup_env();

	return;
}

void diag_do_set_all_flag(void)
{
	unsigned int mm_flags;

        mm_flags = (T1_0_FLAG | T1_1_FLAG | T1_2_FLAG | T1_3_FLAG | T2_FLAG| T3_0_FLAG | T3_1_FLAG | \
                    T3_2_FLAG | T3_3_FLAG | T3_4_FLAG | T3_5_FLAG); //| T3_6_FLAG | EOT_FLAG);

	diag_set_env_conf_u32("mm_flags", mm_flags);

	return;
}

void diag_do_view_all_flag(void)
{
	unsigned int mm_flags;

	mm_flags = diag_get_env_conf_u32("mm_flags");

	printf("T1-0 Flag is %s.\n", (mm_flags & T1_0_FLAG)?"set":"cleared");
	printf("T1-1 Flag is %s.\n", (mm_flags & T1_1_FLAG)?"set":"cleared");
	printf("T1-2 Flag is %s.\n", (mm_flags & T1_2_FLAG) ? "set" : "cleared");
	printf("T1-3 Flag is %s.\n", (mm_flags & T1_3_FLAG) ? "set" : "cleared");
	printf("T2 Flag is %s.\n",   (mm_flags & T2_FLAG)?"set":"cleared");
	printf("T3_0_Flag is %s.\n", (mm_flags & T3_0_FLAG ) ? "set" : "cleared");
	printf("T3_1_Flag is %s.\n", (mm_flags & T3_1_FLAG ) ? "set" : "cleared");
	printf("T3_2_Flag is %s.\n", (mm_flags & T3_2_FLAG ) ? "set" : "cleared");
	printf("T3_3_Flag is %s.\n", (mm_flags & T3_3_FLAG ) ? "set" : "cleared");
	printf("T3_4_Flag is %s.\n", (mm_flags & T3_4_FLAG ) ? "set" : "cleared");
	printf("T3_5_Flag is %s.\n", (mm_flags & T3_5_FLAG ) ? "set" : "cleared");
//	printf("T3_6_Flag is %s.\n", (mm_flags & T3_6_FLAG ) ? "set" : "cleared");
//	printf("EOT Flag is %s.\n",  (mm_flags&EOT_FLAG)?"set":"cleared");
	
	return;
}

burnin_cmd all_flag_cmd_value[] = {
	{0, '0', "Clear All Flag", diag_do_clear_all_flag, 1},
	{1, '1', "Set All Flag", diag_do_set_all_flag, 1},
	{2, '2', "View All Flag", diag_do_view_all_flag, 1},
	{-1, '*', "",0,0}
};

void diag_do_all_flag(void)
{
	burnin_cmd *burnin_temp = all_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* Set/Clear ALL MENU - END */

__maybe_unused static void diag_do_get_boot_counter (void)
{
    char *s;

    s = getenv("boot_counter");

    if (s == NULL){
            printf("boot counter : 0 times\n");
    } else {
            printf("boot counter : %s times\n", s);
    }
}

__maybe_unused static void diag_do_get_count_tester (void)
{
    char *s;

    s = getenv("tester");

    if (s == NULL){
            printf("tester : 0 times\n");
    } else {
            printf("tester : %s times\n", s);
    }
}

burnin_cmd set_clear_flag_cmd_value[] = {
	{0, '0', "T0 Flag", diag_do_t0_flag, 1},
	{1, '1', "T1 Flag", diag_do_t1_flag, 1},	
	{2, '2', "T2 Flag", diag_do_t2_flag, 1},	
	{3, '3', "T3 Flag", diag_do_t3_flag, 1},	
	{4, '4', "EOT Flag", diag_do_eot_flag, 1},	
	{5, '5', "Module Flag", diag_do_module_flag, 1},		
	/*{6, '6', "get boot counter", diag_do_get_boot_counter, 1},		
	{7, '7', "get count test", diag_do_get_count_tester, 1},		*/
	{98, 'a', "Set/Clear All Flag", diag_do_all_flag, 1},
	{-1, '*', "", 0, 0}
};

void diag_do_set_clear_flag_func(void)
{
	burnin_cmd *burnin_temp = set_clear_flag_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- Set/Clear FLAG MENU - END ----- */

/* ----- DOWNLOAD/UPLOAD MENU - START ----- */

int do_download_bios(void)
{
	char *bios_name = "SPI.ROM";
	int ret = 0;
	char buf[128] = {0};

	printf ("\r\nBIOS File Name (%s): ", bios_name);

	if ((ret = con_get_string (buf, sizeof (buf))) == -1) {
		printf ("\r\n");
		goto EXIT;
	} else if (ret == 0) {
		sprintf (buf, "%s", bios_name);
	}

	ret = download_bios(buf);

	if(ret)
		goto EXIT;

	printf ("\n");

EXIT:
	return ret;	

}

void diag_do_download_bios(void)
{
	int ret = 0;

	ret = do_download_bios();

	if (ret)
		 printf ("\r\nBIOS Upgrade FAIL...\r\n");
	else
		 printf ("\r\nBIOS Upgrade OK...\r\n");

	return;
}


void diag_do_download_firmware_mirror_mmc (void)
{
	int ret = 0;

	ret = do_download_firmware_mirror_mmc();

	if (ret)
		printf ("\r\nFIRMWARE Upgrade FAIL...\r\n");
	else
		printf ("\r\nFIRMWARE Upgrade OK...\r\n");
	
	return;
}

int do_download_firmware_copy_from_file (void)
{
	char *fw_name = "firmware.img";
	int ret = -1;
	char buf [MAX_SIZE_128BYTE] = {0};

	printf ("\r\nFirmware File Name (%s): ", fw_name);

	if ((ret = con_get_string (buf, sizeof (buf))) == -1) {
		printf ("\r\n");
		goto EXIT;
	} else if (ret == 0) {
		sprintf (buf, "%s", fw_name);
	}

	ret = download_firmware_copy_from_file(buf);

EXIT:
	return ret;
}

void diag_do_download_firmware_copy_from_file (void)
{
	int ret = 0;

	ret = do_download_firmware_copy_from_file();

	if (ret)
		printf ("\r\nFIRMWARE Upgrade FAIL...\r\n");
	else
		printf ("\r\nFIRMWARE Upgrade OK...\r\n");
	
	return;
}

int do_tftp_download_firmware (void)
{
	char *fw_name = "firmware.img";
	int ret = 0;
	char buf[MAX_SIZE_128BYTE] = {0};
	unsigned int set_or_clean = 0;

	printf("\r\n Current IP Address \n");
	
	ret = show_ip();

	if (ret)
		goto EXIT;

	printf("\nDo you want to change the ip address?\n");

	diag_get_value("0 - No, 1 - Yes", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE);

	if (set_or_clean == 1){
		ret = change_ip();
	
		if(ret)
			goto EXIT;
	}
                
	printf ("\r\nFirmware File Name (%s): ", fw_name);

	if ((ret = con_get_string (buf, sizeof (buf))) == -1) {
		printf ("\r\n");
		goto EXIT;
	} else if (ret == 0) {
		sprintf (buf, "%s", fw_name);
	}

	printf("download firmware name:%s\n", buf);

	printf ("\n");

	ret = tftp_download_firmware(buf, &sys_info);

EXIT:

        return ret;

}

void diag_do_tftp_download_firmware (void)
{
	int ret = 0;

	ret = do_tftp_download_firmware();


	printf("**************************************************\n");
	printf("*                                                *\n");
	
	if(ret)
		printf("*          TFTP FIRMWARE file transfer fail.     *\n");
        else
		printf("*          TFTP FIRMWARE file transfer success   *\n");
	
	printf("*                                                *\n");
	printf("**************************************************\n");

	return;
}

burnin_cmd download_firmware_cmd_value[] = {
	{0, '0', "Mirror MMC0", diag_do_download_firmware_mirror_mmc, 1},
	{1, '1', "Copy File From SD card", diag_do_download_firmware_copy_from_file, 1}, //R42A
	{2, '2', "Copy File From TFTP", diag_do_tftp_download_firmware, 1}, //R
	{-1,'*', "",0,0}
};

void diag_do_download_firmware(void)
{
	burnin_cmd *burnin_temp = download_firmware_cmd_value;
	bios_manual_testing (burnin_temp);
}

burnin_cmd download_upload_cmd_value[] = {
	{ 0,'0',"BIOS", diag_do_download_bios, 1},
	{ 1,'1',"Firmware", diag_do_download_firmware, 1},
	{ 2,'2',"Error Log", diag_do_nothing, 1},	
	{ 3,'3',"IMAGE(FB)", diag_do_nothing, 1},	
	{ 4,'4',"Kernel", diag_do_nothing, 1},	
	{ 5,'5',"Root Disk", diag_do_nothing, 1},	
	{ 6,'6',"User Disk", diag_do_nothing, 1},		
	{ -1,'*',"",0,0}
};

void diag_do_download_upload_func(void)
{
	burnin_cmd *burnin_temp = download_upload_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- DOWNLOAD/UPLOAD MENU - END ----- */

/* ----- SET SN/MAC MENU - START ----- */

//R66 - Start
void diag_do_r_w_serial_number_by_eeprom(UINT8 r_w)
{

	char buf [MAX_SN_SIZE] = {0};
	int i = 0;

	if (r_w) {
		memset(CharBuf, 0, sizeof (CharBuf));
		memset(buf, 0, MAX_SN_SIZE);

		eeprom_read(EEPROM_I2C_ADDR, SN_BASE_ADDR, (unsigned char *)CharBuf, MAX_SN_SIZE);

		for (i = 0; i < MAX_SN_SIZE; i++) {
			if (CharBuf[i] == 0xff)
				break;

			buf[i] = CharBuf[i];
		}

		printf("(EEPROM)Serial Number: %s\r\n", buf);
	
	} else {
		memset(buf, 0xff, MAX_SN_SIZE);

		if (eeprom_write(EEPROM_I2C_ADDR, SN_BASE_ADDR, (unsigned char *)buf, MAX_SN_SIZE) != 0)
			printf("Write Serial Number to EEPROM Fail!\n");

		if (eeprom_write(EEPROM_I2C_ADDR, SN_BASE_ADDR, (unsigned char *)CharBuf, strlen((char *)CharBuf)) != 0)
			printf("Write Serial Number to EEPROM Fail!\n");
	}

}

void diag_do_r_w_model_name_by_eeprom(UINT8 r_w)
{

	char buf[MAX_MN_SIZE] = {0};
	int i = 0;

	if (r_w) {
		memset(buf, 0, MAX_MN_SIZE);
		memset(CharBuf, 0, sizeof (CharBuf));
		eeprom_read(EEPROM_I2C_ADDR, MN_BASE_ADDR, (unsigned char *)CharBuf, MAX_MN_SIZE);

		for(i = 0; i < MAX_MN_SIZE; i++) {
			if (CharBuf[i] == 0xff)
				break;

			buf[i] = CharBuf[i];
		}

		printf("(EEPROM)Model Name: %s\r\n", buf);

	} else {

		memset(buf, 0xff, MAX_MN_SIZE);

		if (eeprom_write(EEPROM_I2C_ADDR, MN_BASE_ADDR, (unsigned char *)buf, MAX_MN_SIZE) != 0)
			printf("Write Model Name to EEPROM Fail!\n");

		if (eeprom_write(EEPROM_I2C_ADDR, MN_BASE_ADDR, (unsigned char *)CharBuf, strlen((char *)CharBuf)) != 0)
			printf("Write Model Name to EEPROM Fail!\n");
	}

}

void diag_do_r_w_customer_id_by_eeprom(UINT8 r_w)
{

	char buf [MAX_CV_SIZE] = {0};
	int i = 0;

	if (r_w) {
		memset(buf, 0, MAX_CV_SIZE);
		memset (CharBuf, 0, sizeof (CharBuf));
		eeprom_read(EEPROM_I2C_ADDR, CV_BASE_ADDR, (unsigned char *) CharBuf, MAX_CV_SIZE);

		for(i = 0; i < MAX_CV_SIZE; i++) {
			if (CharBuf[i] == 0xff)
				break;

			buf[i] = CharBuf[i];
		}

		printf("(EEPROM)Customer ID: %s\r\n", buf);
	
	} else {

		memset (buf, 0xff, MAX_CV_SIZE);

		if (eeprom_write(EEPROM_I2C_ADDR, CV_BASE_ADDR, (unsigned char *)buf, MAX_CV_SIZE) != 0)
			printf("Write Customer ID to EEPROM Fail!\n");

		if (eeprom_write(EEPROM_I2C_ADDR, CV_BASE_ADDR, (unsigned char *)CharBuf, strlen((char *)CharBuf)) != 0)
			printf("Write Customer ID to EEPROM Fail!\n");
	}
}


void diag_do_set_model_name(void)
{
        int ret, i;

MODEL_NAME:

	printf("\r\nModel Name : ");
	if ((i = con_get_string(CharBuf,sizeof(CharBuf))) == -1) {
		printf("\r\n");
		return;
	} else if(i == 0) {
		goto MODEL_NAME;
	} else if(i > 80) {
		printf ("\r\nModl Name Length != 12\n");
		goto MODEL_NAME;
	}

	printf("\r\n");
	diag_do_r_w_model_name_by_eeprom(0);
	sprintf(CmdMsg, "setenv modelname %s", CharBuf);
        
	if ((ret = run_command(CmdMsg, 0)) != 0){
		printf("Set model name Fail [%d] \r\n", ret);
		return;
	}

	if ((ret = run_command("saveenv", 0)) != 0){
		printf("Set model name Fail [%d] \r\n", ret);
		return;
	}

	return;
}

void diag_do_set_customer_id(void)
{
	int ret, i;

CUSTOMER_ID:
	printf("\r\nCustomer ID : ");

	if ((i = con_get_string(CharBuf, sizeof(CharBuf))) == -1) {
		printf("\r\n");
		return;
	} else if (i == 0){
		goto CUSTOMER_ID;
	} else if (i != 12) {
		printf ("\r\nCustomer ID Length != 12\n");
		goto CUSTOMER_ID;
	}

	printf("\r\n");
	diag_do_r_w_customer_id_by_eeprom(0);
	sprintf(CmdMsg, "setenv customer_id %s", CharBuf);

	if ((ret = run_command(CmdMsg, 0)) != 0){
		printf("Set Customer ID Fail [%d] \r\n", ret);
		return;
	}

	if ((ret = run_command("saveenv", 0)) != 0){
		printf("Set Customer ID Fail [%d] \r\n", ret);
		return;
	}

	return;
}

/* SN Setting - START */
void diag_do_set_serial_number(void)
{
	int ret, i;

SERIAL_NUMBER:
	printf("\r\nSerial Number : ");
	
	if((i = con_get_string(CharBuf, sizeof(CharBuf))) == -1){
		printf("\r\n");
		return;
	} else if(i == 0) {
		goto SERIAL_NUMBER;
	} else if(i != 12) {
		printf ("\r\nSerial Number Length != 12\n");
		goto SERIAL_NUMBER;
	}

	printf("\r\n");

	diag_do_r_w_serial_number_by_eeprom(0);
	sprintf(CmdMsg, "setenv serialnumber %s", CharBuf);

	if ((ret = run_command(CmdMsg, 0)) != 0) {
		printf("Set Serial Number Fail [%d] \r\n", ret);
		return;
	}

	printf("\r\n");
	sprintf(CmdMsg, "saveenv");

	if ((ret = run_command(CmdMsg, 0)) != 0) {
		printf("Set Serial Number Fail [%d] \r\n", ret);
		return;
	}
	
	return;
}
/* SN Setting - END */

/* MAC Setting - START */
void diag_do_set_mac_address(void)
{

	int ret, i, length = 0;

MAC_ADDRESS:
	printf("\r\nMAC Address(0090e8112233) : ");
	if ((i=con_get_string(CharBuf, sizeof(CharBuf))) == -1){
		printf("\r\n");
		return;
	} else if(i == 0) {
		goto MAC_ADDRESS;
	}

	if ((length = strlen((char *)CharBuf)) == 17) {
		for (i = 0; i < length; i++) {
			if ((i >= 2) && ((i-2)%3 == 0)) {
				if(CharBuf[i] != ':') {
					printf("MAC Address Incorrect\r\n");
					goto MAC_ADDRESS;
				}
			} else {
				if (_is_hex(CharBuf[i]) == 0) {
					printf("MAC Address Incorrect\r\n");
					goto MAC_ADDRESS;
				}
			}
		}
	} else if (length == 12) {
		for(i = 0; i < length; i++){
			if(_is_hex(CharBuf[i]) == 0){
				printf("MAC Address Incorrect\r\n");
				goto MAC_ADDRESS;
			}
		}

		CharBuf[17] = 0;
		CharBuf[16] = CharBuf[11];
		CharBuf[15] = CharBuf[10];
		CharBuf[14] = ':';
		CharBuf[13] = CharBuf[9];
		CharBuf[12] = CharBuf[8];
		CharBuf[11] = ':';
		CharBuf[10] = CharBuf[7];
		CharBuf[9] = CharBuf[6];
		CharBuf[8] = ':';
		CharBuf[7] = CharBuf[5];
		CharBuf[6] = CharBuf[4];
		CharBuf[5] = ':';
		CharBuf[4] = CharBuf[3];
		CharBuf[3] = CharBuf[2];
		CharBuf[2] = ':';
		CharBuf[1] = CharBuf[1];
		CharBuf[0] = CharBuf[0];
		length = 17;
	} else {
		printf("MAC Address Incorrect\r\n");
		goto MAC_ADDRESS;
	}

	printf("\r\n");
	sprintf(CmdMsg, "setenv ethaddr %s", CharBuf);

	if ((ret = run_command(CmdMsg, 0)) != 0) {
		printf("Set LAN1 MAC Address Fail [%d] \r\n", ret);
		return;
	}

	printf("\r\n");
	i = 16;
	
	while(i) {
		if ((CharBuf[i] == 'F')|| (CharBuf[i] == 'f')) {
			CharBuf[i] = '0';
			i--;
		} else if (CharBuf[i] == ':') {
			i--;	
		} else {
			break;	
		}
	}
	
	if (CharBuf[i] == '9')
		CharBuf[i] = 'A';
	else
		CharBuf[i] = CharBuf[i]+1;
	
	printf("\r\n");
	sprintf(CmdMsg, "setenv eth1addr %s", CharBuf);

	if ((ret = run_command(CmdMsg, 0)) != 0) {
		printf("Set LAN2 MAC Address Fail [%d] \r\n", ret);
		return;
	}

	printf("\r\n");
		
	sprintf(CmdMsg, "saveenv");

	if ((ret = run_command(CmdMsg, 0)) != 0) {
		printf("Saveenv Fail [%d] \r\n", ret);
		return;
	}

	return;
}
/* MAC Setting - END */

/* VIEW SN/MAC Setting - START */
void diag_do_show_sn_mac(void)
{
	char *s;

	s = getenv ("serialnumber");
	printf("\r\n");
	printf("Serial Number: %s\r\n", s);
	
	s = getenv ("ethaddr");
	printf("LAN1 MAC Address: %s\r\n", s);
	
	s = getenv ("eth1addr");
	printf("LAN2 MAC Address: %s\r\n", s);

	s = getenv ("modelname");
	printf("Model Name: %s\r\n", s);
        
	s = getenv ("customer_id");
        printf("Customer ID: %s\r\n", s);
       
	diag_do_r_w_model_name_by_eeprom(1);
	diag_do_r_w_serial_number_by_eeprom(1);
	diag_do_r_w_customer_id_by_eeprom(1);

	return;
}

void diag_do_set_hw_version (void) {

	unsigned int set_or_clean = 0;
	char hw_ver [MAX_HW_VERSION_SIZE];
	int ret = 0;

	memset(&hw_ver, 0, sizeof(hw_ver));

	ret = diag_get_value("set HwMajorVersion1: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret){
		printf("set HwMajorVersion1 fail");
		return;
	}

	hw_ver [HW_MAJ1] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMinorVersion1: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret) {
		printf("set HwMinorVersion1 fail");
		return;
	}

	hw_ver [HW_MIN1] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMajorVersion2: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret){
		printf("set HwMajorVersion1 fail");
		return;
	}

	hw_ver [HW_MAJ2] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMinorVersion2: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret){
		printf("set HwMinorVersion1 fail");
		return;
	}

	hw_ver [HW_MIN2] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMajorVersion3: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret){
		printf("set HwMajorVersion3 fail");
		return;
	}

	hw_ver [HW_MAJ3] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMinorVersion3: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret) {
		printf("set HwMinorVersion3 fail");
		return;
	}

	hw_ver [HW_MIN3] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMajorVersion4: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret) {
		printf("set HwMajorVersion4 fail");
		return;
	}

	hw_ver [HW_MAJ4] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMinorVersion4: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret) {
		printf("set HwMinorVersion4 fail");
		return;
	}

	hw_ver [HW_MIN4] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMajorVersion5: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret) {
		printf("set HwMajorVersion5 fail");
		return;
	}

	hw_ver [HW_MAJ5] = (set_or_clean & 0xff);

	ret = diag_get_value("set HwMinorVersion5: 0 ~ 255", &set_or_clean, 0, 255, DIAG_GET_VALUE_DEC_MODE);

	if (ret){
		printf("set HwMinorVersion5 fail");
		return;
	}

	hw_ver [HW_MIN5] = (set_or_clean & 0xff);

	if(eeprom_write(EEPROM_I2C_ADDR, HW_VERSION_BASE_ADDR, (unsigned char *)hw_ver, MAX_HW_VERSION_SIZE) != 0)
		printf("Write HW_VERSION to EEPROM Fail!\n");

	printf("OK\n");

	return;
}

int do_get_hw_verison(char *s) {

	i2c_set_bus_num(0);

	return eeprom_read(EEPROM_I2C_ADDR, HW_VERSION_BASE_ADDR, (unsigned char *) s, MAX_HW_VERSION_SIZE);
}

/* VIEW HW Verison - START */
void diag_do_show_hw_verison(void)
{
	char hw_ver [MAX_HW_VERSION_SIZE];
	int  i = 0;

	memset(&hw_ver, 0, sizeof(hw_ver));

	do_get_hw_verison(hw_ver);

	for ( i = 0; i < MAX_HW_VERSION_SIZE; i += 2) {
		printf("HwMajorVersion%d: %d, hex: 0x%x\n", i, hw_ver[i], hw_ver[i]);
		printf("HwMinorVersion%d: %d, hex: 0x%x\n", i, hw_ver[i + 1], hw_ver[i + 1]);
	}

	return;
}
/* VIEW Hw version  - END */
/* VIEW SN/MAC Setting - END */

burnin_cmd sn_mac_config_cmd_value[] = {
    {0, '0', "SN Setting", diag_do_set_serial_number, 1},
    {1, '1', "MAC Setting", diag_do_set_mac_address, 1},
    {1, '2', "Model Name Setting", diag_do_set_model_name, 1},
    {1, '3', "Customer ID Setting", diag_do_set_customer_id, 1},
    {2, '2', "View MOXA Info Setting", diag_do_show_sn_mac, 1},  //R66 
    {3, '3', "HW Version Setting", diag_do_set_hw_version, 1},
    {4, '4', "View HW Version", diag_do_show_hw_verison, 1},
    {-1, '*', "", 0, 0}
};

void diag_do_sn_mac_config_func(void)
{
	burnin_cmd *burnin_temp = sn_mac_config_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- SET SN/MAC - END ----- */

/* ----- TFTP SETTING - START ----- */
/* Default Setting - START */
void diag_do_tftp_setting_default(void)
{
	tftp_setting_default();
	return;
}
/* Default Setting - END */

/* User Define - START */
void diag_do_change_ip(void)
{
	change_ip();
	return;
}
/* User Define - END */

/* View Setting - START */
void diag_do_show_ip(void)
{
	show_ip();
	return;

}
/* User Define - END */

burnin_cmd tftp_setting_cmd_value[] = {
    {0, '0', "Default Setting", diag_do_tftp_setting_default, 1},
    {1, '1', "User Define", diag_do_change_ip, 1},
    {2, '2', "View Setting", diag_do_show_ip, 1},
    {-1, '*', "",0,0}
};

void diag_do_tftp_setting_func(void)
{
	burnin_cmd *burnin_temp = tftp_setting_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- TFTP SETTING - END ----- */

/* ----- Ethernet Test (MAC&PHY) - START ----- */
void diag_do_eth1_100_cmd_test(void)
{
	do_eth1_100_cmd_test();
 	return;
}

void diag_do_eth2_100_cmd_test(void)
{
	do_eth2_100_cmd_test();
	return;
}

void diag_do_eth1_10_cmd_test(void)
{
	do_eth1_10_cmd_test();
	return;
}

void diag_do_eth2_10_cmd_test(void)
{
	do_eth2_10_cmd_test();
	return;
}

void diag_do_eth_100_burnin_test(void)
{
  	run_command("mii write 4 4 100", 0);	
  	run_command("mii write 4 0 8000", 0);	
  	run_command("mii write 5 4 100", 0);	
  	run_command("mii write 5 0 8000", 0);	
	udelay(1000*1000);
	while(1) {
                udelay(50*1000);
		if (check_if_press_character(ESC)) {
			return;
		}		
                run_command("setenv -f ethact cpsw0", 0);
        	run_command("ethloop", 0);
                run_command("setenv -f ethact cpsw1", 0);
        	run_command("ethloop", 0);
	}
	return;
}

void diag_do_eth_phy_power_saving_analog_off_1 (void)
{
	unsigned int set_or_clean;
	
	if (diag_get_value("0 - disable, 1 - enable", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_eth_phy_power_saving_analog_off_1(set_or_clean);

	return;
}

void diag_do_eth_phy_power_saving_power_down_1 (void)
{
	unsigned int set_or_clean;
	
	if (diag_get_value("0 - disable, 1 - enable", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_eth_phy_power_saving_power_down_1(set_or_clean);

	return;
}


void diag_do_eth_phy_power_saving_analog_off_2 (void)
{
	unsigned int set_or_clean;
	
	if (diag_get_value("0 - disable, 1 - enable", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_eth_phy_power_saving_analog_off_2(set_or_clean);	

	return;
}

void diag_do_eth_phy_power_saving_power_down_2 (void)
{
	unsigned int set_or_clean;
	
	if (diag_get_value("0 - disable, 1 - enable", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_eth_phy_power_saving_power_down_2(set_or_clean);

	return;
}

burnin_cmd eth_phy_power_cmd_value[] = {
	{0, '0', "Eth1 Analog OFF mode", diag_do_eth_phy_power_saving_analog_off_1, 1},
	{1, '1', "Eth1 Power Down mode", diag_do_eth_phy_power_saving_power_down_1, 1},
	{2, '2', "Eth2 Analog OFF mode", diag_do_eth_phy_power_saving_analog_off_2, 1},
	{3, '3', "Eth2 Power Down mode", diag_do_eth_phy_power_saving_power_down_2, 1},		
	{-1,'*',"",0,0}
};

void diag_do_eth_phy_power_saving(void)
{
	burnin_cmd *burnin_temp = eth_phy_power_cmd_value;
	bios_manual_testing(burnin_temp);
	return;
}

burnin_cmd eth_cmd_value[] = {
	{0, '0', "ETH1 10M External Loop Back",diag_do_eth1_10_cmd_test, BIOS_10M_TEST},
	{1, '1', "ETH2 10M External Loop Back",diag_do_eth2_10_cmd_test, BIOS_10M_TEST},
	{2, '2', "ETH1 100M External Loop Back",diag_do_eth1_100_cmd_test, BIOS_100M_TEST},
	{3, '3', "ETH2 100M External Loop Back",diag_do_eth2_100_cmd_test, BIOS_100M_TEST},
	{4, '4', "ETH  100M Brun-in",diag_do_eth_100_burnin_test, BIOS_100M_TEST},
	{5, '5', "PHY Power Saving Mode",diag_do_eth_phy_power_saving, 0},
	{-1,'*',"",0,0}
};

void diag_do_eth_func(void)
{
	burnin_cmd *burnin_temp = eth_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- Ethernet Test (MAC&PHY) - END ----- */

/* ----- UART Test - START ----- */
void diag_do_pcuart_test(void)
{
		/* Set SN/MAC by UART without beep at the end */
        GetSN_MAC(0);
}

void diag_do_uart_232_test (void)
{
	int ret = 0;
        
        ret = do_usb_uart_test (0, 10);

	if (ret)
		printf("FAIL\n");
	else
		printf("SUCCESS");

	return;
}

void diag_do_uart_422_test (void)
{
	int ret = 0;

        ret = do_usb_uart_test (1, 10);

	if (ret)
		printf("FAIL\n");
	else
		printf("SUCCESS");


	return;
}

void diag_do_uart_485_2w_test (void)
{
	int ret = 0;
	
        ret = do_usb_uart_test (2, 10);
	
	if (ret)
		printf("FAIL\n");
	else
		printf("SUCCESS");
	

	return ;
}

void diag_do_uart_232_rts_test (void)
{
        //unsigned int val;
        int ret = 0;
/*	if (diag_get_value(" 1 - port1\n 2 - port2\n 3 - port3\n 4 - port4\n", &port, 1, 4, DIAG_GET_VALUE_DEC_MODE))
		return;

	if (diag_get_value("0 - disable, 1 - enable", &val, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;*/
        
        ret = do_usb_uart_test (0, 1);
	if (ret){
		return;
        }
	

        diag_do_uart_232_RTS_CTS_test();
        //set_uart_rts (port, val);
}

void diag_do_uart_232_cts_print (void)
{
        int port;

        for(port = 1; port <= 4; port ++)
                get_uart_cts (port);
}

burnin_cmd uart_cmd_value[] = {
    {0, '0', "PC to UART Command", diag_do_pcuart_test, 1},
#if	RS232_MP
    {1, '1', "RS-232 Test", diag_do_uart_232_test, 1},
#endif
#if	RS422_MP
    {2, '2', "RS-422 Test", diag_do_uart_422_test, 1},
#endif
#if	RS4852W_MP
    {3, '3', "RS-485-2W Test", diag_do_uart_485_2w_test, 1},
#endif
#if	RS232_MP
    {4, '4', "RS-232 RTS Test", diag_do_uart_232_rts_test, 1},
#endif
    //{4, '4', "RS-232 CTS print", diag_do_uart_232_cts_print, 1},
    {-1,'*', "",0,0}
};

void diag_do_uart_func(void)
{
	burnin_cmd *burnin_temp = uart_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- UART Test - END ----- */

/* ----- IO TEST - START ----- */
/* ----- RTC Test - START ----- */
void diag_do_read_rtc (void)
{
	do_RTC_init();

	while(1) {

		if (check_if_press_character(ESC))
			return;
		
		udelay(200*1000);
		do_RTC_view_datetime();
	}

	return;
}

void diag_do_set_rtc(void)
{
	do_RTC_init();
	do_RTC_set_datetime();

	return;
}

void diag_do_rtc_test(void)
{
	int ret = do_rtc_test();

	if (ret)
		printf("\nRTC testing is fail !");
	else
		printf("\nRTC testing is OK.");
	return;
}

void diag_do_rtc_set_enable(void)
{
	do_rtc_set_enable();
	return;
}

void diag_do_read_rtc_wo_init (void)
{
	while(1) {

		if (check_if_press_character(ESC))
			return;

		udelay(200*1000);
		do_RTC_view_datetime();
	}

	return;
}

void diag_do_rtc_backup_read_test(void)
{
	return;
}
void diag_do_rtc_backup_write_test(void)
{
	return;
}

burnin_cmd rtc_cmd_value[] = {
	{0, '0', "Read RTC", diag_do_read_rtc, 1},
	{1, '1', "Set RTC", diag_do_set_rtc, 1},
	{2, '2', "RTC testing", diag_do_rtc_test, 1},
	{3, '3', "RTC Enable/Disable", diag_do_rtc_set_enable, 1},
	{4, '4', "Read RTC (w/o init)", diag_do_read_rtc_wo_init, 1},
	{1, 1, "RTC Backup Battery Read Test", diag_do_rtc_backup_read_test, 0},
	{1, 1, "RTC Backup Battery Write Test", diag_do_rtc_backup_write_test, 0},
	{-1, '*', "", 0, 0}
};

void diag_do_rtc_func(void)
{
	burnin_cmd *burnin_temp = rtc_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- RTC Test - START ----- */

/* ----- USB Test - START ----- */
void diag_do_usb_detect_test(void)
{
	int ret = 0;

#if 0

	ret = usb_signal_init(dev, rf_info, USB_FLASH);
	
	if (ret){
		printf("USB INIT FAIL\n");
		goto EXIT;
	}

        ret = run_command("usb dev 0", 0);

        if (ret){
                printf("USB STORAGE NOT FOUND\n");
                goto EXIT;
        }

iXIT:
	
	if (ret){
		printf("USB STORAGE NOT FOUND\n");
		goto EXIT;
	}
#endif
	ret = do_usb_detect_test(&sys_info);

	if (ret)
		printf("USB StorageRead/Write Test..FAIL\n");
	else
		printf("USB StorageRead/Write Test..OK\n");
//EXIT:
	return;
}

void diag_do_gps_test(void)
{
	int ret = 0;

	ret = do_gps_test(&sys_info, rf_info);

	if (ret)
		printf("GPS Test..FAIL\n");
	else
		printf("GPS Test..OK\n");

	return;
}


void diag_do_sim_card_test(void)
{

//	struct usb_device *dev = NULL;
	int ret = 0;
        //unsigned int sim_side;
        __maybe_unused unsigned long success = 0 ;
        __maybe_unused unsigned long fail = 0 ;

	//if(diag_get_value("Set A or B side (A:0 B:1)", &sim_side, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
	//	return;


        ret = diag_do_check_sim_card(&sys_info, rf_info, 0);

                if (ret){
                        printf("SIM Test..FAIL\n");
                        //diag_set_env_conf_u32("cellualr_Failed", ++ fail);
                }else{
                        printf("SIM Test..OK\n");
                        //diag_set_env_conf_u32("cellualr_Success", ++ success);
                }
	return;
}

int diag_do_cellular_for_8960_test(u8 port)
{
	int ret = 0;

	ret = diag_do_cellular_for_rf_test(&sys_info, rf_info, port);
	
/*	if (ret)
		printf("CELLULAR Test..FAIL\n");
	else
		printf("CELLULAR Test..OK\n");*/

//EXIT:
	return ret;

}

void diag_do_RF_debugport_test(void)
{	
	diag_do_cellular_for_8960_test(0);
}

void diag_do_RF_com1_test(void)
{
	diag_do_cellular_for_8960_test(0);
}


int sys_board_init(void) {

	int ret = 0;

	ret = board_info_init(&sys_info);
	
	if (ret)
		printf("Board init Fail\n");

	return ret;
}


burnin_cmd usb_cmd_value[] = {
	{0, '0', "USB Host test",diag_do_usb_detect_test, 1},
	{1, '1', "Cellular Command", diag_do_RF_debugport_test, 1},
	{1, '1', "SIM card Test", diag_do_sim_card_test, 1},
	{1, '1', "GPS Test", diag_do_gps_test, 0},
	{1, '1', "Cellular for 8960 Test", diag_do_RF_com1_test, 1},
	{-1,'*', "",0,0}
};

burnin_cmd usb_cmd_value_without_cellular[] = {
	{0, '0', "USB Host test",diag_do_usb_detect_test, 1},
	{-1,'*', "",0,0}
};

void diag_do_usb_host_func(void)
{
        burnin_cmd *burnin_temp = usb_cmd_value_without_cellular;

        burnin_temp = usb_cmd_value;

	bios_manual_testing(burnin_temp);

}

/* ----- USB Test - END ----- */

void diag_do_sw_button_test(void)
{    
	printf("\r\n\r\n*** Please press SW Button to close test\r\n");

	Wait_SW_Button();
	return;
}

void diag_do_pca9535_set_direction(void)
{
	//unsigned int i2c_bus;
	unsigned int addr;
	unsigned int gpio_bit;
	unsigned int gpio_dir;

	//if(diag_get_value("Select I2C Bus: ", &i2c_bus, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
	//	return;

	if(diag_get_value("Set PCA953x Addr (0:0x20, 1:0x21): ", &addr, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

	if(diag_get_value("Set Bit: ", &gpio_bit, 0, 15, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

        if(diag_get_value("Set Direction (0:OUT, 1:IN): ", &gpio_dir, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

	do_pca9535_set_direction(2, addr + PCA953X_ADDR1, gpio_bit, gpio_dir);
	
	return;
}

void diag_do_pca9535_set_value (void)
{
	//unsigned int i2c_bus;
	unsigned int addr;
	unsigned int gpio_bit;
	unsigned int gpio_val;

	//if(diag_get_value("Select I2C Bus: ", &i2c_bus, 2, 2, DIAG_GET_VALUE_DEC_MODE) != 0)
	//	return;

	if(diag_get_value("Set PCA953x Addr (0:0x20, 1:0x21): ", &addr, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

	if(diag_get_value("Set Bit : ", &gpio_bit, 0, 15, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

	if(diag_get_value("Set Value: ", &gpio_val, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

	do_pca9535_set_value(2, addr + PCA953X_ADDR1, gpio_bit, gpio_val);

	return;
}

void diag_do_pca9535_get_value(void)
{
	//unsigned int i2c_bus;
	unsigned int addr;
	int gpio_val;

	//if(diag_get_value("Select I2C Bus: ", &i2c_bus, 2, 2, DIAG_GET_VALUE_DEC_MODE) != 0)
	//	return;
	
        if(diag_get_value("Set PCA953x Addr (0:0x20, 1:0x21): ", &addr, 0, 1, DIAG_GET_VALUE_DEC_MODE) != 0)
		return;

	gpio_val = do_pca9535_get_value (2, addr + PCA953X_ADDR1);

	printf ("Value: %04X\n", gpio_val);

	return;
}

void diag_do_dio_do_test(void)
{
	unsigned int value;

	if(diag_get_value("Set DO Value", &value, 0, 0xF, DIAG_GET_VALUE_HEX_MODE) != 0)
		return;

	I2C_SET_BUS(0);

	pca953x_set_val(PCA953X_ADDR1, 0x00F0, (value << 4));

	return;
}

__maybe_unused static void diag_do_SD_power_control_loop(void)
{
        int i ; 
        int delay = 3;
        uint8_t gpio_val = 0;

	puts(ESCString);
        while (1){
		if (check_if_press_character(ESC)) {
	                //do_user_led_func(0);
			return;
		}		

                if (gpio_val){
                        printf("turn on SD POWER\n");
	                do_pca9535_set_value(2, 0x21, 3, 1);
                } else {
                        printf("turn off SD POWER\n");
	                do_pca9535_set_value(2, 0x21, 3, 0);
	        }
	        gpio_val = ~gpio_val;

                for (i = 0; i < delay; i++)
                        udelay(1000 * 1000); 

        }
}
#if CONFIG_MOXA_DIO
void diag_do_dio_test(void)
{
        do_dio_test();
}
#endif
void diag_do_hw_ver_main (void)
{
	return;
}

void diag_do_hw_ver_io (void)
{
	return;
}

burnin_cmd io_cmd_value[] = {
	{1, 1, "SW Button & Beeper test", diag_do_sw_button_test, 1},
	{2, 2, "PCA9535 Set Direction", diag_do_pca9535_set_direction, 1},
	{3, 3, "PCA9535 Set Value", diag_do_pca9535_set_value, 1},
	{4, 4, "PCA9535 Get Value", diag_do_pca9535_get_value, 1},
#if CONFIG_MOXA_DIO
        {5, 5, "CPU DI/O test" , diag_do_dio_test, 1},
#endif
//      {6, 6, "SD power loop", diag_do_SD_power_control_loop, 1},
	{-1,'*', "",0,0}
};

void diag_do_io_func(void)
{
	burnin_cmd *burnin_temp = io_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- IO TEST - END ----- */

/* ----- MEMORY/STORAGE MENU - START ----- */
/**	\brief
 *
 *	Test RAM with 8 bits pattern
 *
 * 	\param[in]	address : test address
 * 	\param[in]	length : test length
 * 	\retval		0 : OK, 1 : Fail, 2 : Escape
 *
 */
int ram_test_byte(unsigned long address, unsigned long length)
{
	int ret = 0;

	ret = ram_test_for_pattern(address, length, BIT8_MODE);

	return ret;
}

/**	\brief
 *
 *	Test RAM with 16 bits pattern
 *
 * 	\param[in]	address : test address
 * 	\param[in]	length : test length
 * 	\retval		0 : OK, 1 : Fail, 2 : Escape
 *
 */
int ram_test_short(unsigned long address, unsigned long length)
{
	int ret = 0;

	ret = ram_test_for_pattern(address, length, BIT16_MODE);

	return ret;
}

/**	\brief
 *
 *	Test RAM with 32 bits pattern
 *
 * 	\param[in]	address : test address
 * 	\param[in]	length : test length
 * 	\retval		0 : OK, 1 : Fail, 2 : Escape
 *
 */
int ram_test_long(unsigned long address, unsigned long length)
{
	int ret = 0;

	ret = ram_test_for_pattern(address, length, BIT32_MODE);

	return ret;
}


/**	\brief
 *
 *	Test RAM with 32 biits pattern, the pattern is the same as the address[param in]
 *
 * 	\param[in]	address : test address
 * 	\param[in]	length : test length
 * 	\param[in]	mode : test mode 1: byte,  2: short, 4: long
 * 	\retval		0 : OK, 1 : Fail, 2 : Escape
 *
 */
int do_ram_test_for_address(unsigned long address, unsigned long length, unsigned int mode)
{
	int ret = 0;

	ret = ram_test_for_address(address, length, mode);

	return ret;
}

void diag_do_byte_test(void)
{
	unsigned int length = 0, ret;

	printf("Start Address = 0x%.8x \r\n" , CONFIG_SYS_MEMTEST_START);

	if (diag_get_value("Test Length : ", (unsigned int*)&length, 0x0, (CONFIG_SYS_SDRAM_BASE +gd->ram_size \
				- CONFIG_SYS_MEMTEST_START - CONFIG_SYS_MEMTEST_RESERVED), DIAG_GET_VALUE_HEX_MODE)){return ;}

	printf("End Address = 0x%.8x \r\n" , CONFIG_SYS_MEMTEST_START + length);
	
	if ((ret = ram_test_byte(CONFIG_SYS_MEMTEST_START, length)) != 0) {
		if (ret == 2)
			printf("*** SDRAM BYTE(8-bit) TEST Abort!\r\n");
		else
			printf("*** SDRAM BYTE(8-bit) TEST FAIL!!\r\n");
		
	} else {
		printf("SDRAM BYTE(8-bit) TEST PASS.\r\n");
	}
}

void diag_do_short_test(void)
{
	unsigned int length = 0, ret;

	printf("Start Address = 0x%.8x \r\n" , CONFIG_SYS_MEMTEST_START);

	if (diag_get_value("Test Length : ", (unsigned int*)&length, 0x0, (CONFIG_SYS_SDRAM_BASE +gd->ram_size \
				- CONFIG_SYS_MEMTEST_START - CONFIG_SYS_MEMTEST_RESERVED), DIAG_GET_VALUE_HEX_MODE)){return ;}

	printf("End Address = 0x%.8x \r\n" , CONFIG_SYS_MEMTEST_START+length);


	if ((ret = ram_test_short(CONFIG_SYS_MEMTEST_START, length)) != 0) {
		if (ret == 2)
			printf("*** SDRAM SHORT(16-bit) TEST Abort!\r\n");
		else
			printf("*** SDRAM SHORT(16-bit) TEST FAIL!!\r\n");
		
	} else {
		printf("SDRAM SHORT(16-bit) TEST PASS.\r\n");
	}
}

void diag_do_long_test(void)
{
	unsigned int length = 0, ret;

	printf("Start Address = 0x%.8x \r\n" , CONFIG_SYS_MEMTEST_START);
	
	if (diag_get_value("Test Length : ", (unsigned int*)&length, 0x0, (CONFIG_SYS_SDRAM_BASE +gd->ram_size \
				- CONFIG_SYS_MEMTEST_START - CONFIG_SYS_MEMTEST_RESERVED), DIAG_GET_VALUE_HEX_MODE)){return ;}

	printf("End Address = 0x%.8x \r\n" , CONFIG_SYS_MEMTEST_START+length);

	if ((ret = ram_test_long(CONFIG_SYS_MEMTEST_START, length)) != 0) {
		if (ret == 2)
			printf("*** SDRAM LONG(32-bit) TEST Abort!\r\n");
		else
			printf("*** SDRAM LONG(32-bit) TEST FAIL!!\r\n");
		
	} else {
		printf("SDRAM LONG(32-bit) TEST PASS.\r\n");
	}
}

void diag_do_address_test(void)
{
	unsigned int length = 0, ret;

	printf("Start Address = 0x%.8x \r\n", CONFIG_SYS_MEMTEST_START);

	if (diag_get_value("Test Length : ", (unsigned int*)&length, 0x0, (CONFIG_SYS_SDRAM_BASE +gd->ram_size\
			 - CONFIG_SYS_MEMTEST_START - CONFIG_SYS_MEMTEST_RESERVED), DIAG_GET_VALUE_HEX_MODE)){return ;}

	printf("End Address = 0x%.8x \r\n", CONFIG_SYS_MEMTEST_START + length);

	ret = do_address_test(CONFIG_SYS_MEMTEST_START, CONFIG_SYS_MEMTEST_START + length);

	if (ret)
		printf("*** SDRAM TEST FAIL!\r\n");
	else
		printf("*** SDRAM TEST PASS!\r\n");

	return;

}

burnin_cmd sdram_cmd_value[] = {
	{0, '0', "Byte Test", diag_do_byte_test, 1},
	{1, '1', "Short Test", diag_do_short_test, 1},
	{2, '2', "Long Test", diag_do_long_test, 1},
	{3, '3', "Address Test", diag_do_address_test, 1},
	{-1,'*', "",0,0}
};

void diag_do_sdram_func(void)
{
	burnin_cmd *burnin_temp = sdram_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- DRAM MENU - END ----- */

burnin_cmd memory_storage_cmd_value[] = {
	{0, '0',"DRAM Test", diag_do_sdram_func, BIOS_DARAM_TEST},
	{-1, '*',"",0,0}
};

void diag_do_memory_storage_func(void)
{
	burnin_cmd *burnin_temp = memory_storage_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- MEMORY/STORAGE MENU - END ----- */


__maybe_unused void count_tester (void)
{
        unsigned long counter;
	char *s;

	s = getenv("tester");

	if (NULL != s) {
		counter = (int)simple_strtol(s, NULL, 10);
        } else {  
                counter = 0 ;
        }
        
        counter ++ ;
	s = getenv("boot_counter"); 

        printf("boot     : %s times\n", s);
        printf("tester : %ld times\n", counter);
	diag_set_env_conf_u32("tester", counter); 
}

__maybe_unused void boot_counter (void)
{
        unsigned long counter;
	char *s;

	s = getenv("boot_counter");

	if (NULL != s) {
		counter = (int)simple_strtol(s, NULL, 10);
        } else {  
                counter = 0 ;
        }
        
        counter ++ ;
        printf("boot_counter : %ld times\n", counter);
	diag_set_env_conf_u32("boot_counter", counter); 
}

int tpm2_cfg(void)
{
	int ret = -1;
	char res[4096] = {0};
	int value = 0;
	char *s;
	int enable = 0;

	s = getenv("en_tpm2");

	if (NULL != s) {
		enable = (int)simple_strtol(s, NULL, 10);
	} else {
		diag_set_env_conf_u32("en_tpm2", 1); 
		enable = 1; 
	}

	if (enable == 0) {

		if (tpm2_init() != 0){
			return ret;
		}

		udelay(100 * 1000);

		if (tpm2_startup(TPM2_SU_CLEAR) != 0){
			printf("TPM2 Startup Fail![%d]\n",ret);
			return ret;
		}

		udelay(100 * 1000);


		if ((tpm2_hierarchy(TPM2_OWNER, 0)) != 0){
			printf("Failed to disable hierarchy(TPM2_OWNER)![%d]\n",ret); 
			return ret;
		}

		udelay(100 * 1000);

		if ((tpm2_hierarchy(TPM2_ENDORSEMENT, 0)) != 0){
			printf("Failed to disable hierarchy(TPM2_ENDORSEMENT)![%d]\n",ret); 
			return ret;
		}

		udelay(100 * 1000);

		if ((tpm2_hierarchy(TPM2_PLATFORM, 0)) != 0){
			printf("Failed to disable hierarchy(TPM2_PLATFORM)![%d]\n",ret); 
			return ret;
		}

		tpm2_get_capability(TPM2_PROPERTIES_VARIABLE, res);
	
		value = buf_to_u32(res + 31);

		
		if (value & (PHENABLE | SHENABLE | EHENABLE)) {
			printf("Failed to disable hierarchy item\n"); //R95
			return -1;
		}
	}
	return 0;
}
/* ----- TPM ENABLED/DISABLED SETTING - START ----- */

void diag_do_tpm2_enable_func(int enable) 
{

	char *s;
	char str[128] = {0};
	int state = 0;

	s = getenv("en_tpm2");

	if (NULL != s) {
		state = (int)simple_strtol(s, NULL, 10);
	} else {
		diag_set_env_conf_u32("en_tpm2", 1);	
		state = 1;	
	}

	if (enable)
		printf("Current State: Enable\n");
	else
		printf("Current State: Disable\n");

	if (enable != state) {
		sprintf(str, "setenv en_tpm2 %d", enable);
		run_command(str, 0);
		run_command("saveenv", 0);
	}

	printf("\r\n### Please reboot the system to complete the operation ###\n");
	
	while(1);

	return;
}

void diag_do_set_tpm2_enabled_func(void)
{
	diag_do_tpm2_enable_func(1);
}

void diag_do_set_tpm2_disabled_func(void)
{
	diag_do_tpm2_enable_func(0);
}

void diag_do_tpm2_init_func(void)
{
	tpm2_init();
}

void diag_do_tpm2_startup_func(void)
{
	int ret = 0;

   	if ((ret = tpm2_startup(TPM2_SU_CLEAR)) != 0){
		printf("TPM2 Startup TPM2_SU_CLEAR Fail![%d]\n", ret);
		return;
	} else {
		printf("TPM2 Startup TPM2_SU_CLEAR OK!\n");
	}
}

void diag_do_tpm2_force_clear_func(void)
{
	tpm2_force_clear();

	return;
}

void diag_do_tpm2_get_capability_func(void)
{
	int ret = 0;

	if ((ret = run_command("tpm2 get_capability properties-fixed", 0)) != 0) {
		printf("TPM Get Capability info(properties-fixed) Fail! [%d]\n",ret);
		return;
	} else {
		printf("TPM Get Capability info(properties-fixed) OK!\n");
	}

	if ((ret = run_command("tpm2 get_capability properties-variable", 0)) != 0){
		printf("TPM Get Capability info(properties-variable) Fail! [%d]\n",ret);
		return;
	} else {
		printf("TPM Get Capability info(properties-variable) OK!\n");
	}

	return;
}

burnin_cmd tpm2_cmd_value[] = {
	{ 1,1, "Enable TPM2", diag_do_set_tpm2_enabled_func, 1},
	{ 1,1, "Disable TPM2", diag_do_set_tpm2_disabled_func, 1},
	{ 1,1, "Init TPM2", diag_do_tpm2_init_func, 1},
	{ 1,1, "Startup TPM2", diag_do_tpm2_startup_func, 1},
	{ 1,1, "Force Clear TPM2", diag_do_tpm2_force_clear_func, 1},
	{ 1,1, "Get Capability of TPM2", diag_do_tpm2_get_capability_func, 1},
	{ -1,'*',"",0,0}
};
void diag_do_tpm2_func(void)
{
        char *s;
        int valid = 0;

        s = getenv("tpm2");

        if (NULL != s) {
                valid = (int)simple_strtol(s, NULL, 10);
        } else {
                diag_set_env_conf_u32("tpm2", 0);
                valid = 0;
        }

        if(valid == 1){
                burnin_cmd *burnin_temp = tpm2_cmd_value;
                bios_manual_testing(burnin_temp);
        } else {
                printf("\r\n***This module does not support TPM2 features***\n");
        }
}

burnin_cmd tpm2_config_cmd_value[] = {
	{ 0,'0',"Enable TPM2", diag_do_set_tpm2_enabled_func, 1},
	{ 1,'1',"Disable TPM2", diag_do_set_tpm2_disabled_func, 1},
	{ -1,'*',"",0,0}
};

void diag_do_tpm2_config(void)
{

	char *s;
	int valid = 0;

	s = getenv("tpm2");

	if (NULL != s) {
		valid = (int)simple_strtol(s, NULL, 10);
	} else {
		diag_set_env_conf_u32("tpm2", 0);
		valid = 0;
	}

	if(valid == 1){
		burnin_cmd *burnin_temp = tpm2_config_cmd_value;
		bios_manual_config(burnin_temp);
	} else {
		printf("\r\n***This module does not support TPM2 features***\n");
	}
}

void diag_do_card_detect_scan(void)
{
	int mmc_num;

	for (mmc_num = 1; mmc_num < MMC_MAX_DEVICE - 1; mmc_num++) {
		printf("MMC%d:\n", mmc_num);
		do_mmc_card_detect(mmc_num, 1);
	}
}


int diag_do_sd_protected_func(int sd_num)
{

	unsigned int sd_protected;
	unsigned int set_or_clean;
	unsigned int tmp;

	if (sd_num == 0)
		printf("\nCurrent Boot Storage Write Protect is ");
	else
	   	printf("\nCurrent Extend Storage Write Protect is ");
	
	sd_protected = diag_get_env_conf_u32("sd_protected");

	if (sd_num == 0)
		tmp = sd_protected & boot_SD_Protected;
	else
		tmp = sd_protected & extend_SD_Protected;

	if (tmp)
		printf("Enabled.\n");
	else
		printf("Disabled.\n");

	if(diag_get_value("Change to , 0 - Disabled, 1 - Enabled", &set_or_clean, \
						0, 1, DIAG_GET_VALUE_DEC_MODE))
		return -1;

	if (sd_num == 0)
		tmp = boot_SD_Protected;
	else
		tmp = extend_SD_Protected;
	
	if (set_or_clean)
		sd_protected |= boot_SD_Protected;
	else
		sd_protected &= ~boot_SD_Protected;

	diag_set_env_conf_u32("sd_protected", sd_protected);

	return 0;
}



void diag_do_sd0_protected_func(void)
{
	diag_do_sd_protected_func(0);	
}

void diag_do_sd1_protected_func(void)
{
	diag_do_sd_protected_func(1);
}

burnin_cmd sd_protected_cmd_value[] = {
	{1, '1', "Boot Storage Write Protect", diag_do_sd0_protected_func, 1},
	{1, '1', "Extend Storage Write Protect", diag_do_sd1_protected_func, 1},
   	{-1, '*', "", 0, 0}
};

void diag_do_sd_protected_config(void)
{
	burnin_cmd *burnin_temp = sd_protected_cmd_value;
	bios_manual_testing(burnin_temp);
}

void do_sd_rw_func(unsigned int sd_num, unsigned int part)
{
	int ret = 0;

	ret = sd_rw_func(sd_num, part, MMC_TEST_PATTERN1);
	
	if (ret)
		printf("MMC%d Read/Write testing ** FAIL **.(0x%x) \r\n", sd_num, MMC_TEST_PATTERN1);

	ret |= sd_rw_func(sd_num, part, MMC_TEST_PATTERN2);	

	if (ret)
		printf("MMC%d Read/Write testing ** FAIL **.(0x%x) \r\n", sd_num, MMC_TEST_PATTERN2);

	return;
}

void diag_do_sd0_rw_func(void)
{
	do_sd_rw_func(MMC0, 1);
	return;
}

void diag_do_sd1_rw_func(void)
{
	do_sd_rw_func(MMC1, 1);
	return;
}

void diag_do_sd2_rw_func(void)
{
	do_sd_rw_func(MMC2, 1);
	return;
}

void diag_do_cd_wp_pin_status(void)
{

	int val;
	int ret = 0;

	unsigned int mmc;
	
	if (diag_get_value("\nMMC Control number:", &mmc, 0, 2, DIAG_GET_VALUE_DEC_MODE))
		return;

	ret = cd_wp_pin_status(mmc, &val);

	if (ret) {
		printf("MMC%d: Check Status Fail.\n", mmc);
		goto EXIT;
	}

	if(val & BIT18) {
		printf("MMC%d: Card is inserted.\n", mmc);	

		if(val & BIT19)
			printf("MMC%d: Card is write protect.\n", mmc);	
		else
			printf("MMC%d: Card not write protect.\n", mmc);				
	} else {
		printf("MMC%d: Card not inserted.\n", mmc);			
	}

EXIT:
	return;
}

burnin_cmd sd_cmd_value[] = {
	{0, '0', "MMC1: SD Read/Write Test", diag_do_sd1_rw_func, 1},
	{1, '1', "MMC Card Detect Chcek", diag_do_card_detect_scan, 1},
	{2, '2', "MMC CD & WP Pin Status", diag_do_cd_wp_pin_status, 1},
    	{-1, '*', "", 0, 0}
};

void diag_do_sd_func(void)
{
	burnin_cmd *burnin_temp = sd_cmd_value;
	bios_manual_testing(burnin_temp);

}
/* ----- SD READ/WRITE TEST - END ----- */
/* ----- LED TEST - START ----- */
void diag_do_sd_led_func(void)
{
	unsigned int set_or_clean;

	if (diag_get_value("\nSD LED Set to 0 - OFF, 1 - ON", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;
	
	do_sd_led_func(set_or_clean);

	return;
}

void diag_do_zig_led_func(void)
{
	unsigned int set_or_clean;
	
	if (diag_get_value("\nZigbee LED Set to 0 - OFF, 1 - ON", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_zig_led_func(set_or_clean);
	
	return;
}

void diag_do_usb_plug_led_func(void)
{
	unsigned int set_or_clean;

	if (diag_get_value("\nUSB PLUG LED Set to 0 - OFF, 1 - ON", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_usb_plug_led_func(set_or_clean);
	
	return;

}

void diag_do_cellular_led_func(void)
{
	unsigned int set_or_clean;
	
	if (diag_get_value("\nCellular LED Set to 0~7", &set_or_clean, 0, 7, DIAG_GET_VALUE_DEC_MODE))
		return;
	
	do_cellular_led_func(set_or_clean);
	
	return;
}

void diag_do_diagnostic_led_func(void)
{	
	unsigned int set_or_clean;

	if (diag_get_value("\nDiagnostic Set to 0~7", &set_or_clean, 0, 7, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_diagnostic_led_func(set_or_clean);

	return;
}

void diag_do_user_led_func(void)
{
	unsigned int set_or_clean = 1;
	int i = 0;
	/*if (diag_get_value("\nUser LED Set to 0~7", &set_or_clean, 0, 7, DIAG_GET_VALUE_DEC_MODE))
		return;*/
	
	puts(ESCString);
        while(1){
		if (check_if_press_character(ESC)) {
	                do_user_led_func(0);
			return;
		}		

	        do_user_led_func(set_or_clean << i);
                i ++;
                i = i % 3;
                udelay (300*1000);
        }
	return;
}

void diag_do_debug_led_func(void)
{
	unsigned int set_or_clean = 1;
	int i = 0;
	/*if (diag_get_value("\nDEBUG LED Set to 0~15", &set_or_clean, 0, 15, DIAG_GET_VALUE_DEC_MODE))
		return;*/
	
	do_debug_led_func(0);
	puts(ESCString);
        while(1){
		if (check_if_press_character(ESC)) {
                        do_debug_led_func(0);
			return;
		}		

	        do_debug_led_func(set_or_clean << i);
                i ++;
                i = i % 5;
                udelay (300*1000);
        }
	return;
}

void diag_do_full_led_func(void)
{
	LED_MPTEST(0);
}

burnin_cmd led_cmd_value[] = {
	{1, 1, "SD LED Test", diag_do_sd_led_func, SD_LED},
	{1, 1, "Zigbee LED Test", diag_do_zig_led_func, Zigbee_LED},
	{1, 1, "USB PLUG LED Test", diag_do_usb_plug_led_func, USB_PLUG_LED},
	{1, 1, "Cellular LED Test", diag_do_cellular_led_func, CELLULAR_LED},
	{1, 1, "Diagnostic LED Test", diag_do_diagnostic_led_func, DIAGNOSTIC_LED},
	{1, 1, "User LED Test", diag_do_user_led_func, 0},
	{1, 1, "Debug LED Test", diag_do_debug_led_func, 1},
	{1, 1, "Full LED Test", diag_do_full_led_func, 1},
   	{-1, '*', "", 0, 0}
};

void diag_do_led_func(void)
{
	burnin_cmd *burnin_temp = led_cmd_value;
	bios_manual_testing(burnin_temp);

}
/* ----- LED TEST - END ----- */

/* ----- WDT TEST - START ----- */
void diag_do_wdt_test (void)
{
	unsigned int timeout;

	if (diag_get_value ("WDT Timeout :", &timeout, 1, 10, DIAG_GET_VALUE_DEC_MODE))
		return;

	do_wdt_test(timeout);

	while (timeout) {
		printf ("%d...", timeout--);
		udelay(1000 * 1000);
	}
}

burnin_cmd wdt_cmd_value[] = {
	{1, 1, "WDT Reset Test", diag_do_wdt_test, 1},
   	{-1, '*', "", 0, 0}
};

void diag_do_wdt_func (void)
{
	burnin_cmd *burnin_temp = wdt_cmd_value;
	bios_manual_testing (burnin_temp);
}
/* ----- WDT TEST - END ----- */


void diag_do_check_backup_env (void)
{
	int ret = 0;

	ret = backup_env_relocate(0);

	if (ret == 0)
		printf("check_backup_env ok\n");
	else
		printf("check_backup_env failed\n");

	return;
}

void diag_do_import_backup_env (void)
{
	int ret = 0;

	ret = backup_env_relocate(1);

	if (ret == 0)
		printf("import_backup_env ok\n");
	else
		printf("import_backup_env failed\n");

	return;
}

void diag_do_save_backup_env (void)
{
	int ret = 0;

	ret = save_backup_env();

	if (ret == 0)
		printf("save_backup_env ok\n");
	else
		printf("save_backup_env failed\n");

	return;
}

void diag_do_clear_backup_env (void)
{
	int ret = 0;

	ret = clear_backup_env();

	if (ret == 0)
		printf("clear_backup_env ok\n");
	else
		printf("clear_backup_env failed\n");

	return;
}


void diag_do_recovery_env_from_backup (void)
{
	int ret = 0;

	ret = user_env_recovery();

	if (ret == 0)
		printf("recovery_env_from_backup ok\n");
	else
		printf("recovery_env_from_backup failed\n");

	return;
}

burnin_cmd backup_env_cmd_value[] = {
	{1, 1, "Clear backup env", diag_do_clear_backup_env, 1},
	{1, 1, "Save backup env", diag_do_save_backup_env, 1},
	{1, 1, "Import backup env", diag_do_import_backup_env, 1},
	{1, 1, "check backup env", diag_do_check_backup_env, 1},
	{1, 1, "recovery env from backup env", diag_do_recovery_env_from_backup, 1},
   	{-1, '*', "", 0, 0}
};

void diag_do_set_clear_backup_env_func (void)
{
	burnin_cmd *burnin_temp = backup_env_cmd_value;
	bios_manual_testing (burnin_temp);
}

/* ----- QA MENU - START ----- */
void diag_do_qa_test_func(void)
{
	WDT_MODE = 1;
	MP_PROGRAM(FG_MODE);
	WDT_MODE = 0;

	return;
}
/* ----- QA MENU - END ----- */

/* ----- Run Kernel - START ----- */
void diag_do_run_mmc0_func(void)
{
	int ret = 0;

	if (sys_info.mmc0 == -1) {
		printf(" No Support MMC0\n");
		return;
	}

	ret = do_run_mmc0_func(&sys_info);

	if (ret)
		printf("Failed to boot from mmc0\n");

	return;

}

void diag_do_run_mmc1_mp_func(void)
{
        int ret = 0;
        ret = do_run_mmc1_mp_func(&sys_info);

	if (ret)
		printf("Failed to boot from SD\n");

	return;
}

void diag_do_run_mmc1_func(void)
{
	int ret = 0;

	if (sys_info.mmc1 == -1) {
		printf(" No Support MMC1\n");
		return;
	}

	ret = do_run_mmc1_func(&sys_info);

	if (ret)
		printf("Failed to boot from mmc1\n");

	return;
}

void diag_do_run_mmc2_func(void)
{
	int ret = 0;

	if (sys_info.mmc2 == -1) {
		printf(" No Support MMC2\n");
		return;
	}

	ret = do_run_mmc2_func(&sys_info);

	if (ret)
		printf("Failed to boot from mmc2\n");

	return;
}

void diag_do_run_mmc_func(void)
{
	printf ("Boot to MMC0: eMMC...\n");
	do_run_mmc_boot(&sys_info);
}

burnin_cmd run_kernel_cmd_value[] = {
	{0, '0', "Run Kernel from eMMC", diag_do_run_mmc0_func, 1},
	{1, '1', "Run Kernel from SD", diag_do_run_mmc1_func, 1}, //R41
	{2, '2', "Run Kernel Normal Release", diag_do_run_mmc_func,1},
	{3, '3', "Run Kernel from USB", diag_do_nothing,0},
        {4, '4', "Run MP firmware", diag_do_run_mmc1_mp_func, 0},
	{-1, '*', "",0,0}
};

void diag_do_run_kernel_func(void)
{
	burnin_cmd *burnin_temp = run_kernel_cmd_value;
	bios_manual_testing(burnin_temp);
}
/* ----- Run Kernel - END ----- */

/*
 * MP Test subroutine
*/
void MP_PROGRAM(int mode)
{
	int i = 0;
        char *s;

	printf("Ready to Run T1\r\n");

//	uart_mode_default();

	for (i = 0; i < 32; i++){
		switch(i){

#ifdef	CPU_MP
                case CPU_MP:
			if (mode != FG_MODE) {
				ShowLED(CPU_MP);
				CPU_MPTEST(CPU_MP, sys_info.modeltype);
			}
                        
                        break;
#endif
#ifdef	RTC_MP
		case RTC_MP:
			if (mode != FG_MODE) {
				ShowLED(RTC_MP);
				RTC_MPTEST(RTC_MP);
			}
			break;
#endif
#ifdef	SDRAM_MP
		case SDRAM_MP:
			ShowLED(SDRAM_MP);
			SDRAM_MPTEST(SDRAM_MP);
			break;
#endif

#ifdef	LAN_10M_CROSSCABLE_MP
		case LAN_10M_CROSSCABLE_MP:
			ShowLED(LAN_10M_CROSSCABLE_MP);
				
			#ifdef	LAN1_10M
			LAN_10M_CROSSCABLE_MPTEST(LAN_10M_CROSSCABLE_MP,0);
			#endif  //LAN1_10M
				
			#ifdef	LAN2_10M
			LAN_10M_CROSSCABLE_MPTEST(LAN_10M_CROSSCABLE_MP,1);
			#endif  //LAN2_10M
			break;
#endif	//LAN_10M_CROSSCABLE_MP

#ifdef	LAN_100M_CROSSCABLE_MP
		case LAN_100M_CROSSCABLE_MP:
			ShowLED(LAN_100M_CROSSCABLE_MP);
				
			#ifdef	LAN1_100M
			LAN_100M_CROSSCABLE_MPTEST(LAN_100M_CROSSCABLE_MP,0);
			#endif  //LAN1_100M
				
			#ifdef	LAN2_100M
			LAN_100M_CROSSCABLE_MPTEST(LAN_100M_CROSSCABLE_MP,1);
			#endif  //LAN2_100M
			break;
#endif	//LAN_100M_CROSSCABLE_MP

//#ifdef	TPM_MP
#if	0
		case TPM_MP:
			if((Check_CPU_Type(0) == 4) || (Check_CPU_Type(0) == 5)){
				ShowLED(TPM_MP);
				TPM_MPTEST(TPM_MP);
			}

			break;
#endif	//TPM_MP

#ifdef	TPM2_MP
		case TPM2_MP:
			ShowLED(TPM2_MP);

			s = getenv("tpm2");

                        if (NULL != s) {
                                if (strcmp(s, "1") == 0)
                                        TPM2_MPTEST(TPM2_MP);
                        }


			break;
#endif	//TPM_MP

#ifdef	SD_MP
		case SD_MP:
			//ShowLED(SD_MP);				
			#ifdef	SD_0				
			SD_MPTEST(SD_MP, 0);
			#endif	//SD_0

			#ifdef	SD_1
			SD_MPTEST(SD_MP, 1);
			#endif	//SD_1
			break;
#endif	//SD_MP
#ifdef	USBH_MP
		case USBH_MP:
		        printf ("DO USB MP TEST \n\n\n");
			ShowLED(USBH_MP);
			USB_HOST_MPTEST(USBH_MP);
			break;
#endif	//USBH_MP
#ifdef	LED_MP
		case LED_MP:
			ShowLED(LED_MP);
			LED_MPTEST(LED_MP);
			break;
#endif	//LED_MP
#if	RS232_MP
		case RS232_MP:
			ShowLED(RS232_MP);
			UART_MPTEST(RS232_MP, i);

			break;
#endif	//RS232_MP
#if	RS422_MP
		case RS422_MP:
			ShowLED(RS422_MP);				
			UART_MPTEST(RS422_MP, i);

			break;
#endif	//RS422_MP
#if	RS4852W_MP
		case RS4852W_MP:
			ShowLED(RS4852W_MP);				
			UART_MPTEST(RS4852W_MP, i);

			break;
#endif	//RS4852W_MP
#ifdef	USB_CELLULAR_MP
		case USB_CELLULAR_MP:
			if (mode != FG_MODE){
					ShowLED(USB_CELLULAR_MP);
					USB_CELLULAR_MPTEST(&sys_info, rf_info, USB_CELLULAR_MP);
			}

			break;
#endif	//USB_CELLULAR_MP
#ifdef  WDT_MP
		case WDT_MP:
			if(mode != FG_MODE){
				ShowLED(WDT_MP);
				WDT_MPTEST(WDT_MODE, WDT_MP);
			}
			break;
#endif 
#if CONFIG_MOXA_DIO
                case DIO_MP:
                        ShowLED(DIO_MP);
                        DIO_MPTEST(DIO_MP); 
                        break;
#endif //DIO_MP                        
		default :
		        break;
		}
	}
	
	return;
}

int set_cellular_module_flags(void)
{
	unsigned int set_or_clean = 0;

	printf("\nIf cellular module exist, it will set 1. Conversely, 0 sets for inexistence.\n");

	if (diag_get_value("0 - None, 1 - Exist", &set_or_clean, 0, 1, DIAG_GET_VALUE_DEC_MODE))
		return 1;

	diag_set_env_conf_u32("cell_flags", set_or_clean);

	return 0;
}

int set_gps_test_flag(void)
{
	unsigned long set_or_clean = 1;

	printf("\nIf cellular module exist, set 1 to test GPS later, 0 is not.\n");

	if (diag_get_value("0 - None, 1 - Exist", (unsigned int *) (&set_or_clean), 0, 1, DIAG_GET_VALUE_DEC_MODE))
                return 1;

        diag_set_env_conf_u32("gps_flags", set_or_clean);

        return 0;

}
void Factory_Test_Flow(void)
{
	unsigned int mm_flags;
	__maybe_unused char buf [80] = {0};
	UINT8 data;
	__maybe_unused int ret = 0;
	__maybe_unused int cnt = 0;	
	int env_state = 0;
	int rdyled = PIO_LED_SD;

	env_state = backup_env_relocate(0);

	if (env_state == BACKUP_ENV_OK) {
		if (strcmp(getenv("mm_flags"), "58556") != 0) {
        		diag_set_env_conf_u32("mm_flags", (T1_0_FLAG | T1_1_FLAG | T1_2_FLAG | T1_3_FLAG | \
			T2_FLAG | T3_2_FLAG | T3_3_FLAG | T3_4_FLAG | T3_5_FLAG));
		}
	}

	run_command ("i2c dev 0", 0);

	mm_flags = diag_get_env_conf_u32("mm_flags");
	
	if ((mm_flags & T1_0_FLAG) == 0) {
	        wdt_stop();
		printf("Enter T1(MP) Mode\r\n");

		data = i2c_reg_read (I2C_DS1374_ADDR, DS1374_TOD_CNT_BYTE3_ADDR);

		if (data == ENABLE_WDT_SUCCESS) {
			ShowLED(WDT_MP);
			data = i2c_reg_read(I2C_DS1374_ADDR, DS1374_CTL_ADDR);
			data &= ~DS1374_CTL_BIT_WACE;
			i2c_reg_write(I2C_DS1374_ADDR, DS1374_CTL_ADDR, data);
			i2c_reg_write(I2C_DS1374_ADDR, DS1374_TOD_CNT_BYTE3_ADDR, 0x45);
			printf("\nWDT testing ** OK ** \r\n.");
		} else if (data == ENABLE_WDT_FAILED) {
			halt(WDT_MP, HALT_MODE2);
			return;
		} else {
			WDT_MODE = 0;
			MP_PROGRAM(T1_MODE);
			return;
		}

                printf("into OS for T1 test\n");
                run_OS_test_func("T1(MP)", sys_info.dtbname);
	}




	if ((mm_flags & T1_1_FLAG) == 0) {
	        wdt_stop();
#ifdef RTC_DS1374
                check_ds1374_battery(sys_info.modeltype);
#endif /* RTC_DS1374 */
		printf("Enter T1-1(MAC) Mode\r\n");
		ReadyLed(rdyled,1);
		ShowLED(0);
		GetSN_MAC(1);

		/* NOT GOTO HERE */
		return;
	}
	
	if((mm_flags & T1_2_FLAG) == 0) {
	        wdt_stop();
		printf("Enter T1-2(RF) Mode\r\n");
		ReadyLed(rdyled, 1);

		if(set_cellular_module_flags()){
                        ReadyLed(rdyled, 0);
			printf("Set T1-2(RF) FLAG FAIL...\r\n");
			halt(0, HALT_MODE2);
		}

		diag_set_env_conf_u32("mm_flags", (mm_flags | T1_2_FLAG));
                printf("T1-2(RF) has Completed!\r\n");
		halt(0, HALT_MODE3);	
	}

	if((mm_flags & T1_3_FLAG) == 0){
		wdt_stop();
		printf("Enter T1-3(GPStest) Mode\r\n");
		ReadyLed(rdyled, 1);

		if (set_gps_test_flag()){
                        ReadyLed(rdyled, 0);
			printf("Set T1-3(GPStest) FLAG FAIL...\r\n");
			halt(0, HALT_MODE2);
		}

		diag_set_env_conf_u32("mm_flags", (mm_flags | T1_3_FLAG));
                printf("T1-3(GPStest) has Completed!\r\n");
		halt(0, HALT_MODE3);	
	}

	if((mm_flags & T2_FLAG) == 0){
	        wdt_stop();
	        run_OS_test_func("T2", sys_info.dtbname);
		return;
	}
	
	if((mm_flags & T3_2_FLAG) == 0){
	        wdt_stop();
		printf("Enter T3-2(MN) Setting\r\n");
		ReadyLed(rdyled, 1);
		ShowLED(0);
		GetSN_MAC(3);
	}

        ReadyLed(rdyled, 0);
	ShowLED(0);

	if ((mm_flags & T3_3_FLAG) == 0){ 
	        wdt_stop();
		printf("Enter T3-3(RF) Mode\r\n");
		ReadyLed(rdyled, 1);
		__maybe_unused int cell_flags = diag_get_env_conf_u32("cell_flags");
		ShowLED(CELLULAR_GPS_MP + cnt);

		if (cell_flags) { //&& ((sys_info.modeltype == UC5102) || (sys_info.modeltype == UC5112))){
			ret = 0;
                        //run_OS_test_func("T3-3(RF)", sys_info.dtbname);
			ret = diag_do_cellular_for_8960_test(0);

			if (ret == DIAG_OK) {
				BIOS_SET_TEST_FLAG(T3_3_FLAG);
				flag_set_ok(sys_info.modeltype);
			} else {
				printf("\r\nRF TEST FAIL\r\n");
                                halt( PIO_LED_SD, HALT_MODE3);
			
                        }

		}else{
                        printf("T3-3(RF) has Completed!\r\n");
			printf("\r\nRF TEST OK\r\n");
                        BIOS_SET_TEST_FLAG(T3_3_FLAG);
                        flag_set_ok(sys_info.modeltype);
                }

		ReadyLed(rdyled, 0);
                halt( PIO_LED_SD, HALT_MODE3);
	}


	if ((mm_flags & T3_4_FLAG) == 0 ){
	        wdt_stop();
		__maybe_unused int gps_flags = diag_get_env_conf_u32("gps_flags");

                if (gps_flags == 0){
                        printf("Set T3-4(GPS) FLAG ...\r\n");
                        BIOS_SET_TEST_FLAG(T3_4_FLAG);
                        printf("T3-4(GPS) has Completed!\r\n");
                        flag_set_ok(sys_info.modeltype);
                } else {
			ret = do_gps_test(&sys_info, rf_info);

			if (ret == DIAG_OK) {
				BIOS_SET_TEST_FLAG(T3_4_FLAG);
				flag_set_ok(sys_info.modeltype);
			}
                
                }
		printf("\r\nGPS_TEST_FAIL\r\n");
		ReadyLed(rdyled, 0);
		halt(0, HALT_MODE3);
        }

	if((mm_flags & T3_5_FLAG) == 0) {
	        wdt_stop();
		printf("Enter T3-5(FG) Setting\r\n");
		MP_PROGRAM(FG_MODE);

                run_OS_test_func("T3-5(FG)", sys_info.dtbname);

	}

	if (env_state == BACKUP_ENV_FAIL)
		diag_do_save_backup_env();

	ShowLED(0);

}

// -------------------------------------------- U-Boot Command ------------------------------------------
int do_jump_mm_bios(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])	//R17
{
	burnin_cmd *burnin_temp=burnin_cmd_value;

	ReturnUboot = 0;
	bios_init_global_value();

	while(1){
		if (ReturnUboot)
			break;

		bios_manual_testing(burnin_temp);
	}

	return 1;
}

U_BOOT_CMD(
	mm_bios,     1,     0,      do_jump_mm_bios,
	"mm_bios      - Jump into MM test menu.\n",
	"Type \"mm_bios\" to jump into MM test menu. \n"
);

int do_jump_bios_config(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	burnin_cmd *burnin_temp = BIOS_config_cmd_value;

	ReturnUboot = 0;

	bios_init_global_value();

	while (1) {
		if (ReturnUboot)
			break;

		bios_manual_testing(burnin_temp);
	}

	return 1;
}

U_BOOT_CMD(
	bios,     1,     0,      do_jump_bios_config,
	"bios      - Jump into BIOS config menu.\n",
	"Type \"bios\" to jump into BIOS config menu. \n"
);

