/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <net.h>
#include <spi.h>
#include <rf.h>
#include <asm/gpio.h>
#include <environment.h>
#include <bios.h>                        
#include <model.h>
#include "cmd_bios.h"
#include "types.h"
#include "mmc.h"
#include "fs.h"
#include "fat.h"
#include <pca953x.h>
#include <cli.h>
#include "moxa_console.h"
#include "moxa_lib.h"
#include "moxa_boot.h"
#include "moxa_upgrade.h"
DECLARE_GLOBAL_DATA_PTR;

#define TFTP_DEFAULT_LOCAL_IP "192.168.30.174"
#define TFTP_DEFAULT_SERVER_IP  "192.168.30.71"

int tftp_upgrade_start = 0;
extern unsigned  int fw_tftp_size;
extern char console_buffer[CONFIG_SYS_CBSIZE + 1];
static board_infos sys_info;

int download_bios(const char *name)
{
	int ret = 0;
	char cmd [MAX_SIZE_64BYTE] = {0};

	run_command("setenv -f ethact cpsw0", 0);

	sprintf (cmd, "tftp 0x81000000 %s", name);
	
	if ((ret = run_command (cmd, 0)) != 0) {
		printf ("TFTP BIOS file transfer fail.\r\n");
		goto EXIT;
	}

	mdelay (100);
	
	sprintf (cmd, "sf probe");

	if ((ret = run_command (cmd, 0)) != 0) {
		printf ("SPI probe fail.\n");
		goto EXIT;
	}

	mdelay (100);
	
	if ((ret = run_command ("sf update 0x81000000 0 0x180000", 0)) != 0) {
		printf ("BIOS upgrade fail.\n");
		goto EXIT;
	} else {
		printf("\n**Warning** Override Env parameters? (y/n)");
		cli_readline(" ");

		if(strcmp(console_buffer,"Y") == 0 ||
			strcmp(console_buffer,"yes") == 0 ||
			strcmp(console_buffer,"y") == 0 ) {
			run_command("env default -f -a", 0);
			run_command("env save", 0);
		}
	}
	
EXIT:
	return ret;

}

int mirror_mmc_to_mmc (int from_mmc, int dest_mmc, u32 total_blk)
{
	int ret = 0;
	u32 remain_blk = total_blk;
	u32 finish_blk = 0;
	u32 mirror_size = 0x60000;
	char cmd[MAX_SIZE_256BYTE];
	int retry;

	retry = 0;

	while (remain_blk > 0) {
		
		if (retry > 3) {
			printf ("Mirror MMC%d to MMC%d Fail...\n", from_mmc, dest_mmc);
			ret = (-1);
			break;
		}

		sprintf (cmd, "mmc rescan && mmc dev %d && mmc read 0x80000000 0x%x 0x%x && mmc dev %d\
			 && mmc write	0x80000000 0x%x 0x%x", from_mmc, dest_mmc, finish_blk, mirror_size,\
								 finish_blk, mirror_size);
	
		printf ("%s\n", cmd);
		
		ret = run_command (cmd, 0);
		
		if (ret != 0) {
			retry++;
			continue;
		}

		finish_blk += mirror_size;
		
		if (finish_blk == total_blk)
			break;

		if (remain_blk > mirror_size)
			remain_blk -= mirror_size;

		if (remain_blk < mirror_size)
			mirror_size = remain_blk;

		retry = 0;
	}

	return ret;
}

int download_firmware_mirror_mmc (int from_mmc, int to_mmc)
{
	struct mmc *mmc;
	u32 total_blk_msd;
	u32 total_blk_emmc;
	u32 blk_len;
	int ret = 0;

	mmc = find_mmc_device (from_mmc);

	if (mmc) {
		ret = mmc_init (mmc);
		
		if(ret)
			goto EXIT;

		total_blk_msd = mmc->capacity / mmc->read_bl_len;
#ifdef DEBUG
		printf ("Size: %llu\n", mmc->capacity);
		printf ("Blk: %lx\n", total_blk_msd);
#endif /* DEBUG */
		mmc = find_mmc_device(to_mmc);
	
		if(mmc){
			ret = mmc_init (mmc);
	
			if(ret)
				goto EXIT;

			total_blk_emmc = mmc->capacity / mmc->read_bl_len;
#ifdef DEBUG
			printf ("Size: %llu\n", mmc->capacity);
			printf ("Blk: %lx\n", total_blk_emmc);
#endif /* DEBUG */
			if (total_blk_msd > total_blk_emmc) {
				printf ("MicroSD size is larger than eMMC, only copy partial data to eMMC\n");
				blk_len = total_blk_emmc;
			} else {
				blk_len = total_blk_msd;
			}

			ret = mirror_mmc_to_mmc (from_mmc, to_mmc, blk_len);
		} else {
			printf ("no mmc device at MOXA_MMC2\n");
			ret = -1;
		}
	} else {
		printf("no mmc device at MOXA_MMC0\n");
		ret = -1;
	}

EXIT:
	return ret;
}

