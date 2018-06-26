/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    mem.h

    Memory Test function.
    
    2017-01-03	HsienWen Tsai.			Create it.
*/

#ifndef _MEM_H
#define _MEM_H

enum {
	BIT8_MODE = 0,
	BIT16_MODE,
	BIT32_MODE,
};

#define MAX_TEST_PATTERN_SIZE	6

int ram_test_for_pattern(unsigned long address, unsigned long length, unsigned int mode);
int ram_test_for_address(unsigned long address, unsigned long length, unsigned int mode);
int do_address_test(unsigned long start_addr, unsigned long length);
void SDRAM_MPTEST(int led);
void runbar(u32 count, u32 speed);

#endif
