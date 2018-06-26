/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    tpm.c

    This file includes functions of SLB9645 TPM chip.
    
    2014-01-09	Sun Lee
*/

#ifndef _TPM_MP_H
#define _TPM_MP_H

enum TPM_FAIL_STATE{
	F_TPM_INIT = -1,
	F_TPM_ST_CLEAR = -2,
	F_TPM_PRESENCE_CMD = -3,
	F_TPM_CAPABILITY = -4,
	F_TPM_PRESENCE_PRESENT = -5,
	F_TPM_PHTSICAL_ENABLE = -6,
	F_TPM_PHTSICAL_DISABLE = -7,
	F_TPM_SET_ACTIVATED = -8,
	F_TPM_SET_DEACTIVATED = -9,
	F_TPM_GET_CAPABILITY = -10
};

int do_tpm_tsc_physical_presence_present_func(void);
int do_tpm_tsc_physical_presence_cmd_enable_func(void);
int do_tpm_physical_set_deactivated_func(void);
int do_tpm_physical_clear_deactivated_func(void);
int do_tpm_Physical_enabled_func(void);
int do_tpm_Physical_disabled_func(void);
int do_tpm_init_func(void);
int do_tpm_force_clear_func(void);
int do_tpm_startup_func(void);
int do_tpm_get_capability_func(int *enable, int *active);
int do_tpm_disabled_func(int *enable, int *active);
int do_tpm_enabled_func(int *enable, int *active);
void TPM_MPTEST(u8 led);
#endif

