/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/


enum MMC_DEVICE{
	MOXA_MMC0 = 0,
	MOXA_MMC1,
	MOXA_MMC2
};

int download_bios(const char *name);
int mirror_mmc_to_mmc (int from_mmc, int dest_mmc, u32 total_blk);
int download_firmware_mirror_mmc (int from_mmc, int to_mmc);
int do_download_firmware_mirror_mmc(void);
int copy_file_to_mmc(char *fw_name, signed long long fw_size, int fw_blk, uint mmc_blk_len, int from_mmc, int to_mmc);
int mmc_firmware_upgrade (char *fw_name, int from_mmc, int dest_mmc);
int download_firmware_copy_from_file (char *fw_name);
int copy_file_to_emmc (unsigned int fw_size);
int copy_tftp_firmware_to_emmc (void);
int tftp_download_firmware (char *fw_name, board_infos *sys_info);
int tftp_setting_default(void);
int change_ip(void);
int show_ip(void);