int do_download_firmware_mirror_mmc(void)
{
	int ret = 0;

	ret = download_firmware_mirror_mmc(MOXA_MMC0, MOXA_MMC2);
	
	return ret;	
}


int copy_file_to_mmc(char *fw_name, signed long long fw_size, int fw_blk, uint mmc_blk_len, int from_mmc, int to_mmc)
{
	int ret = 0;
	char cmd[MAX_SIZE_256BYTE];
	u32 rlen = 0xC000000;
	u32 wlen = rlen / mmc_blk_len;
	u32 roffset = 0;
	u32 woffset = 0;
	signed long long remain_len = fw_size;
	signed long long finish_len = 0;
	int retry = 0;

	while (remain_len > 0) {

		if (retry > 3) {
			printf ("Copy file ERROR...\n");
			ret = (-1);
			break;
		}
		
		sprintf (cmd, "mmc rescan && mmc dev %d && fatload mmc %d:1 0x80000000 %s 0x%x 0x%x \
			&& mmc write 0x80000000 0x%x 0x%x", to_mmc, from_mmc, fw_name, rlen, roffset, woffset, wlen);

		printf ("%lld %lld\n", finish_len, remain_len);
		printf ("%s\n", cmd);

		ret = run_command (cmd, 0);
		
		if (ret != 0) {
			retry++;
			continue;
		}

		finish_len += rlen;
		roffset += rlen;
		woffset += wlen;
		remain_len -= rlen;

		if (remain_len < rlen) {
			rlen = remain_len;
			wlen = remain_len / mmc_blk_len;
			
			if (remain_len % mmc_blk_len)
				wlen++;
			
		}
	}

	return ret;
}

int mmc_firmware_upgrade (char *fw_name, int from_mmc, int dest_mmc)
{
	int ret = 0;
	signed long long fw_size;
	struct mmc *mmc;
	u32 emmc_blk;
	u32 fw_blk;
	int i = 0;
	char tmp [4] = {0};
	
	sprintf(tmp,"%d", from_mmc);

	ret = fs_set_blk_dev ("mmc", tmp, 1);
	
	if (ret < 0) {
		printf ("no mmc device at MOXA_MMC1...\n");
		goto EXIT;
	}

	for (i = 0; i < strlen(fw_name); i++) {
		if(fw_name[i] == '\0')
			break;

		TOLOWER(fw_name[i]);
	}
	
	fw_size = do_fat_get_file_size (fw_name);

	if (fw_size == (-1)) {
		printf ("Firmware File %s not exist...\n", fw_name);
		ret = -1;
		goto EXIT;
	}

	printf ("File Size: %lld\n", fw_size);

	mmc = find_mmc_device (1);

	if (mmc) {
		mmc_init (mmc);
		emmc_blk = mmc->capacity / mmc->read_bl_len;
		fw_blk = fw_size / mmc->read_bl_len;
		
		if (fw_size % mmc->read_bl_len)
			fw_blk++;

		if (fw_blk > emmc_blk) {
			printf ("Firmware File size is larger than eMMC size\n");
			goto EXIT;
		}

		copy_file_to_mmc (fw_name, fw_size, fw_blk, mmc->read_bl_len, from_mmc, dest_mmc);
		
		gpio_direction_output(PIO_LED_MP_0, 1);
		gpio_direction_output(PIO_LED_MP_1, 1);
		gpio_direction_output(PIO_LED_MP_2, 1);
		gpio_direction_output(PIO_LED_MP_3, 1);
		gpio_direction_output(PIO_LED_MP_4, 1);

	} else {
		printf ("no mmc device at MOXA_MMC2\n");
	}

EXIT:
	return ret;

}

int download_firmware_copy_from_file (char *fw_name)
{
	return mmc_firmware_upgrade(fw_name, MOXA_MMC1, MOXA_MMC0);
}

int copy_file_to_emmc (unsigned int fw_size)
{

	int ret = 0;
	char cmd[MAX_SIZE_128BYTE];
	u32 wlen = 0;
	unsigned int w_size = 0;
	
	wlen = fw_size % EMMC_COPY_LIMIT_SIZE;

	if (fw_size > EMMC_COPY_LIMIT_SIZE){
		w_size = fw_size - EMMC_COPY_LIMIT_SIZE;
	}

	if (wlen == 0){
		w_size = (fw_size - EMMC_COPY_LIMIT_SIZE) / 512;
		wlen = (EMMC_COPY_LIMIT_SIZE / 512);
	}else{
		w_size = (fw_size - wlen) / 512;
		wlen = (wlen / 512) ;
	}

	sprintf (cmd, "mmc rescan");
	ret = run_command (cmd, 0);

	sprintf (cmd, "mmc dev %d", sys_info.sys_mmc);
	ret = run_command (cmd, 0);
	
	sprintf (cmd, "mmc write 0x81000000 0x%x 0x%x", w_size, wlen);
	ret = run_command (cmd, 0);
	
	printf("cmd:%s, w_size:%d, wlen:%x\n", cmd, w_size, wlen);
	
	return ret;
}

