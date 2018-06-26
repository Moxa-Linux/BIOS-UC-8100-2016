/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

//R02	04/03/2014   SUN   Modify RTC backup register access method
//R01 	02/21/2014   SUN   Added RTC battery check function
//R00   10/08/2013   DON   First initial

#ifndef __RTC_TPS65910_H__
#define __RTC_TPS65910_H__

void rtc_tps65910_get_date(pDateStruct a);
void rtc_tps65910_set_date(DateStruct a);
int rtc_tps65910_init (int mode);
int rtc_tps65910_release (int mode);
//R02 void rtc_tps65910_backup_write (unsigned char val);	//R01
//R02 unsigned char rtc_tps65910_backup_read (void);		//R01
void rtc_tps65910_backup_write (unsigned char reg, unsigned char val);	//R02
unsigned char rtc_tps65910_backup_read (unsigned char reg);				//R02
#endif //__RTC_TPS65910_H__

