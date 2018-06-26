/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

/*
    ethernet.h

    Definitions of TI AM335X MAC.
    The Mii functions are also included in it.

    2016-12-22  HsienWen Tsai
*/

#ifndef _ETHERNET_H
#define _ETHERNET_H

int do_eth_phy_power_saving_analog_off_1 (unsigned long enable);
int do_eth_phy_power_saving_power_down_1 (unsigned long enable);
int do_eth_phy_power_saving_analog_off_2 (unsigned long enable);
int do_eth_phy_power_saving_power_down_2 (unsigned long enable);
int do_eth1_100_cmd_test(void);
int do_eth2_100_cmd_test(void);
int do_eth1_10_cmd_test(void);
int do_eth2_10_cmd_test(void);
int do_eth_100_burnin_test(void);
void LAN_100M_CROSSCABLE_MPTEST(int led, int i);
void LAN_10M_CROSSCABLE_MPTEST(int led, int i);

#endif
