/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    sd.c

    This file includes functions of TI AM335X SD controller.
    
    2014-01-10	Sun Lee
*/

#include <common.h>
#include <command.h>
#include <bios.h>
#include <model.h>
#include "soc_AM335x.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "../cmd_bios.h"
#include <types.h>
#include "sd.h"

/*
 *	SD test ;
*/

int sd_rw_func(unsigned int sd_num, unsigned int part, int test_pattern)
{
	unsigned int i;
	char msg [MAX_SIZE_64BYTE] = {0};

	sprintf(msg,"mmc dev %d", sd_num);
	
	if (run_command(msg, 0) != 0)
		printf("mmc %d device not foumd\n", sd_num);

	run_command("mmc rescan", 0);

	printf("*******MMC%d: Write Data(0x%x) Test*******\n", sd_num, test_pattern);

	for(i = 0; i < MEM_MAX_SIZE; i += 4)
		VPlong(MMC_TEST_ADDR1 + i) = test_pattern;
	
	sprintf(msg, "fatwrite mmc %d:%d 0x81000000 test.bin 0x800000", sd_num, part);
	run_command(msg, 0);
	udelay(100*1000);
	
	sprintf(msg, "fatload mmc %d:%d 0x82000000 test.bin 0x800000", sd_num, part);
	run_command(msg, 0);
	udelay(100*1000);

	sprintf(msg, "fatls mmc %d:%d", sd_num, part);
	run_command(msg, 0);

	for(i = 0; i < MEM_MAX_SIZE; i += 4) {
		if (VPlong(MMC_TEST_ADDR2 + i) != test_pattern) {
			printf("MMC%d: Read/Write data(0x%x) Test..Fail\n", sd_num, test_pattern);
			printf("test.bin [addr1 = 0x%x,val = 0x%lx]\n", (MMC_TEST_ADDR2 + i), VPlong(MMC_TEST_ADDR2 + i));
			return -1;
		}
	}

	return 0;

}

int cd_wp_pin_status(unsigned int mmc, int *val)
{

	switch(mmc){
	case MMC0:
		*val = (VPlong(SOC_MMCHS_0_REGS + 0x224) & (BIT18 | BIT19));
		break;
	case MMC1:
		*val = (VPlong(SOC_MMCHS_1_REGS + 0x224) & (BIT18 | BIT19));
		break;
	case MMC2:
		*val = (VPlong(SOC_MMCHS_2_REGS + 0x224) & (BIT18 | BIT19));
		break;
	default:
		break;
	}


	return 0;
}

int do_mmc_card_detect(int mmc_num, int show)
{
	char msg [128] = {0};
	int ret = 0;

	sprintf(msg,"mmc dev %d", mmc_num);
	ret = run_command(msg, 0);

	ret = run_command("mmc rescan", 0);

	if (ret != 0){
		printf("mmc rescan failed\n");
		return -1;
	}

	if (show){
		if (ret == 0)
			printf("MMC%d :SD card IS inserted.\n", mmc_num);
		else
			printf("MMC%d :SD card NOT inserted.\n", mmc_num);
	}

EXIT:
	return ret;

}

#ifdef  SD_MP
void SD_MPTEST(u8 led, u8 mmc)
{

	int ret = 0;

	if (mmc == 0)
		printf("[0x%04x]Standard SD Read/Write testing ... \n",led);
	else
		printf("[0x%04x]Micro SD Read/Write testing ... \n",led);

        ret = do_mmc_card_detect(mmc, 0);

	if (ret  == 0) {
		ret = sd_rw_func(mmc, 1, MMC_TEST_PATTERN1);
		ret |= sd_rw_func(mmc, 1, MMC_TEST_PATTERN2);
	}

        if (ret < 0) {
                if(mmc == 0)
			printf("Standard SD Read/Write testing ** FAIL ** \r\n");
		else
                        printf("Micro SD Read/Write testing ** FAIL ** \r\n");

                halt(led, HALT_MODE1);
	} else {
		if(mmc == 0)
			printf("Standard SD Read/Write testing ** OK ** \r\n");
		else
			printf("Micro SD Read/Write testing ** OK ** \r\n");
        }
}
#endif  //SD_MP

