/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
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
#include "sd.h"
#include <i2c.h>
#include <pca953x.h>
#include <cli.h>
#include "moxa_lib.h"
#include "moxa_boot.h"
#include "cmd_bios.h"
#include "sys_info.h"

unsigned long start_offset [16] = {0x5DC00, 0x5EB00, 0x60900, 0x73500, 0x86100, 0xAB900, 0xD1100,\
			 0xF6900, 0x25800, 0x4B000, 0x96000, 0xBB800, 0xE1000, 0xD7A00, 0xCE400, 0x79E00};

unsigned char direction_offset [3] = {0x00, 0x55, 0xAA};
unsigned char jump_offset [16] = {0x04, 0x02, 0x08, 0x06, 0x0A, 0x08, 0x06, 0x04, 0x02, 0x02, 0x08, 0x06, 0x04, 0x06, 0x08, 0x0A};

#if 0
static int board_info(void)
{
	int model = 0;

	gpio_request(GPIO1_12, "gpmc_ad12");
	gpio_direction_input(GPIO1_12);
	gpio_request(GPIO1_13, "gpmc_ad13");
	gpio_direction_input(GPIO1_13);
	gpio_request(GPIO1_14, "gpmc_ad14");
	gpio_direction_input(GPIO1_14);
	gpio_request(GPIO1_15, "gpmc_ad15");
	gpio_direction_input(GPIO1_15);

	model = gpio_get_value(GPIO1_15) | gpio_get_value(GPIO1_14) << 1 | gpio_get_value(GPIO1_13) << 2 | gpio_get_value(GPIO1_12) << 3;

        return model;
}
#endif

int do_encryption_func(uchar sd_num, uchar re_key,unsigned long f1addr,unsigned long f2addr,unsigned long fs1,unsigned long fs2)
{
	uchar key1 = 0;
	uchar key2 = 0;
	uchar key3 = 0;
	uchar val = 0;
	int mmc = 0;
	unsigned long Start_offset = 0;	//by key1
	unsigned char direction = 0;	//by key2
	unsigned char jump_Offset = 0;	//by key3
	unsigned long i = 0;
	unsigned long j = 1;
	unsigned long size = 0;
	unsigned long baddr = 0;
	char cmd_msg [MAX_SIZE_64BYTE] = {0};
	char boot_msg [MAX_SIZE_128BYTE] = {0};

	if (re_key) {
		sprintf(cmd_msg, "fatload mmc %d:1 0x81000000 uImage", sd_num);

		if(run_command(cmd_msg, 0) != 0){
#ifdef Secure_boot_debug		
			printf("Load uImage file to mem:0x81000000 fail\n");
#endif	//Secure_boot_debug		
			return -1;
		} else {
			fs1 = diag_get_env_conf_u32_mem("filesize");
			f1addr = 0x81000000;		
		}

		i = randkey();	
		val = (uchar)(randkey());		//Get randkey value
		key1 = (val & 0x0F);			//set key1 value

		if(val % 2)
			key2 = 2;			//set key2 value(DOWN)
		else
			key2 = 1;			//set key2 value(UP)			

		key3 = ((val >> 4) & 0X0F);		//set key3 value
	} else {
		key1 = VPchar(f1addr + fs1 - 3);
		key2 = VPchar(f1addr + fs1 - 2);
		key3 = VPchar(f1addr + fs1 - 1);	
	}

	//get key value by predefine_table.h file
	Start_offset = start_offset[key1];
	direction = direction_offset[key2];	
	jump_Offset = jump_offset[key3];	
	
	size = (fs1 / jump_Offset);
	
	if (fs1 % jump_Offset)
		size++;

#ifdef Secure_boot_debug		
	printf("key1 = %x\n",key1);
	printf("key2 = %x\n",key2);
	printf("key3 = %x\n",key3);
	printf("Start_offset = %lx\n",Start_offset);
	printf("direction = %x\n",direction);
	printf("jump_Offset = %x\n",jump_Offset);
#endif	//Secure_boot_debug		
					
	if (direction == 0x55) {			//UP
		for(i=0; i < size; i++){
			val = VPchar(f1addr+Start_offset);
			
			val ^= VPchar(f2addr+baddr);
			VPchar(f1addr+Start_offset) = val;
			
			if (i == ((fs2*j)-1)) {
				baddr = 0;
				j++;
			} else {
				baddr++;
			}
			
			if (Start_offset < jump_Offset){
//				key1_Start_offset -= key3_jump_Offset;
				Start_offset = (fs1 - jump_Offset);
			} else {					
				Start_offset -= jump_Offset;
			}
		}
	}else{
		//DOWN
		for(i = 0; i < size; i++){
			val = VPchar(f1addr+Start_offset);
			val ^= VPchar(f2addr+baddr);
			VPchar(f1addr+Start_offset) = val;
//					VPchar(f1addr+key1_Start_offset) = 0xff;	//for debug
			if (i == ((fs2*j)-1)) {
				baddr = 0;
				j++;
			} else {
				baddr++;
			}

			Start_offset += jump_Offset;					
			if (Start_offset > fs1) {
				Start_offset = (Start_offset - fs1);
			}
		}
				
	}

	if(re_key) {
//R28A		printf("Please not to Power OFF the machine.\n");
		VPchar(f1addr+fs1-3) = key1;
		VPchar(f1addr+fs1-2) = key2;
		VPchar(f1addr+fs1-1) = key3;
		sprintf(cmd_msg, "fatwrite mmc %d:1 0x81000000 uImage %lx", sd_num, fs1);

		if (run_command(cmd_msg, 0) != 0) {
#ifdef Secure_boot_debug		
			printf("fatwrite uImage file to mmc %d fail\n", sd_num);
#endif// Secure_boot_debug		
			return -1;
		}
	}

	if (sd_num) {
		if (run_command("mmc dev 0", 0) == 0)
			mmc = 1;
		else
			mmc = 0;
		
		sprintf(boot_msg, "setenv bootargs mac=${ethaddr} sd=${sd_protected} ver=${biosver} console=ttyS0,115200n8 \
						root=/dev/mmcblk%dp2 rootfstype=ext4 rootwait", mmc);
		run_command(boot_msg, 0);
	} else {
		run_command("setenv bootargs mac=${ethaddr} sd=${sd_protected} ver=${biosver} console=ttyS0,115200n8 root=/dev/mmcblk0p2 rootfstype=ext4 rootwait", 0);
	}

	run_command("bootm 0x80007FC0", 0);
	return 0;
}

