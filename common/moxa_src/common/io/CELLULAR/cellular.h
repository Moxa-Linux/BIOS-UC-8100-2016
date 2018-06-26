/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    uart.h

    Definitions of TI AM335X CELLULAR controller.

    2016-12-22  HsienWen Tsai
*/

#ifndef _CELLULAR_H
#define _CELLULAR_H

int diag_do_check_sim_card(board_infos *sys_info, struct _rf_info *rf_info, int sim_side);
int diag_do_cellular_for_rf_test(board_infos *sys_info, struct _rf_info *rf_info, int port);
void USB_CELLULAR_MPTEST(board_infos *sys_info, struct _rf_info *rf_info, u8 led);

#endif
