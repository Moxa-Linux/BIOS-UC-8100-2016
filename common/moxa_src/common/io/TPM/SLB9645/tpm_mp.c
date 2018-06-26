/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    tpm.c

    This file includes functions of SLB9645 TPM chip.
    
    2014-01-09	Sun Lee
*/

#include <common.h>
#include <command.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <bios.h>
#include <model.h>
#include "../cmd_bios.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "tpm_mp.h"

/*
 *	TPM test ;
*/
int do_tpm_tsc_physical_presence_present_func(void)
{
	int ret = 0;

	ret = run_command("tpm tsc_physical_presence 0x0008", 0);

	return ret;
}

int do_tpm_tsc_physical_presence_cmd_enable_func(void)
{
	int ret = 0;

	ret = run_command("tpm tsc_physical_presence 0x0020", 0);

	return ret;
}

int do_tpm_physical_set_deactivated_func(void)
{
	int ret = 0;
	
	ret = run_command("tpm physical_set_deactivated 1", 0);
	
	return ret;
}

int do_tpm_physical_clear_deactivated_func(void)
{
        int ret = 0;

        ret = run_command("tpm physical_set_deactivated 0", 0);

	return ret;
}

int do_tpm_Physical_enabled_func(void)
{
	int ret = 0;

	ret = run_command("tpm physical_enable", 0);

	return ret;
}

int do_tpm_Physical_disabled_func(void)
{
	int ret=0;

	ret = run_command("tpm physical_disable", 0);	
	
	return ret;
}

int do_tpm_init_func(void)
{
	int ret = 0;

	ret = run_command("tpm init", 0);

	return ret;
}

int do_tpm_force_clear_func(void)
{
	int ret = 0;

	gpio_request (PIO_TPM_FORCE_CLEAR, "TPM_Force_Clear");
	gpio_direction_output (PIO_TPM_FORCE_CLEAR, 0);

	ret = run_command("tpm force_clear", 0);
        
	gpio_direction_output (PIO_TPM_FORCE_CLEAR, 1);
        
	return ret;
}

int do_tpm_startup_func(void)
{
	int ret = 0;

	ret = run_command("tpm startup TPM_ST_CLEAR", 0);

	return ret;
}

int do_tpm_get_capability_func(int *enable, int *active)
{
	int ret = 0;

	ret = run_command("tpm get_capability 0x04 0x108 0x81000000 0x16", 0);

	if (ret != 0)
		return ret;

	printf("TPM Get Capability info to memory Addr(0x81000000) OK!\n");

	*enable = VPchar(0x81000002);

	if (enable)
		printf("TPM is disabled ");
	else
		printf("TPM is enabled ");

	*active = VPchar(0x81000004);

	if (active)
		printf("and deactivated\n");
	else
		printf("and activated\n");

	return ret;

}

int do_tpm_disabled_func(int *enable, int *active)
{
	int ret = 0;

	if ((ret = run_command("tpm init", 0)) != 0)
		return F_TPM_INIT;

	printf("Currenn TPM status check..");

        udelay(1000*1000);

	if ((ret = run_command("tpm startup TPM_ST_CLEAR", 0)) != 0)
		return F_TPM_ST_CLEAR;

	printf("..");

	udelay(1000*1000);

	if ((ret = run_command("tpm tsc_physical_presence 0x0020", 0)) != 0)
		return F_TPM_PRESENCE_CMD;

	printf("..");

	udelay(1000*1000);

	if ((ret = run_command("tpm get_capability 0x04 0x108 0x81000000 0x16", 0)) != 0)
		return F_TPM_CAPABILITY;
	
	printf("..");

	udelay(1000*1000);

	*enable = VPchar(0x81000002);

	*active = VPchar(0x81000004);

	if((*enable == 0) || (*active == 0)){
		if ((ret = run_command("tpm tsc_physical_presence 0x0008", 0)) != 0)
			return F_TPM_PRESENCE_PRESENT;

		udelay(1000*1000);

		if ((ret = run_command("tpm physical_set_deactivated 1", 0)) != 0)
			return F_TPM_SET_DEACTIVATED;

		udelay(1000*1000);

		if ((ret = run_command("tpm physical_disable", 0)) != 0)
			return F_TPM_PHTSICAL_DISABLE;

		udelay(1000*1000);
	
		if ((ret = run_command("tpm get_capability 0x04 0x108 0x81000000 0x16", 0)) != 0)
			return F_TPM_GET_CAPABILITY;
			
		printf("\n***Current TPM status change to ");

		udelay(1000*1000);

		*enable = VPchar(0x81000002);
		*active = VPchar(0x81000004);
	}
	
	return ret;
}

void TPM_MPTEST(u8 led)
{

   	printf("[0x%04x]SLB9645 TPM testing ... ",led);
	printf("** FAIL **\r\n");
	printf("** OK ** \r\n\n");
}

int do_tpm_enabled_func(int *enable, int *active)
{
	int ret = 0;
	
	if ((ret = run_command("tpm init", 0)) != 0)
		return F_TPM_INIT;

	printf("TPM status check..");

	udelay(1000*1000);

	if ((ret = run_command("tpm startup TPM_ST_CLEAR", 0)) != 0)
		return F_TPM_ST_CLEAR;

	printf("..");

	udelay(1000*1000);
	
	if ((ret = run_command("tpm tsc_physical_presence 0x0020", 0)) != 0)
		return F_TPM_PRESENCE_CMD;

	printf("..");
        
	udelay(1000*1000);

        if ((ret = run_command("tpm get_capability 0x04 0x108 0x81000000 0x16", 0)) != 0)
		return F_TPM_GET_CAPABILITY;

	printf("..");

	udelay(1000*1000);

	*enable = VPchar(0x81000002);

	*active = VPchar(0x81000004);

	if((enable != 0) || (active != 0)){
		if ((ret = run_command("tpm tsc_physical_presence 0x0008", 0)) != 0)
			return F_TPM_PRESENCE_PRESENT;

		udelay(1000*1000);

		if ((ret = run_command("tpm physical_enable", 0)) != 0)
			return F_TPM_PHTSICAL_ENABLE;

		udelay(1000*1000);

		if ((ret = run_command("tpm physical_set_deactivated 0", 0)) != 0)
			return F_TPM_SET_DEACTIVATED;

		udelay(1000*1000);

		if ((ret = run_command("tpm get_capability 0x04 0x108 0x81000000 0x16", 0)) != 0)
			return F_TPM_GET_CAPABILITY;

		udelay(1000*1000);

		*enable = VPchar(0x81000002);
		*active = VPchar(0x81000004);
        }

	return ret;
}