int do_secure_boot(unsigned char  sd_num)
{
	unsigned long filesize1 = 0;
	unsigned long filesize2 = 0;
	unsigned long fileaddr1 = 0;
	unsigned long fileaddr2 = 0;
	unsigned long firstkey = 0;	
	unsigned char key1 = 0;
	unsigned char key2 = 0;
	unsigned char key3 = 0;
	char cmd_msg [MAX_SIZE_64BYTE] = {0};


	sprintf(cmd_msg, "mmc dev %d", sd_num);
	run_command(cmd_msg, 0);
	sprintf(cmd_msg, "fatload mmc %d:1 0x80007FC0 uImage", sd_num);

	if (run_command(cmd_msg, 0) == 0) {	
		filesize1 = diag_get_env_conf_u32_mem("filesize");
		fileaddr1 = 0x80007FC0;
		//load u-boot.img to memory
		sprintf(cmd_msg, "fatload mmc %d:1 0x80B08000 u-boot.img", sd_num);

		if (run_command(cmd_msg, 0) == 0) {
			filesize2 = diag_get_env_conf_u32_mem("filesize");
			fileaddr2 = 0x80B08000;
		} else {
			return -1;
		}

		//get key by uImange last 3 byte
		key1 = VPchar(fileaddr1 + filesize1 - 3);
		key2 = VPchar(fileaddr1 + filesize1 - 2);
		key3 = VPchar(fileaddr1 + filesize1 - 1);
		firstkey = ((key1 << 16) | (key2 << 8) | key3);
#ifdef Secure_boot_debug		
		printf("First key value=0x%lx\n",firstkey);
#endif	//Secure_boot_debug
		if (firstkey == 0x40004)//if 0x40004 must to change key
			return (do_encryption_func(sd_num,1,fileaddr1,fileaddr2,filesize1,filesize2));
		else
			return (do_encryption_func(sd_num,0,fileaddr1,fileaddr2,filesize1,filesize2));
	} else {
		return -1;
	}
}

/* Run Linux Kernel
 * Input: Boot from MMC/SD number
 *
 */
