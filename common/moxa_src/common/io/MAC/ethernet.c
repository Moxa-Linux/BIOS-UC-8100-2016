/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    ethernet.c

    This file includes functions of TI AM335X MAC.
    The Mii functions are also included in it.
    
    2014-01-09	Sun Lee
*/

#include <common.h>
#include <command.h>
#include <net.h>
#include <bios.h>
#include <model.h>
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "ethernet.h"

/*
 *	Ethernet test ;
*/

int do_eth_phy_power_saving_analog_off_1 (unsigned long enable)
{
	int ret = 0;	

	if(enable)
		ret = run_command("mii write 10 11 0DC0", 0);
	else
		ret = run_command("mii write 10 11 05C0", 0);

        return ret;
}

int do_eth_phy_power_saving_power_down_1 (unsigned long enable)
{
	int ret = 0;

	if(enable)
		ret = run_command("mii write 10 0 3900", 0);
	else
		ret = run_command("mii write 10 0 3100", 0);
        
	return ret;
}


int do_eth_phy_power_saving_analog_off_2 (unsigned long enable)
{
	int ret = 0;

	if(enable)
		ret = run_command("mii write 2 11 0DC0", 0);
	else
		ret = run_command("mii write 2 11 05C0", 0);

        return ret;
}

int do_eth_phy_power_saving_power_down_2 (unsigned long enable)
{
	int ret = 0;

	if(enable)
		ret = run_command("mii write 2 0 3900", 0);
	else  
		ret = run_command("mii write 2 0 3100", 0);

	return ret;
}

int do_eth1_100_cmd_test(void)
{
	int ret = 0;

	ret = run_command("mii write 4 4 100", 0);

	if (ret)
		return -1;

	ret = run_command("mii write 4 0 1200", 0);
	
	if (ret)
		return -1;

	udelay(1000*1000);

	ret = run_command("ethloop", 0);
	
	if (ret)
		return -1;

	return ret;
}

int do_eth2_100_cmd_test(void)
{
	int ret = 0;

	ret = run_command("setenv -f ethact cpsw1", 0);
	
	if (ret)
		return -1;

	ret = run_command("mii write 5 4 100", 0);
	
	if (ret)
		return -1;
	
	ret = run_command("mii write 5 0 1200", 0);
	
	if (ret)
		return -1;
	
	udelay(1000*1000);
	
	ret = run_command("ethloop", 0);
	
	if (ret)
		return -1;
	
	ret = run_command("setenv -f ethact cpsw0", 0);
	
	if (ret)
		return -1;
	
	return ret;
}

int do_eth1_10_cmd_test(void)
{
	int ret = 0;
	

	ret = run_command("setenv -f ethact cpsw0", 0);


	ret = run_command("mii write 4 4 40", 0);
	
	if (ret)
		return -1;
	
	ret = run_command("mii write 4 0 1200", 0);
	
	if (ret)
		return -1;
	
	udelay(1000*1000);

	ret = run_command("ethloop", 0);
	
	if (ret)
		return -1;
	
	return ret;
}

int do_eth2_10_cmd_test(void)
{
	int ret = 0;

	ret = run_command("setenv -f ethact cpsw1", 0);
	
	if (ret)
		return -1;
	
	ret = run_command("mii write 5 4 40", 0);
	
	if (ret)
		return -1;
	
	ret = run_command("mii write 5 0 1200", 0);

	if (ret)
		return -1;
	
	udelay(1000*1000);

	ret = run_command("ethloop", 0);

	if (ret)
		return -1;
	
	ret = run_command("setenv -f ethact cpsw0", 0);

	if (ret)
		return -1;
	
	return ret;
}

