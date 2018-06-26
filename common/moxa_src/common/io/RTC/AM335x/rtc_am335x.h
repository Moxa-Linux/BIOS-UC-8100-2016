/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

//R00   10/08/2013   DON   First initial

#ifndef __RTC_AM335X_H__
#define __RTC_AM335X_H__

void rtc_am335x_get_date(pDateStruct a);
void rtc_am335x_set_date(DateStruct a);
int rtc_am335x_init (int mode);
int rtc_am335x_release (int mode);

#endif //__RTC_AM335X_H__