int do_run_linux_func(unsigned char sd_num)
{
	return do_secure_boot(sd_num);		//R28
}
/* Run WinCE Kernel
 * Input: Boot from MMC/SD number
 *
 */
int do_run_wince_func(unsigned char sd_num)
{
	char ce_head[7] = {0x42, 0x30, 0x30, 0x30, 0x46, 0x46, 0x0A};
	unsigned long i, offset=0, ImageStart, ImageLength, RecAddr, RecLen, RecChk, LaunchAddr, kernel_address;
	char cmd_msg [MAX_SIZE_64BYTE] = {0};

	sprintf(cmd_msg, "mmc dev %d", sd_num);
	run_command(cmd_msg, 0);
	sprintf(cmd_msg, "fatload mmc %d:1 0x80002000 NK.nb0", sd_num);

	if (run_command(cmd_msg, 0) == 0) {
	    	printf("Load NK.nb0 ..OK\n");
	    	run_command("go 0x80002000", 0);
		return 0;	    	
	} else {
		kernel_address = 0x88000000;
		sprintf(cmd_msg, "fatload mmc %d:1 %lx NK.bin", sd_num, kernel_address);
		if (run_command(cmd_msg, 0) == 0) {
			for (i = 0; i < 7; i++){
				if (ce_head[i] == *(char *)(kernel_address + i)) {
					continue;
				} else {
					printf("Not NK.Bin Format file.\n");
					return -1;	    	
				}		
			}

			offset += 7;
			memcpy((char *)&ImageStart, (char *)kernel_address + offset, 4);
			offset += 4;
 			memcpy((char *)&ImageLength, (char *)kernel_address + offset, 4);
			offset += 4;
		
			while (1){
				memcpy((char *)&RecAddr, (char *)kernel_address + offset, 4);
				offset += 4;
				memcpy((char *)&RecLen, (char *)kernel_address + offset, 4);
				offset += 4;
				memcpy((char *)&RecChk, (char *)kernel_address + offset, 4);
				offset += 4;
			
				// check for last record
				if ((!RecAddr) && (!RecChk)){
					// update launch address
					LaunchAddr = RecLen;
	    				printf("Load NK.bin ..OK\n");
					printf("Total Offset = 0x%lx, StartUp Addr = 0x%lx\r\n\r\n", offset, LaunchAddr);
					break;
				}
			
				memcpy((char *)RecAddr, (unsigned char *)kernel_address+offset, RecLen);
				offset += RecLen;
			}
    			sprintf(cmd_msg, "go %lx", LaunchAddr);
    			run_command(cmd_msg, 0);

			return 0;

		} else {
			return -1;
		}
	}		
}


int do_run_mmc0_func(board_infos *board_info)
{
	int fs = 0;
	printf ("Boot to MMC0 SD card...\n");

	run_mmc_func(MMC0, board_info->dtbname, fs);

	return -1;
}

int do_run_mmc1_mp_func(board_infos *board_info)
{
	int fs = 0;
	printf ("Boot to MMC0 SD card...\n");

	//run_mmc_func(MMC0, board_info->dtbname, fs);
        run_OS_test_func("T1(MP)", board_info->dtbname);

	return -1;
}

int do_run_mmc1_func(board_infos *board_info)
{
	int fs_info = 0;
	char fs_dev[MAX_SIZE_16BYTE] = {0};
	
	printf ("Boot to MMC1 eMMC...\n");

	sprintf(fs_dev, "mmc dev %d", MMC0);

	if (run_command(fs_dev, 0) == 0)
		fs_info = 1;
	else
		fs_info = 0;

	run_mmc_func(MMC1, board_info->dtbname, fs_info);

	return -1;

}

int do_run_mmc2_func(board_infos *board_info)
{
	int fs_info = 0;
	char fs_dev[MAX_SIZE_16BYTE] = {0};
	
	printf ("Boot to MMC2 eMMC...\n");

	sprintf(fs_dev, "mmc dev %d", MMC0);

	if (run_command(fs_dev, 0) == 0)
		fs_info = 1;

	sprintf(fs_dev, "mmc dev %d", MMC1);

	if (run_command(fs_dev, 0) == 0)
		fs_info += 1;

	run_mmc_func(MMC2, board_info->dtbname, fs_info);

	return -1;

}

