/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

//R03	04/03/2014   SUN   Modify RTC backup register access method
//R02 	02/21/2014   SUN   Added RTC battery check function
//R01   10/16/2013   DON   Enable or Disable RTC function
//R00   10/08/2013   DON   First initial

#include <common.h>
#include <i2c.h>
#include <bios.h>
#include <model.h>

#include "rtc.h"
#include "rtc_tps65910.h"

#define RTC_BASE_ADDR 0x2d

#define RTC_SECONDS			0x00
#define RTC_MINUTES			0x01
#define RTC_HOURS			0x02
#define RTC_DAYS			0x03
#define RTC_MONTHS			0x04
#define RTC_YEARS			0x05
#define RTC_WEEKS			0x06
#define RTC_SECONDS_ALARM		0x08
#define RTC_MINUTES_ALARM		0x09
#define RTC_HOURS_ALARM			0x0A
#define RTC_DAYS_ALARM			0x0B
#define RTC_MONTHS_ALARM		0x0C
#define RTC_YEARS_ALARM			0x0D

#define RTC_CTRL			0x10
#define RTC_STATUS			0x11
#define RTC_INTERRUPTS			0x12

#define RTC_DEVCTRL			0x3F
#define RTC_BCK1			0x17


static uchar rtc_read (uchar reg);
static void rtc_write (uchar reg, uchar val);

void rtc_tps65910_get_date(pDateStruct a)
{
	udelay(5000);
	a->Year = rtc_read(RTC_YEARS);
	BCD(0, &a->Year);

	udelay(5000);
	a->Month = rtc_read(RTC_MONTHS);
	BCD(0,&a->Month);

	udelay(5000);
	a->Date = rtc_read(RTC_DAYS);
	BCD(0,&a->Date);

	udelay(5000);
	a->Day = rtc_read(RTC_WEEKS);
	BCD(0,&a->Day);

	a->Hours = rtc_read(RTC_HOURS);
	udelay(5000);
 	if ( a->Hours & 0x80 ) {		 // PM mode
		a->Hours &= ~0x80;
		BCD(0,&a->Hours);
		a->Hours += 12;
  		if ( a->Hours >= 24 ) {
			a->Hours = 0;
		}
 	} else {
 		BCD(0,&a->Hours);
 	}

	a->Minutes = rtc_read(RTC_MINUTES);
	BCD(0,&a->Minutes);
	udelay(5000);

	a->Seconds = rtc_read(RTC_SECONDS);
	BCD(0,&a->Seconds);
	udelay(5000);

	return;
}

void rtc_tps65910_set_date(DateStruct a)
{
	unsigned char v;
	
	BCD(1,&a.Hours);
	rtc_write(RTC_HOURS, a.Hours);
	udelay(5000);

	BCD(1,&a.Minutes);
	rtc_write(RTC_MINUTES, a.Minutes);
	udelay(5000);

	BCD(1,&a.Seconds);
	rtc_write(RTC_SECONDS, a.Seconds);
	udelay(5000);

	v = (unsigned char)get_week(a.Year, a.Month, a.Date);
 	rtc_write(RTC_WEEKS, v);
	udelay(5000);

	BCD(1,&a.Year);
 	rtc_write(RTC_YEARS, a.Year);
 	udelay(5000);

	BCD(1,&a.Month);
 	rtc_write(RTC_MONTHS, a.Month);
 	udelay(5000);

	BCD(1,&a.Date);
 	rtc_write(RTC_DAYS, a.Date);
 	udelay(5000);

 	return;
}

int rtc_tps65910_init (int mode)
{
	int res = 0;
	ulong reg;

	reg = rtc_read (RTC_DEVCTRL);
	reg &= ~(1<<6); /* RTC Enable */
	reg &= ~(1<<5); /* crystal oscillator */
	rtc_write (RTC_DEVCTRL, reg);

	/* Set to 24-hour mode */
	reg = rtc_read (RTC_CTRL);
	reg &= ~(0x08);
	rtc_write (RTC_CTRL, reg);

	/* Start the RTC */
	reg = rtc_read (RTC_CTRL);
	reg |= 0x01;
	rtc_write (RTC_CTRL, reg);

	return res;
}

int rtc_tps65910_release (int mode)
{
	int res = 0;
	ulong reg;

	/* Start the RTC */
	reg = rtc_read (RTC_CTRL);
	reg &= ~(0x01);
	rtc_write (RTC_CTRL, reg);

	return res;
}

void rtc_tps65910_backup_write (unsigned char reg, unsigned char val)
{
	rtc_write (reg, val);
	udelay(5000);
}

unsigned char rtc_tps65910_backup_read (unsigned char reg)
{
	 return rtc_read(reg);
}

static uchar rtc_read (uchar reg)
{
	return (i2c_reg_read (RTC_BASE_ADDR, reg));
}

static void rtc_write (uchar reg, uchar val)
{
	i2c_reg_write (RTC_BASE_ADDR, reg, val);
}

