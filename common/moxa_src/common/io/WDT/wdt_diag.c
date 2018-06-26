
#include <common.h>

#include "../cmd_bios.h"
#include "wdt_diag.h"
#include "moxa_lib.h"
#include "ds1374_wdt.h"
#include "i2c.h"


int do_wdt_test (unsigned long second)
{
	int res = 0;

	wdt_start (second * 1000);

	return res;
}

void do_set_wdt_flag (void)
{
	wdt_set_flag(ENABLE_WDT_SUCCESS);
	return ;
}

void do_set_wdt_unflag (void)
{
	wdt_set_flag(ENABLE_WDT_FAILED);
	return ;
}
#if 0
void diag_do_wdt_test (void)
{
	unsigned int timeout;
	
	if (diag_get_value ("WDT Timeout :", &timeout, 1, 10, DIAG_GET_VALUE_DEC_MODE)) {
		return;
	}

	do_wdt_test (timeout);

	while (timeout) {
		printf ("%d...", timeout--);
		udelay (1000 * 1000);
	}
}
#endif

void WDT_MPTEST (int mode, unsigned char led)
{
	int cnt = 0;

        i2c_set_bus_num(0);

	printf("[0x%04x]WDT Test...\r\n",led);


	if (mode != FG_MODE)
		do_set_wdt_flag();

	do_wdt_test(2);

	for (cnt = 0; cnt < 5; cnt++)
		udelay(1000*1000);

	if (mode != FG_MODE)
		do_set_wdt_unflag();

	printf("WDT Testing fail!!\r\n");

	halt(led, HALT_MODE1);

	return ;

}