#if 0
int run_mmc_boot(void){

	int ret = 0;
	int val = 0;

	i2c_set_bus_num(2);

	pca953x_set_dir (0x22, (1<<10), 1);
	
	val = pca953x_get_val (0x22);
	
	i2c_set_bus_num(2);
	
	if (val & (1<<10)){
		ret = do_run_mmc_func(1);
        }else{
		ret = do_run_mmc_func(0);
        }
	
	i2c_set_bus_num(0);

	return ret;
}
#endif


int do_run_mmc_boot(board_infos *board_info)
{
	int fs_info = 0;
	char fs_dev[MAX_SIZE_16BYTE] = {0};
	int mmc_num = 0;

	for (mmc_num = 0; mmc_num < board_info->sys_mmc; mmc_num++) {

		sprintf(fs_dev, "mmc dev %d", mmc_num);

		if (run_command(fs_dev, 0) == 0);
			//fs_info += 1;
	}

	return run_mmc_func(board_info->sys_mmc, board_info->dtbname, fs_info);
}

#if 0
int run_mmc_func(int boot_mmc, char *dtbname, fs_info)
{
	char msg[MAX_SIZE_256BYTE] = {0};
	char dtb_info[MAX_SIZE_64BYTE] = {0};
	char kernel_info[MAX_SIZE_64BYTE] = {0};
	char fs_dev[MAX_SIZE_32BYTE] = {0};
	int fs_info = 0;
	int ret = 0;
	int fastboot = 0;

	sprintf(dtb_info, "fatload mmc %d:1 0x80000000 %s", sys_info->sys_mmc, sys_info->dtbname);
	sprintf(kernel_info, "fatload mmc %d:1 0x81000000 uImage", sys_info->sys_mmc);

	if (run_command (dtb_info, 0) == 0) {
		if (run_command (kernel_info, 0) == 0) {

			if (sys_info->ext_mmc1 >= 0) {
				sprintf(fs_dev, "mmc dev %d", sys_info->ext_mmc1);

				if (run_command(fs_dev, 0) == 0)
					fs_info = 1;
				else
					fs_info = 0;
			} else {

				fs_info = 0;
			}

			if (fastboot)  
				sprintf(msg, "setenv bootargs mac=${ethaddr} sd=2 ver=3 console=ttyS0,115200n8 root=/dev/mmcblk%dp2 \
					rw quiet lpj=4980736 rootfstype=ext4 rootwait", fs_info);
			else
				sprintf(msg, "setenv bootargs mac=${ethaddr} sd=2 ver=3 console=ttyS0,115200n8 root=/dev/mmcblk%dp2 \
					rw rootfstype=ext4 rootwait", fs_info);
			
			ret = run_command(msg, 0);
			
			if(ret)
				goto EXIT;
	
			ret = run_command ("bootm 0x81000000 - 0x80000000", 0);
			
			if(ret)
				goto EXIT;

		}
	}

EXIT:
	return ret;

}
#endif

int run_mmc_func(int boot_mmc, char *dtbname, int fs_info)
{
	char msg[MAX_SIZE_256BYTE] = {0};
	char dtb_info[MAX_SIZE_64BYTE] = {0};
	char kernel_info[MAX_SIZE_64BYTE] = {0};
	char rb_str[MAX_SIZE_64BYTE] = {0};
	char fb_str[MAX_SIZE_64BYTE] = {0};
	char if_str[MAX_SIZE_64BYTE] = {0};
	int ret = 0;
	unsigned int fastboot = 0;
	unsigned int robust = 0;
	unsigned int realtime = 0;
	unsigned int ifnames = 0;

        if (getenv("fastboot") == NULL) diag_set_env_conf_u32("fastboot", 0);

        if (getenv("robust") == NULL) diag_set_env_conf_u32("robust", 0);

        if (getenv("realtime") == NULL) diag_set_env_conf_u32("realtime", 0);

        if (getenv("ifnames") == NULL) diag_set_env_conf_u32("ifnames", 0);

        fastboot = diag_get_env_conf_u32 ("fastboot");
        robust = diag_get_env_conf_u32 ("robust");
        realtime = diag_get_env_conf_u32 ("realtime");
        ifnames = diag_get_env_conf_u32 ("ifnames");

	sprintf(dtb_info, "fatload mmc %d:1 0x80000000 %s", boot_mmc, dtbname);

	if (realtime){
                sprintf(kernel_info, "fatload mmc %d:1 0x81000000 uImage-rt", boot_mmc);
        }else{
                sprintf(kernel_info, "fatload mmc %d:1 0x81000000 uImage", boot_mmc);
        }
        //printf("%s\n", kernel_info);

	if (run_command (dtb_info, 0) == 0) {
		if (run_command (kernel_info, 0) == 0) {

			if (robust) 
			        sprintf(rb_str ,"fsck.mode=force fsck.repair=yes");
			if (fastboot)  
			        sprintf(fb_str ,"quiet lpj=4980736 systemd.show_status=0");    

                        if (ifnames == 0) 
                                sprintf(if_str ,"net.ifnames=0 biosdevname=0");

			sprintf(msg, "setenv bootargs mac=${ethaddr} sd=2 ver=3 console=ttyS0,115200n8 root=/dev/mmcblk%dp2 \
				rw %s %s %s rootfstype=ext4 rootwait", fs_info, if_str, rb_str, fb_str);

                        //printf("%s\n", msg);

			ret = run_command(msg, 0);
			
			if(ret)
				goto EXIT;
	
			ret = run_command ("bootm 0x81000000 - 0x80000000", 0);
			
			if(ret)
				goto EXIT;
		}
	}

EXIT:
	return ret;

}

