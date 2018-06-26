/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <bios.h>
#include <model.h>
#include <malloc.h>
#include <rtc.h>
#include "moxa_rtc.h"
#include "../cmd_bios.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#ifdef RTC_AM335X
#include "rtc_am335x.h"
#endif

#ifdef RTC_TPS65910
#include "rtc_tps65910.h"
#endif

static unsigned long DateToDay[12]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
//R03 static unsigned char sys_daysofmonth[12]={31,28,31,30,31,30,31,31,30,31,30,31};
static char day_string[7][4]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

unsigned char BCD(int mode, unsigned char *j)
{ 
	unsigned char temp10,temp1;
	
	if (mode == 1) {
		temp10 = *j/10;
		temp1 = *j%10;
		*j = temp1 | (temp10<<4);
	} else {
		temp10 = (*j & 0xf0)>>4;
		temp1 = (*j & 0xf);
		*j = temp1+(temp10*10);
	}
	return *j;
}

unsigned long date_to_seconds(pDateStruct date)
{
	unsigned long	ret=0, day;

	ret = (unsigned long)date->Seconds;
	ret += ((unsigned long)date->Minutes * 60);
	ret += ((unsigned long)date->Hours * 3600);
	day = (unsigned long)date->Year * 365;
	if ( date->Year )
	{
		day += (((unsigned long)date->Year-1) / 4);
		day -= (((unsigned long)date->Year-1) / 100);
	}
	day += DateToDay[date->Month-1];
	if ( !(date->Year & 0x03) && (date->Year % 100) && (date->Month > 2) )
	{
		day++;
	}
	day += ((unsigned long)date->Date - 1);
	ret += (day * 24 * 3600);
	return ret;
}

unsigned long get_week(unsigned long year, unsigned long month, unsigned long date)
{
	unsigned long	ret;

	ret = year * 365;
	if ( year ) {
		ret += (((unsigned long)year-1) / 4);
		ret -= (((unsigned long)year-1) / 100);
	}
	ret += DateToDay[month-1];
	if ( !(year & 0x03) && (year % 100) && (month > 2) ) {
		ret++;
	}
	ret += ((unsigned long)date - 1);
	ret %= 7;
	return ret;
}

int rtc_init (int mode)
{
	int res = 0;

#ifdef RTC_AM335X
	rtc_am335x_init (mode);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_init (mode);
#endif //RTC_TPS65910

	return res;
}

int rtc_release (int mode)
{
	int res = 0;

#ifdef RTC_AM335X
	rtc_am335x_release (mode);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_release (mode);
#endif //RTC_TPS65910

	return res;
}

int do_RTC_init(void)
{
	rtc_init(0);	
	return DIAG_OK;
}

