/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#ifndef __RTC_H__
#define __RTC_H__

struct _Date_Struct {
	unsigned char	Year;		// 0-99, base on 2000
	unsigned char	Month;		// 1 - 12
	unsigned char	Date;		// 1 - 31
	unsigned char	Day;		// 0 - 6
	unsigned char	Hours;		// 0 - 23
	unsigned char	Minutes;	// 0 - 59
	unsigned char	Seconds;	// 0 - 59
}__attribute__((packed));
typedef struct _Date_Struct DateStruct, *pDateStruct;

unsigned char BCD(int mode, unsigned char *j);
unsigned long date_to_seconds(pDateStruct date);
unsigned long get_week(unsigned long year, unsigned long month, unsigned long date);

int do_RTC_init(void);
int do_rtc_test(void);
int do_rtc_set_enable (void);
int do_RTC_set_time(void);
int do_RTC_set_date(void);
int do_rtc_get_date (pDateStruct a, int chip);
int do_RTC_view_time(void);
int do_RTC_view_date(void);
int do_RTC_set_datetime (void);
int do_RTC_view_datetime (void);
void rtc_backup_write (unsigned char reg, unsigned char val);
unsigned char rtc_backup_read (unsigned char reg);
void RTC_MPTEST(unsigned char led);

#endif //__RTC_H__

