/*  Copyright (C) MOXA Inc. All rights reserved.

	This software is distributed under the terms of the
	MOXA License.  See the file COPYING-MOXA for details.
 */
/*
   test.c

   Test functions.

   2013-10-03	Donald Hsu.			Create it.
 */
#include <common.h>
#include <command.h>
#include <bios.h>
#include <model.h>
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "moxa_mem.h"
#include <i2c.h>
#include <ns16550.h>


static char RUNBAR[4] = {'\\', '|', '/', '-'};
unsigned long moxa_test_pattern[MAX_TEST_PATTERN_SIZE] = {0x00000000, 0x55555555, 0x5A5A5A5A, 0xAAAAAAAA, 0xA5A5A5A5, 0xFFFFFFFF};

DECLARE_GLOBAL_DATA_PTR;

int ram_test_for_pattern(unsigned long address, unsigned long length, unsigned int mode)
{
	unsigned long j;
	int i = 0;

	printf("\r\n");

	if ((mode != BIT8_MODE) && (mode != BIT16_MODE) && (mode != BIT32_MODE)){
		printf("Parameter 3 Fail\r\n");
		return 1;
	}

	for (j = 0; j < length; j += (1 << mode)) {
		if (check_if_press_character(ESC)) {
			printf("\r\nUser End The Test\r\n");
			return 2;
		}

		if ((j % 0x100000) == 0)
			printf("Mode[%d] Write %08lx...\r\n", mode, address+j);

		for (i = 0; i < 6; i++) {
			if (mode == BIT8_MODE){
				VPchar(address + j) = (unsigned char) moxa_test_pattern[i] & 0xff;

				if (((unsigned char)moxa_test_pattern[i] & 0xff) != VPchar(address + j)) {
					printf("\r\n[%lx]Test Byte Fail![%x][%x]\r\n", address + j, VPchar(address + j), \
										(unsigned char)moxa_test_pattern[i] & 0xff);
					return 1;
				}

			} else if (mode == BIT16_MODE) {
				VPshort(address + j) = (unsigned short)moxa_test_pattern[i] & 0xffff;
				if (((unsigned short)moxa_test_pattern[i] & 0xffff) != VPshort(address + j)) {
					printf("\r\n[%lx]Test Short Fail![%x][%x]\r\n", address + j, VPshort(address + j), \
										(unsigned short)moxa_test_pattern[i] & 0xffff);
					return 1;
				}

			} else if (mode == BIT32_MODE) {
				VPlong(address + j) = (unsigned long)moxa_test_pattern[i] & 0xffffffff;
				
				if (((unsigned long)moxa_test_pattern[i] & 0xffffffff) != VPlong(address + j)) {
					printf("\r\n[%lx]Test Long Fail![%lx][%lx]\r\n", address + j, VPlong(address + j), \
										(unsigned long)moxa_test_pattern[i] & 0xffffffff);
					return 1;
				}
			}

		}


	}

	printf("\r\n");

	return 0;
}

/**     \brief
 *
 *      Test RAM with 32 bits pattern, the pattern is the same as the address[param in]
 *
 *      \param[in]      address : test address
 *      \param[in]      length : test length
 *      \param[in]      mode : test mode 1: byte,  2: short, 4: long
 *      \retval         0 : OK, 1 : Fail, 2 : Escape
 *
 */
