//R01   12/15/2015   SHA   Add WDT test function.
//R00   05/04/2015   DON   Add WDT test function.

#ifndef __WDT_DIAG_H__
#define __WDT_DIAG_H__

int do_wdt_test (unsigned long second);
void diag_do_wdt_test (void);
void do_set_wdt_flag(void); //R01
void do_set_wdt_unflag(void); //R01
void WDT_MPTEST(int mode, unsigned char led); //R01

#endif /* __WDT_DIAG_H__ */