//R01-start
int do_rtc_set_enable (void)
{
	int res = DIAG_OK;
	int rtc_count;
	unsigned int value;
	unsigned int enable;
	struct RTC_LINK {
		int id;
		char *name;
		int (*init)(int);
		int (*release)(int);
		struct RTC_LINK *next;
	} ;
	struct RTC_LINK *rtc_link;
	struct RTC_LINK __maybe_unused *rtc_this = NULL;
	struct RTC_LINK *temp = NULL;

	rtc_count = 0;
	rtc_link = (struct RTC_LINK *) malloc (sizeof (struct RTC_LINK));
	memset (rtc_link, 0, sizeof (struct RTC_LINK));
	rtc_this = rtc_link;

#ifdef RTC_AM335X
	rtc_count++;
	temp = (struct RTC_LINK *) malloc (sizeof (struct RTC_LINK));
	memset (temp, 0, sizeof (struct RTC_LINK));
	temp->id = rtc_count;
	temp->name = strdup ("AM335X");
	temp->init = rtc_am335x_init;
	temp->release = rtc_am335x_release;
	rtc_this->next = temp;
	rtc_this = temp;
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_count++;
	temp = (struct RTC_LINK *) malloc (sizeof (struct RTC_LINK));
	memset (temp, 0, sizeof (struct RTC_LINK));
	temp->id = rtc_count;
	temp->name = strdup ("TPS65910");
	temp->init = rtc_tps65910_init;
	temp->release = rtc_tps65910_release;
	rtc_this->next = temp;
	rtc_this = temp;

#endif //RTC_TPS65910

	printf ("RTC Device: ");
	if (rtc_count == 0) {
		printf ("Not support\n");
		free (rtc_link);
		return res;
	}
	temp = rtc_link->next;
	do {
		printf ("[%d]:%s ", temp->id, temp->name);
		temp = temp->next;
	} while (temp != NULL);
	if (diag_get_value ("Select:", &value, 1, rtc_count, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC) {
		res = DIAG_ESC;
		goto diag_do_rtc_set_enable_end;
	}
	printf ("\n[0]: Disable, [1]:Enable");
	if (diag_get_value ("Select:", &enable, 0, 1, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC) {
		res = DIAG_ESC;
		goto diag_do_rtc_set_enable_end;
	}

	temp = rtc_link->next;
	do {
		if ((int)value == temp->id) {
			break;
		}
		temp = temp->next;
	} while (temp != NULL);

	if (enable) {
		if (temp->init != NULL) {
			(temp->init)(0);
		}
	} else {
		if (temp->release != NULL) {
			(temp->release)(0);
		}
	}

diag_do_rtc_set_enable_end:
	do {
		temp = rtc_link;
		rtc_link = rtc_link->next;
		if (temp->name) {
			free (temp->name);
		}
		free (temp);
	} while (rtc_link != NULL);
	
	return res;
}
//R01-end

int do_RTC_set_time(void)
{
	DateStruct __maybe_unused time_info;
#ifndef RTC_DS1374
	DateStruct datetime_info;
#endif /* RTC_DS1374 */

	unsigned int value;

	if (diag_get_value("Now hour", &value, 0, 23, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC)
		return DIAG_ESC;

	time_info.Hours = (unsigned char)value;
	
	if (diag_get_value("Now mintue", &value, 0, 59, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC)
		return DIAG_ESC;

	time_info.Minutes = (unsigned char)value;
	
	if (diag_get_value("Now second", &value, 0, 59, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC)
		return DIAG_ESC;

	time_info.Seconds = (unsigned char)value;

#ifdef RTC_AM335X
	rtc_am335x_get_date (&datetime_info);
	datetime_info.Hours = time_info.Hours;
	datetime_info.Minutes = time_info.Minutes;
	datetime_info.Seconds = time_info.Seconds;
	rtc_am335x_set_date (datetime_info);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_get_date (&datetime_info);
	datetime_info.Hours = time_info.Hours;
	datetime_info.Minutes = time_info.Minutes;
	datetime_info.Seconds = time_info.Seconds;
	rtc_tps65910_set_date (datetime_info);
#endif //RTC_TPS65910

	return DIAG_OK;
}

int do_RTC_set_date(void)
{
	DateStruct __maybe_unused time_info;
#ifndef RTC_DS1374
	DateStruct datetime_info;
#endif /* RTC_DS1374 */

	unsigned int value;

	if (diag_get_value("Now year", &value, 2000, 2099, DIAG_GET_VALUE_DEC_MODE) ==  DIAG_ESC)
		return DIAG_ESC;

	time_info.Year = (unsigned char)(value-2000);
	
	if (diag_get_value("Now month", &value, 1, 12, DIAG_GET_VALUE_DEC_MODE) ==  DIAG_ESC)
		return DIAG_ESC;

	time_info.Month = (unsigned char)value;
	
	if ( diag_get_value("Now date", &value, 1, 31, DIAG_GET_VALUE_DEC_MODE) ==  DIAG_ESC)
		return DIAG_ESC;

	time_info.Date = (unsigned char)value;

#ifdef RTC_AM335X
	rtc_am335x_get_date (&datetime_info);
	datetime_info.Year = time_info.Year;
	datetime_info.Month = time_info.Month;
	datetime_info.Date = time_info.Date;
	rtc_am335x_set_date (datetime_info);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_get_date (&datetime_info);
	datetime_info.Year = time_info.Year;
	datetime_info.Month = time_info.Month;
	datetime_info.Date = time_info.Date;
	rtc_tps65910_set_date (datetime_info);
#endif //RTC_TPS65910

	return DIAG_OK;
}

int do_RTC_set_datetime (void)
{
	DateStruct time_info;
#ifndef RTC_DS1374
	DateStruct datetime_info;
#endif /* RTC_DS1374 */
	
	unsigned int value;

#ifdef RTC_DS1374
	struct rtc_time rtc_datetime;
#endif /* RTC_DS1374 */

	if (diag_get_value("Now year", &value, 2000, 2099, DIAG_GET_VALUE_DEC_MODE) ==  DIAG_ESC)
		return DIAG_ESC;

	time_info.Year = (unsigned char)(value-2000);
	
	if (diag_get_value("Now month", &value, 1, 12, DIAG_GET_VALUE_DEC_MODE) ==  DIAG_ESC)
		return DIAG_ESC;

	time_info.Month = (unsigned char)value;
	
	if (diag_get_value("Now date", &value, 1, 31, DIAG_GET_VALUE_DEC_MODE) ==  DIAG_ESC)
		return DIAG_ESC;

	time_info.Date = (unsigned char)value;

	if (diag_get_value("Now hour", &value, 0, 23, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC)
		return DIAG_ESC;

	time_info.Hours = (unsigned char)value;
	
	if (diag_get_value("Now mintue", &value, 0, 59, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC)
		return DIAG_ESC;

	time_info.Minutes = (unsigned char)value;
	
	if (diag_get_value("Now second", &value, 0, 59, DIAG_GET_VALUE_DEC_MODE) == DIAG_ESC)
		return DIAG_ESC;

	time_info.Seconds = (unsigned char)value;

#ifdef RTC_AM335X
	rtc_am335x_get_date (&datetime_info);
	datetime_info.Year = time_info.Year;
	datetime_info.Month = time_info.Month;
	datetime_info.Date = time_info.Date;
	datetime_info.Hours = time_info.Hours;
	datetime_info.Minutes = time_info.Minutes;
	datetime_info.Seconds = time_info.Seconds;
	rtc_am335x_set_date (datetime_info);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_get_date (&datetime_info);
	datetime_info.Year = time_info.Year;
	datetime_info.Month = time_info.Month;
	datetime_info.Date = time_info.Date;
	datetime_info.Hours = time_info.Hours;
	datetime_info.Minutes = time_info.Minutes;
	datetime_info.Seconds = time_info.Seconds;
	rtc_tps65910_set_date (datetime_info);
#endif //RTC_TPS65910

//R05-start
#ifdef RTC_DS1374
	rtc_reset (); //R05A
	rtc_get (&rtc_datetime);
	rtc_datetime.tm_year= time_info.Year + 2000;
	rtc_datetime.tm_mon= time_info.Month;
	rtc_datetime.tm_mday= time_info.Date;
	rtc_datetime.tm_hour= time_info.Hours;
	rtc_datetime.tm_min= time_info.Minutes;
	rtc_datetime.tm_sec= time_info.Seconds;
	rtc_set (&rtc_datetime);
#endif /* RTC_DS1374 */
//R05-end

	return DIAG_OK;
}

int do_rtc_get_date (pDateStruct a, int chip)
{
#ifdef RTC_DS1374
	struct rtc_time rtc_datetime;
#endif /* RTC_DS1374 */

	switch (chip) {

#ifdef RTC_AM335X
		case RTC_AM335X:
			rtc_am335x_get_date (a);
			break;
#endif //RTC_AM335X

#ifdef RTC_TPS65910
		case RTC_TPS65910:
			rtc_tps65910_get_date (a);
			break;
#endif //RTC_TPS65910

#ifdef RTC_DS1374
		case RTC_DS1374:
			rtc_get (&rtc_datetime);
			a->Year = rtc_datetime.tm_year;
			a->Month = rtc_datetime.tm_mon;
			a->Date = rtc_datetime.tm_mday;
			a->Hours = rtc_datetime.tm_hour;
			a->Minutes = rtc_datetime.tm_min;
			a->Seconds = rtc_datetime.tm_sec;
#endif /* RTC_DS1374 */
		default:
			return DIAG_FAIL;
			break;
	}
	
	return DIAG_OK;
}

int do_RTC_view_time(void)
{
#ifndef RTC_DS1374 
	DateStruct time_info;
#endif /* RTC_DS1374 */ //R05

	printf ("\r");

#ifdef RTC_AM335X
	rtc_am335x_get_date(&time_info);
	printf("%d:%d:%d       ", time_info.Hours, time_info.Minutes, time_info.Seconds);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_get_date(&time_info);
	printf("%d:%d:%d       ", time_info.Hours, time_info.Minutes, time_info.Seconds);
#endif //RTC_TPS65910

	return DIAG_OK;
}

int diag_do_RTC_view_date(void)
{
	printf ("\r");

#ifdef RTC_AM335X
	rtc_am335x_get_date(&time_info);
	printf("Now : %d-%d-%d,%s ", 2000+time_info.Year, time_info.Month, time_info.Date, day_string[time_info.Day]);
#endif //RTC_AM335X

#ifdef RTC_TPS65910
	rtc_tps65910_get_date(&time_info);
	printf("Now : %d-%d-%d,%s ", 2000+time_info.Year, time_info.Month, time_info.Date, day_string[time_info.Day]);
#endif //RTC_TPS65910

	return DIAG_OK;
}

int do_RTC_view_datetime (void)
{
	struct rtc_time rtc_datetime;
	
	printf ("\r");

	rtc_get (&rtc_datetime);
	printf ("Now : %d-%02d-%02d,%s ", rtc_datetime.tm_year, rtc_datetime.tm_mon, rtc_datetime.tm_mday, day_string[rtc_datetime.tm_wday]);
	printf ("%02d:%02d:%02d       ", rtc_datetime.tm_hour, rtc_datetime.tm_min, rtc_datetime.tm_sec);

	return DIAG_OK;
}

void rtc_backup_write (unsigned char reg, unsigned char val)
{
//	rtc_tps65910_backup_write (reg, val);
}

unsigned char rtc_backup_read (unsigned char reg)
{
	return	0;
}

int do_rtc_test (void)
{
	int ret = 0;
	DateStruct date1, date2;

	do_RTC_init();
	do_rtc_get_date(&date1, RTC_TEST_CHIP);
	
	udelay(1000*1000);
	udelay(1000*1000);
	udelay(1000*1000);
	
	do_rtc_get_date(&date2, RTC_TEST_CHIP);

	if (date_to_seconds(&date1) >= date_to_seconds(&date2))
		ret = -1;
	else
        	ret = 0;

        return ret;
}

void RTC_MPTEST(unsigned char led)
{
	DateStruct	date1, date2;

	printf("[0x%04x]RTC testing ... ",led);

	do_RTC_init();
	
	printf ("Check T2 Flag...");
	
	if (diag_get_mm_flag (T2_FLAG) == 0) {
			rtc_reset();
	}

	do_rtc_get_date(&date1, RTC_TEST_CHIP);
	udelay(1000*1000);
	udelay(1000*1000);
	udelay(1000*1000);
	do_rtc_get_date(&date2, RTC_TEST_CHIP);
	
	if ( date_to_seconds(&date1) >= date_to_seconds(&date2) ){
		printf("\nRTC testing ** FAIL ** \r\n");
		halt(led, HALT_MODE1);
	}else{
		printf("\nRTC testing ** OK ** \r\n.");
	}
	return;
}
