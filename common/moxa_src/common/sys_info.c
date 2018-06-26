/*  Copyright (C) MOXA Inc. All rights reserved.
_
_    This software is distributed under the terms of the
_    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <types.h>
#include <bios.h>                        
#include <model.h>
#include <asm/gpio.h>
#include "mmc.h"
#include "fs.h"
#include "fat.h"
#include <i2c.h>
#include <pca953x.h>
#include <cli.h>
#include "moxa_lib.h"
#include "sys_info.h"
#include "moxa_boot.h"
#include "cmd_bios.h"

#define SYS_INFO_DEBUG		1

int __weak pre_board_init(board_infos *sys_info)
{
        int model;
        char *s;

	gpio_request(GPIO_2_13, "gpmc_ad12");
	gpio_direction_input(GPIO_2_13);
	gpio_request(GPIO_2_14, "gpmc_ad13");
	gpio_direction_input(GPIO_2_14);
	gpio_request(GPIO_2_15, "gpmc_ad14");
	gpio_direction_input(GPIO_2_15);
	gpio_request(GPIO_2_16, "gpmc_ad15");
	gpio_direction_input(GPIO_2_16);

        model = gpio_get_value(GPIO_2_13) | gpio_get_value(GPIO_2_14) << 1 | gpio_get_value(GPIO_2_15) << 2 | gpio_get_value(GPIO_2_16) << 3; 
        
	sys_info->modeltype = model;

	s = getenv("tpm2");	

	if (NULL == s) {
		char hw_ver [MAX_HW_VERSION_SIZE];

		memset(&hw_ver, 0, sizeof(hw_ver));
		do_get_hw_verison(hw_ver);
		if (hw_ver[2] == 0x2)
			setenv ("tpm2", "1");
		else
			setenv ("tpm2", "0");

		run_command("saveenv", 0);
	}

	return 0;
}


static int board_modelinfo_init(board_infos *sys_info) {

	int ret = 0;	

	switch (sys_info->modeltype) {
	case UC8131:
		ret = sprintf((sys_info->modelname), "UC-8131-LX");
		break;
	case UC8132:
		ret = sprintf((sys_info->modelname), "UC-8132-LX");
		break;
	case UC8162:
		ret = sprintf((sys_info->modelname), "UC-8162-LX");
		break;
	case UC8112:
		ret = sprintf((sys_info->modelname), "UC-8112-LX");
		break;
	default:
		ret = sprintf((sys_info->modelname), "Unknow");
		break;
	}

	return (ret ? 0 : -1);
}


static int board_dtbinfo_init(board_infos *sys_info) {

	int ret = 0;

	switch (sys_info->modeltype) {
	case UC8131:
	//	ret = sprintf(sys_info->dtbname, "moxa-uc8100-me.dtb");
	//	break;
	case UC8132:
	//	ret = sprintf(sys_info->dtbname, "moxa-uc8100-me.dtb");
	//	break;
	case UC8162:
	//	ret = sprintf(sys_info->dtbname, "moxa-uc8100-me.dtb");
	//	break;
	case UC8112:
	//	ret = sprintf(sys_info->dtbname, "moxa-uc8100-me.dtb");
	//	break;
	default:
		ret = sprintf(sys_info->dtbname, "am335x-moxa-uc-8100.dtb");
		break;
	}

	return (ret ? 0 : -1);
}


static int board_mmcinfo_init(board_infos *sys_info) 
{

	int ret = 0;
	int sys_mmc = 0;
	int ext_mmc = 0;
	int mmc0 = 0;
	int mmc1 = 0;
	int mmc2 = 0;

	switch (sys_info->modeltype) {
	case UC8131:
	case UC8132:
	case UC8162:
	case UC8112:
		sys_mmc = 0;
		ext_mmc = 1;
		mmc0 = USED;
		mmc1 = USED;
		mmc2 = UNUSED;
		break;
/*	case UC2104:
	case UC2112_T:
		sys_mmc = 1;
		ext_mmc = 0;
		mmc0 = USED;
		mmc1 = USED;
		mmc2 = UNUSED;
		break;*/
		
	default:
		ret = -1;
		break;
	}

	sys_info->sys_mmc = sys_mmc;
	sys_info->ext_mmc1 = ext_mmc;
	sys_info->mmc0 = mmc0;
	sys_info->mmc1 = mmc1;
	sys_info->mmc2 = mmc2;

	return ret;
}

static int board_usbinfo_init(board_infos *sys_info) 
{

	int usb_num = 0;
	int usb_dongle = 0;
	int usb_cellular = 0;
	int ret = 0;

	switch (sys_info->modeltype) {
	case UC8131:
	case UC8132:
	case UC8162:
	case UC8112:
		usb_num = 2;
		usb_dongle = UNUSED;
		usb_cellular = 1;
		break;
	/*case UC2104:
		usb_num = 1;
		usb_dongle = UNUSED;
		usb_cellular = 1;
		break;
	case UC2111:
	case UC2112:
	case UC2112_T:
		usb_num = 2;
		usb_dongle = UNUSED;
		usb_cellular = UNUSED;
		break;*/
	default:
		ret = -1;
		break;
	}

	sys_info->usb_num = usb_num;
	sys_info->usb_dongle = usb_dongle;
	sys_info->usb_cellular = usb_cellular;

	return ret;
}

static void board_mn_init(board_infos *sys_info){
        char *s;
        s = getenv("modelname");

        if (strcmp("UC-8100", s) == 0){
            setenv("modelname", sys_info->modelname);
            run_command("saveenv",0);
        }
        return;
}

int board_info_init(board_infos *board_info)
{
	int ret = 0;
	
	ret = pre_board_init(board_info);

	if (ret)
		printf("preboard_init fail\n");

	ret = board_modelinfo_init(board_info);
	
	if (ret)
		printf("board_modelinfo_init fail\n");

        board_mn_init(board_info);

	ret = board_dtbinfo_init(board_info);

	if (ret)
		printf("board_dtbinfo_init fail\n");

	ret = board_mmcinfo_init(board_info);

	if (ret)
		printf("board_mmcinfo_init fail\n");

	ret = board_usbinfo_init(board_info);

	if (ret)
		printf("board_usbinfo_init fail\n");


#ifdef SYS_INFO_DEBUG
	printf("modeltype: %d\n",	board_info->modeltype);
	printf("modelname: %s\n",	board_info->modelname);
	printf("dtbname: %s\n",		board_info->dtbname);
	printf("sys_mmc: MMC%d\n",	board_info->sys_mmc);
	printf("ext_mmc1: MMC%d\n",	board_info->ext_mmc1);
	printf("ext_mmc2: MMC%d\n",	board_info->ext_mmc2);
	printf("Enable MMC0: %d\n",	board_info->mmc0);
	printf("Enable MMC1: %d\n",	board_info->mmc1);
	printf("Enable MMC2: %d\n",	board_info->mmc2);
	printf("USB NUMBER: %d\n",	board_info->usb_num);
	printf("usb_dongle: %d\n",	board_info->usb_dongle);
	printf("usb_cellular: %d\n",	board_info->usb_cellular);
#endif

        return ret;
};

