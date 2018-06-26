/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    test.h

    Headers Test function.
    
    2017-09-09	HsienWen Tsai.			Create it.
*/

#ifndef _TEST_H
#define _TEST_H

#ifdef SDRAM_MP
//void SDRAM_MPTEST(UINT8 led);
void Runbar(u32 count, u32 speed);
#endif  //SDRAM_MP
void BIOS_SET_TEST_FLAG(int mm_flag);
void GetSN_MAC(int mode);
void BIOS_MPTEST_OK(void);
void moxa_board_info(int mode);
#endif  //_TEST_H