int ram_test_for_address(unsigned long address, unsigned long length, unsigned int mode)
{
	unsigned long j;

	if ((mode != BIT8_MODE) && (mode != BIT16_MODE) && (mode != BIT32_MODE)) {
		printf("Parameter 3 Fail\r\n");
		return 1;
	}

	printf("Write:\r\n");

	for (j = 0; j < length; j += mode) {
		if (check_if_press_character(ESC)) {
			printf("\r\nUser End The Test\r\n");
			return 2;
		}

		if ((j % 0x100000) == 0)
			printf("Mode[%d] Write %08lx...\r\n", mode, address + j);

		if (mode == BIT8_MODE)
			VPchar(address + j) = (unsigned char)(address + j);
		else if (mode == BIT16_MODE)
			VPshort(address + j) = (unsigned short)(address + j);
		else if (mode == BIT32_MODE)
			VPlong(address + j) = (unsigned long)(address + j);

	}

	printf("\r\nRead:\r\n");

	for (j = 0; j < length; j += mode) {
		if ((j % 0x100000) == 0)
			printf("Mode[%d] Read %08lx...\r\n", mode, address + j);

		if (mode == BIT8_MODE) {
			if (VPchar(address + j) != (unsigned char)(address + j)){
				printf("\r\n[%lx]Ram Test Fail![%x][%x]\r\n", (address + j), VPchar(address + j), (unsigned char)(address+j));
				return 1;
			}
		} else if (mode == BIT16_MODE) {
			if (VPshort(address + j) != (unsigned short)(address + j)) {
				printf("\r\n[%lx]Ram Test Fail![%x][%x]\r\n", (address + j), VPshort(address + j), (unsigned short)(address+j));
				return 1;
			}
                }else if (mode == BIT32_MODE) {
			if (VPlong(address + j) != (unsigned long)(address + j)) {
				printf("\r\n[%lx]Ram Test Fail![%lx][%lx]\r\n", (address + j), VPlong(address + j), (unsigned long)(address + j));
				return 1;
			}
		}
	}

	printf("\r\n");
	return 0;
}

int do_address_test(unsigned long start_addr, unsigned long length)
{
	int ret;

	while(1){
		if ((ret = ram_test_for_address(CONFIG_SYS_MEMTEST_START, length, BIT8_MODE)) != 0){
			if (ret == 2)
				printf("*** SDRAM BYTE TEST Abort!\r\n");
			else
				printf("*** SDRAM BYTE TEST FAIL!!\r\n");
			
			break;
		}

		if ((ret = ram_test_for_address(CONFIG_SYS_MEMTEST_START, length, BIT16_MODE)) != 0){
			if (ret == 2)
				printf("*** SDRAM SHORT TEST Abort!\r\n");
			else
				printf("*** SDRAM SHORT TEST FAIL!!\r\n");
			

			break;
		}

		if ((ret = ram_test_for_address(CONFIG_SYS_MEMTEST_START, length, BIT32_MODE)) != 0){
			if (ret == 2)
				printf("*** SDRAM LONG TEST Abort!\r\n");
			else
				printf("*** SDRAM LONG TEST FAIL!!\r\n");

			break;
		}
	}

	return ret;

}

#ifdef SDRAM_MP
/*
 * SDRAM  test
 *  Test the first byte in every MB
 */
void SDRAM_MPTEST(int led)
{

	int ErrFlag = 0;
	UINT32 start_addr;
	UINT32 i,k;
	UINT32 CompareContent;

	start_addr = CONFIG_SYS_MEMTEST_START;

	printf("[0x%04x]SDRAM Access Test...\r\n",led);

	printf("SDRAM test from 0x%08lx to 0x%08lx ...\r\n",(unsigned long)start_addr,(CONFIG_SYS_SDRAM_BASE+gd->ram_size-CONFIG_SYS_MEMTEST_RESERVED));

	for (i = start_addr, k = 0; i < (CONFIG_SYS_SDRAM_BASE + gd->ram_size - CONFIG_SYS_MEMTEST_RESERVED); i += 0x00100000) {
		VPlong(i) = 0x5AA55AA5;
		CompareContent = VPlong(i);
		if (CompareContent != 0x5AA55AA5) {
			printf("error address:%x, error data:%x, correct data:%x\r\n", start_addr, CompareContent, 0x5AA55AA5);
			ErrFlag++;
			break;
		}

		runbar(k++, 0x1000);
	}

	if (ErrFlag) {
		printf("SDRAM Test *** FAIL ***\r\n");
		halt(led, HALT_MODE1);
	} else {
		printf("SDRAM Test *** OK ***\r\n");
		return;
	}
}

#endif

void runbar(u32 count, u32 speed)
{
	static int i = 0;

	if ((count % speed) == 0)
		printf("%c\b", RUNBAR[(i++)%4]);

	return;
}