void run_OS_test_func(char * testmode, char * dtbname)
{
        char buf[128];
        char *s1;
        run_command("mmc dev 1", 0);

        if(testmode != NULL)
                sprintf(buf,"setenv bootargs mac=${ethaddr} sd=2 ver=3 console=ttyS0,115200 \
                        root=/dev/mmcblk1p2 rw net.ifnames=0 biosdevname=0 rootfstype=ext4 rootwait testmode=%s ",testmode);

        run_command(buf, 0);	
        
        sprintf(buf, "fatload mmc 1:1 0x80000000 %s", dtbname);
        run_command(buf, 0);	

        run_command("fatload mmc 1:1 0x81000000 uimage", 0);
#if 1
        s1 = getenv("overlay_flag");

        if (strcmp(s1, "v1") == 0) {
                setenv("overlay_flag", "debug");
                run_command("saveenv", 0);
        }

        run_command("bootm 81000000 - 80000000", 0);

        setenv("overlay_flag", "v1");
        run_command("saveenv", 0);
#endif
			
        printf("\nBoot to SD Fail\n");
        printf("\nRetry again\n");
        
        halt(DCAN_MP, HALT_MODE2);        
}

#if 0
int do_run_mmc_func(board_infos *sys_info)
{
	char msg[MAX_SIZE_256BYTE] = {0};
	char dtb_info[MAX_SIZE_64BYTE] = {0};
	char kernel_info[MAX_SIZE_64BYTE] = {0};
	char fs_dev[MAX_SIZE_32BYTE] = {0};
	int fs_info = 0;
	int ret = 0;
	int fastboot = 0;

	sprintf(dtb_info, "fatload mmc %d:1 0x80000000 %s", sys_info->sys_mmc, sys_info->dtbname);
	sprintf(kernel_info, "fatload mmc %d:1 0x81000000 uImage", sys_info->sys_mmc);

	if (run_command (dtb_info, 0) == 0) {
		if (run_command (kernel_info, 0) == 0) {

			if (sys_info->ext_mmc1 >= 0) {
				sprintf(fs_dev, "mmc dev %d", sys_info->ext_mmc1);

				if (run_command(fs_dev, 0) == 0)
					fs_info = 1;
				else
					fs_info = 0;
			} else {

				fs_info = 0;
			}

			if (fastboot)  
				sprintf(msg, "setenv bootargs mac=${ethaddr} sd=2 ver=3 console=ttyS0,115200n8 root=/dev/mmcblk%dp2 \
					rw quiet lpj=4980736 rootfstype=ext4 rootwait", fs_info);
			else
				sprintf(msg, "setenv bootargs mac=${ethaddr} sd=2 ver=3 console=ttyS0,115200n8 root=/dev/mmcblk%dp2 \
					rw rootfstype=ext4 rootwait", fs_info);
			
			ret = run_command(msg, 0);
			
			if(ret)
				goto EXIT;
	
			ret = run_command ("bootm 0x81000000 - 0x80000000", 0);
			
			if(ret)
				goto EXIT;

		}
	}

EXIT:
	return ret;

}
#endif