int do_eth_100_burnin_test(void)
{
	int ret = 0;

	ret = run_command("mii write 4 4 100", 0);

	if (ret)
		return -1;
	
	ret = run_command("mii write 4 0 1200", 0);

	if (ret)
		return -1;
	
	ret = run_command("mii write 5 4 100", 0);

	if (ret)
		return -1;
	
	ret = run_command("mii write 5 0 1200", 0);

	if (ret)
		return -1;
	
	udelay(1000*1000);

	while(1) {
		if (check_if_press_character(ESC)) {
			return -1;
		}

		ret = run_command("setenv -f ethact cpsw0", 0);

		if (ret)
			return -1;
	
		ret = run_command("ethloop", 0);

		if (ret)
			return -1;
	
		ret = run_command("setenv -f ethact cpsw1", 0);

		if (ret)
			return -1;
	
		ret = run_command("ethloop", 0);

		if (ret)
			return -1;
	
	}

	return ret;
}

void LAN_100M_CROSSCABLE_MPTEST(int led, int i)
{
	int flag;
        int retry = 2;
	int time;
	printf("[0x%04x]MAC%d 100MHZ External testing ... \n", led, i+1);

        
	if (i == 0) {
		run_command("mii write 4 4 100", 0);
		run_command("mii write 4 0 1200", 0);
		udelay(1000*1000);
		for (time = 0; time < retry; time++){
         		flag = net_loop(ETHLOOP);

                        if(time)
                                break;

         		if(flag < 0){
         		        printf("test failed!.");
                                printf("Please check ethernet test cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
         		}
         	}
	} else {
		run_command("setenv -f ethact cpsw1", 0);
		run_command("mii write 5 4 100", 0);
		run_command("mii write 5 0 1200", 0);
		udelay(1000*1000);
		for (time = 0; time < retry; time++){
         		flag = net_loop(ETHLOOP);

                        if(time)
                                break;

         		if(flag < 0){
         		        printf("test failed!.");
                                printf("Please check ethernet test cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
         		}
         	}
		run_command("setenv -f ethact cpsw0", 0);
	}

	if (flag < 0) {
		printf("MAC%d 100MHz External testing ** FAIL ** \r\n", i+1);
		halt(led, HALT_MODE1);
	} else {
	        if(i == 1){
                        printf("Check LED is solid Green or not. If yes, press SW button to continue.\n");
                        Wait_SW_Button();
                }
		printf("MAC%d 100MHZ External testing ** OK ** \r\n\n", i+1);
	}
}

void LAN_10M_CROSSCABLE_MPTEST(int led, int i)
{
	int flag;
	int retry = 2;
	__maybe_unused int time;

	printf("[0x%04x]MAC%d 10MHZ External testing ... \n", led, i + 1);

	if (i == 0) {
		run_command("mii write 4 4 40", 0);
		run_command("mii write 4 0 1200", 0);
		udelay(1000*1000);
		for (time = 0; time < retry; time++){
         		flag = net_loop(ETHLOOP);

                        if(time)
                                break;

         		if(flag < 0){
         		        printf("test failed!.");
                                printf("Please check ethernet test cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
         		}

         	}
	} else {
		for (time = 0; time < retry; time++){
		run_command("setenv -f ethact cpsw1", 0);
		run_command("mii write 5 4 40", 0);
		run_command("mii write 5 0 1200", 0);
		udelay(1000*1000);
         		flag = net_loop(ETHLOOP);

                        if(time)
                                break;

         		if(flag < 0){
         		        printf("test failed!.");
                                printf("Please check ethernet test cable.\n ");
                                printf("If you're ready. Please Press SW button to retry.\n ");
                                Wait_SW_Button();
                                continue;
         		}

         	}
		run_command("setenv -f ethact cpsw0", 0);
	}
	
	if (flag < 0) {
		printf("MAC%d 10MHz External testing ** FAIL ** \r\n", i+1);
		halt(led, HALT_MODE1);
	} else {
	        if(i == 1){
                        printf("Check LED is solid Orange or not. If yes, press SW button to continue.\n");
                        Wait_SW_Button();
                }
		printf("MAC%d 10MHZ External testing ** OK ** \r\n\n", i+1);
	}

}
