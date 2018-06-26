/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/


//AUTO_MODE:	OS Boot Priority (LX -> NK.nb0 -> NK.bin)
//LX_MODE:	Only LX
//CE_MODE:	NK.nb0 -> NK.bin
#include "sys_info.h"

enum BOOT_MODE{
	AUTO_MODE = 0,
	LX_MODE,
	CE_MODE
};

enum {
        UC5101 = 0,
        UC5102,
        UC5111,
        UC5112,
};

enum {
        UC8131 = 0,
        UC8132,
        UC8162,
        UC8112,
};

int do_Encryption_func(uchar sd_num, uchar re_key, unsigned long f1addr, unsigned long f2addr, unsigned long fs1, unsigned long fs2);
int do_secure_boot(unsigned char sd_num);
int do_run_linux_func(unsigned char sd_num);
int do_run_wince_func(unsigned char sd_num);
int do_run_mmc0_func(board_infos *board_info);
int do_run_mmc1_func(board_infos *board_info);
int do_run_mmc2_func(board_infos *board_info);
int do_run_mmc_boot(board_infos *board_info);
int run_mmc_func(int boot_mmc, char *dtbname, int fs_info);
void run_OS_test_func(char * testmode, char * dtbname);
int do_run_mmc1_mp_func(board_infos *board_info);
