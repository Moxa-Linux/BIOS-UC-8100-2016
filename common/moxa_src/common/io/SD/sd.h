/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    ethernet.h

    Definitions of TI AM335X SD controller.
    
    2014-01-10	Sun Lee
*/

#ifndef _SD_H
#define _SD_H

#define MEM_MAX_SIZE            0x800000
#define MMC_TEST_ADDR1          0x81000000
#define MMC_TEST_ADDR2          0x82000000
#define MMC_TEST_PATTERN1       0x01234567
#define MMC_TEST_PATTERN2       0xAA5555FF

enum MMC_DEICE{
        MMC0,
        MMC1,
        MMC2
};

void SD_MPTEST(u8 led, u8 k);
int sd_rw_func(unsigned int sd_num, unsigned int part, int test_pattern);
int cd_wp_pin_status(unsigned int mmc, int *val);
int do_mmc_card_detect(int mmc_num, int show);
#endif	//_SD_H
