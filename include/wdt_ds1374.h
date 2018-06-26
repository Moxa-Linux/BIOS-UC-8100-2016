#ifndef _DS1374_WDT_H_
#define _DS1374_WDT_H_

#define DS1374_WDT_ADDR			0x68

#define DS1374_WDT_COUNTER_0_ADDR	0x04
#define DS1374_WDT_COUNTER_1_ADDR	0x05
#define DS1374_WDT_COUNTER_2_ADDR	0x06
#define DS1374_CONTROL_ADDR		0x07

#define WDT_1MS				0x4
#define WDT_5S				0x50
#define WDT_10S				0xA0
#define DS1374_WDT_ENABLE		0x40

void wdt_stop (void);

#endif  /* _DS1374_WDT_H_ */