int copy_tftp_firmware_to_emmc (void)
{
	int ret = 0;

	ret = copy_file_to_emmc (fw_tftp_size);

	return ret;
}

int tftp_download_firmware (char *fw_name, board_infos *board_info)
{
	int ret = 0;
	char cmd[MAX_SIZE_256BYTE] = {0};
	char buf[MAX_SIZE_64BYTE] = {0};

	ret = run_command("setenv -f ethact cpsw0", 0);
	sys_info = *board_info;

	if(ret)
		goto EXIT;

	sprintf(buf, "ping %s", getenv("serverip"));

	if ((ret = run_command(buf, 0) != 0)){
		printf("Destination Net Unreachable\n");
		goto EXIT;
	}

	tftp_upgrade_start = 1;
	
	sprintf (cmd, "tftp 0x81000000 %s", fw_name);

	if ((ret = run_command (cmd, 0)) != 0) {
                printf ("TFTP BIOS file transfer fail.\r\n");
		fw_tftp_size = 0;
                tftp_upgrade_start = 0;
		goto EXIT;
	}

	fw_tftp_size = 0;
	tftp_upgrade_start = 0;

EXIT:
	return ret;

}

int tftp_setting_default(void)
{
	int ret;
	char cmd[MAX_SIZE_64BYTE] = {0};

	ret = sprintf(cmd, "setenv ipaddr "TFTP_DEFAULT_LOCAL_IP"");

	if (ret)
		goto EXIT;

	if ((ret = run_command(cmd, 0)) != 0){
		printf("Set IP Fail(setenv ipaddr) [%d] \r\n", ret);
		goto EXIT;
	}
    
	ret = sprintf(cmd, "setenv serverip "TFTP_DEFAULT_SERVER_IP"");
	
	if (ret)
		goto EXIT;

	if ((ret = run_command(cmd, 0)) != 0){
		printf("Set IP Fail(setenv serverip) [%d] \r\n", ret);
		goto EXIT;
	}

	ret = sprintf(cmd, "saveenv");

	if (ret)
		goto EXIT;

	if ((ret = run_command(cmd, 0)) != 0){
		printf("Set IP Fail(saveenv) [%d] \r\n", ret);
		goto EXIT;
	}

	printf("TFTP Setting has been set to default. \r\n");

EXIT:
	return ret;
}

int change_ip(void)
{
	int ret = 0;
	char buf[MAX_SIZE_32BYTE] = {0};
	char cmd[MAX_SIZE_64BYTE] = {0};

LOCAL_IP:
	printf("\r\nLocal IP Address : ");

	if((ret = con_get_string(buf, sizeof(buf))) == -1) {
		printf("\r\n");
		goto EXIT;
	} else if (ret == 0) {
		goto LOCAL_IP;
	}

	if (inet_addr(buf) == 0){
		printf("\r\nIP Value Error!\r\n");
		goto LOCAL_IP;
	}else{
		printf("\r\n");
		sprintf(cmd, "setenv ipaddr %s", buf);

		if ((ret = run_command(cmd, 0)) != 0){
			printf("Set IP Fail(setenv ipaddr) [%d] \r\n", ret);
			goto EXIT;
		}
	}

SERVER_IP:
	printf("\r\nServer IP Address : ");

	if((ret = con_get_string(buf, sizeof(buf))) == -1){
		printf("\r\n");
		goto EXIT;
	} else if (ret == 0) {
		goto SERVER_IP;
	}

	if (inet_addr(buf) == 0) {
		printf("\r\nIP Value Error!\r\n");
		goto SERVER_IP;
	} else {
		printf("\r\n");
		sprintf(cmd, "setenv serverip %s", buf);

		if ((ret = run_command(cmd, 0)) != 0){
			printf("Set IP Fail(setenv serverip) [%d] \r\n", ret);
			goto EXIT;
		}
	}
	
	printf("\r\n");
	sprintf(cmd, "saveenv");

	if ((ret = run_command(cmd, 0)) != 0){
		printf("Set IP Fail(saveenv) [%d] \r\n", ret);
		goto EXIT;
	}

EXIT:	
	return ret;
}


int show_ip(void)
{
	char cmd[MAX_SIZE_64BYTE] = {0};
	int ret = 0;

	printf("\r\nLocal IP Address : ");
	
	sprintf(cmd, "printenv ipaddr");
	
	ret = run_command(cmd, 0);

	if (ret)
		goto EXIT;

	printf("\r\nServer IP Address : ");
	
	sprintf(cmd, "printenv serverip");
	
	ret = run_command(cmd, 0);

EXIT:

	return ret;
}

