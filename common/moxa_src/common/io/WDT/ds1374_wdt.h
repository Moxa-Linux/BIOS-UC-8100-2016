#ifndef __WDT_DS1374_H__
#define __WDT_DS1374_H__

#define ENABLE_WDT_SUCCESS	0x40
#define ENABLE_WDT_FAILED	0x50

void wdt_stop (void);
void wdt_start (unsigned long timeout_ms);
void wdt_set_flag (int flag);  //R01
void check_ds1374_battery (int model) ;
#endif /* __WDT_DS1374_H__ */

