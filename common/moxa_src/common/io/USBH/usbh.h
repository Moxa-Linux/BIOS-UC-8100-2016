/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    usbh.h

    Definitions of TI AM335X USB controller.
    
    2017-10-02	Hsien Wen Tsai
*/

#ifndef _USBH_H
#define _USBH_H

#include "sys_info.h"

int do_usb_detect_test(board_infos *sys_info);
void USB_HOST_MPTEST(u8 led);

#endif  //_USBH_H
