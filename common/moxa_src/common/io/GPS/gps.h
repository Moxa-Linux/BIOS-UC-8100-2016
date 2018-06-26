/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    gps.h

    Definitions of TI AM335X GPS controller.

    2016-12-22  HsienWen Tsai
*/

#ifndef _GPS_H
#define _GPS_H

int gps_mptest (struct usb_device *dev, struct _rf_info *rf_info);
int do_gps_test(board_infos *sys_info, struct _rf_info *rf_info);

#endif
