#include <common.h>
#include <i2c.h>
#include "ds1374_wdt.h"
#include "../cmd_bios.h"
#include "moxa_lib.h"
/* ---------- */
#undef DEBUG_WDT_DS1374
// #define DEBUG_WDT_DS1374

#ifdef DEBUG_WDT_DS1374
#define dbg_wdt(fmt,args...) printf("DBG: %s()[%d]: " fmt, __func__, __LINE__, ##args)
#else
#define dbg_wdt(fmt,args...)
#endif
/* ---------- */

#define I2C_WDT_ADDR 0x68

#define DS1374_WD_COUNTER_0_ADDR 0x04
#define DS1374_WD_COUNTER_1_ADDR 0x05
#define DS1374_WD_COUNTER_2_ADDR 0x06

#define DS1374_CONTROL_ADDR 0x07

void wdt_stop (void)
{
	unsigned char reg;

	reg = i2c_reg_read (I2C_WDT_ADDR, DS1374_CONTROL_ADDR);
	dbg_wdt ("Get WDT Control Register: %02X\n", reg);

	reg &= ~(1 << 6);
	i2c_reg_write (I2C_WDT_ADDR, DS1374_CONTROL_ADDR, reg);
	dbg_wdt ("Disable WDT\n");
}

void wdt_set_flag (int flag)
{
	i2c_reg_write (I2C_WDT_ADDR, 0x3, flag);
}

void wdt_start (unsigned long timeout_ms)
{
	unsigned long t;
	
	dbg_wdt ("Set WDT Timeout to %ldms\n", timeout_ms);

      t = timeout_ms * 4096 / 1000;
	dbg_wdt ("Set WD Counter: %06lX\n", t);

      i2c_reg_write (I2C_WDT_ADDR, DS1374_WD_COUNTER_0_ADDR, (t & 0xFF));
	dbg_wdt ("Set WD_COUNTER_BYTE_0 to %02lX\n", (t & 0xFF));

      i2c_reg_write (I2C_WDT_ADDR, DS1374_WD_COUNTER_1_ADDR, ((t >> 8) & 0xFF));
	dbg_wdt ("Set WD_COUNTER_BYTE_1 to %02lX\n", ((t >> 8) & 0xFF));

      i2c_reg_write (I2C_WDT_ADDR, DS1374_WD_COUNTER_2_ADDR, ((t >> 16) & 0xFF));
	dbg_wdt ("Set WD_COUNTER_BYTE_2 to %02lX\n", ((t >> 16) & 0xFF));

      i2c_reg_write (I2C_WDT_ADDR, DS1374_CONTROL_ADDR, 0x61);
	dbg_wdt ("Enable WDT\n");
}

void check_ds1374_battery (int model) {

	if ((i2c_reg_read (I2C_DS1374_ADDR, DS1374_SR_ADDR) & DS1374_CTL_BIT_EN_OSC) == DS1374_CTL_BIT_EN_OSC) {
		printf ("\r\nRTC Battery Fail...\r\n");
		halt(model, HALT_MODE2);
	}

	return;
}
