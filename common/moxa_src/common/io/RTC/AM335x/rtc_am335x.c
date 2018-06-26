/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

//R01   10/16/2013   DON   Enable or Disable RTC function
//R00   10/08/2013   DON   First initial

#include <common.h>
#include <bios.h>
#include <model.h>

#include <asm/io.h>

#include "rtc.h"
#include "types.h"
#include "rtc_am335x.h"

#define RTC_BASE_ADDR 0x44E3E000

#define RTC_SECONDS			0x00
#define RTC_MINUTES			0x04
#define RTC_HOURS			0x08
#define RTC_DAYS			0x0C
#define RTC_MONTHS			0x10
#define RTC_YEARS			0x14
#define RTC_WEEKS			0x18
#define RTC_SECONDS_ALARM		0x20
#define RTC_MINUTES_ALARM		0x24
#define RTC_HOURS_ALARM			0x28
#define RTC_DAYS_ALARM			0x2C
#define RTC_MONTHS_ALARM		0x30
#define RTC_YEARS_ALARM			0x34

#define RTC_CTRL			0x40
#define RTC_STATUS			0x44
#define RTC_INTERRUPTS			0x48

#define RTC_OSC				0x54

#define RTC_KICK0R			0x6C
#define RTC_KICK1R			0x70
#define RTC_KICK0R_VALUE		0x83E70B13
#define RTC_KICK1R_VALUE		0x95A4F1E0

static ulong rtc_read (ulong reg);
static void rtc_write (ulong reg, ulong val);

void rtc_am335x_get_date(pDateStruct a)
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

void rtc_am335x_set_date(DateStruct a)
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

int rtc_am335x_init (int mode)
{
	int res = 0;
	ulong reg;

	/* Enable the module clock domains */
	rtc_write (RTC_OSC, (1 << 3) | (1 << 6));
	reg = rtc_read (RTC_OSC);
	reg &= ~(1<<4);
	rtc_write (RTC_OSC, reg);

	/* Enable the RTC module using CTRL_REG.RTC_disable */
	reg = rtc_read (RTC_CTRL);
	reg &= ~(0x40);
	rtc_write (RTC_CTRL, reg);

	/* Write to the kick registers (KICK0R, KICK1R) in the RTC */
	rtc_write (RTC_KICK0R, RTC_KICK0R_VALUE);
	rtc_write (RTC_KICK1R, RTC_KICK1R_VALUE);

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

int rtc_am335x_release (int mode)
{
	int res = 0;
	ulong reg;

	/* Stop the RTC */
	reg = rtc_read (RTC_CTRL);
	reg &= ~(0x01);
	rtc_write (RTC_CTRL, reg);

	return res;
}

static ulong rtc_read (ulong reg)
{
	ulong val;

	val = *(volatile unsigned long*)(RTC_BASE_ADDR+reg);
	return (val);
}

static void rtc_write (ulong reg, ulong val)
{
	(*(volatile unsigned long*)(((ulong)RTC_BASE_ADDR)+reg)) = val;
}

