/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    cmd_tpm2.h

    2017-06-27  HsienWen Tsai                              Created it.
*/
//----------------------------------------------------------------------------
//Rev   Date         Name  Description
//---------------------------------------------------------------------------
//R00   06/27/2017   SHA   Added tpm2_probe for mp test.


#ifndef _CMD_TPM2_H_
#define _CMD_TPM2_H_

#define TPM_SPI_BUS			1
#define TPM_SPI_CS			0
#define TPM_SPI_FREQ			500000
#define TPM_SPI_MODE			0
#define TPM_HEADER_SIZE			10


#define TPM_ACCESS(l)			(0x0000 | ((l) << 12))
#define TPM_INT_ENABLE(l)		(0x0008 | ((l) << 12))
#define TPM_INT_VECTOR(l)		(0x000C | ((l) << 12))
#define TPM_INT_STATUS(l)		(0x0010 | ((l) << 12))
#define TPM_INTF_CAPS(l)		(0x0014 | ((l) << 12))
#define TPM_STS(l)			(0x0018 | ((l) << 12))
#define TPM_STS3(l)			(0x001b | ((l) << 12))
#define TPM_DATA_FIFO(l)		(0x0024 | ((l) << 12))

#define TPM_DID_VID(l)			(0x0F00 | l)
#define TPM_RID(l)			(0x0F04 | ((l) << 12))

#define MAX_SPI_FRAMESIZE		64

#define min_t(type, x, y) ({                    \
        type __min1 = (x);                      \
        type __min2 = (y);                      \
		__min1 < __min2 ? __min1: __min2; })

#define swap32(x) (((x >> 24)&0xff) | 		\
                    ((x << 8)&0xff0000) |	\
                    ((x >> 8)&0xff00) |		\
                    ((x << 24)&0xff000000))


#define prop_str(val) val ? "set" : "clear"

//====================== TPM_PT_PERSISTENT ====================
#define OWNERAUTHSET			0x1 
#define ENDORSEMENTAUTHSET		0x2
#define LOCKOUTAUTHSET			0x4
#define PERSISTENT_RESERVED1		0xf8
#define DISABLECLEAR			0x100
#define INLOCKOUT			0x200
#define TPMGENERATEDEPS			0x400
#define PERSISTENT_RESERVED2		0xfffff800

//====================== TPM_PT_STARTUP_CLEAR ====================

#define PHENABLE			0x1 
#define SHENABLE			0x2
#define EHENABLE			0x4
#define PHENABLENV			0x8
#define STARTUP_RESERVED1		0x7ffffff0
#define ORDERLY				0x80000000


//====================== SESSION ADDRESS ====================
#define TPM_ST_ADDR			0x0
#define RES_SIZE_ADDR			0x2
#define TPM_RC_ADDR			0x6
#define TPMI_YES_NO_ADDR		0xA
#define TPM_CAP				0xB
#define RES_GROUP_DATA_SIZE_ADDR	0xE
#define GROUP_ADDR			0x13							

//=============================================================
#define TPM_PT_NONE				(0x00000000)
#define PT_GROUP				(0x00000100)
#define PT_FIXED				(PT_GROUP * 1)
#define TPM_PT_FAMILY_INDICATOR			(PT_FIXED + 0)
#define TPM_PT_LEVEL				(PT_FIXED + 1)
#define TPM_PT_REVISION				(PT_FIXED + 2)
#define TPM_PT_DAY_OF_YEAR			(PT_FIXED + 3)
#define TPM_PT_YEAR				(PT_FIXED + 4)
#define TPM_PT_MANUFACTURER			(PT_FIXED + 5)
#define TPM_PT_VENDOR_STRING_1			(PT_FIXED + 6)
#define TPM_PT_VENDOR_STRING_2			(PT_FIXED + 7)
#define TPM_PT_VENDOR_STRING_3			(PT_FIXED + 8)
#define TPM_PT_VENDOR_STRING_4			(PT_FIXED + 9)
#define TPM_PT_VENDOR_TPM_TYPE			(PT_FIXED + 10)
#define TPM_PT_FIRMWARE_VERSION_1		(PT_FIXED + 11)
#define TPM_PT_FIRMWARE_VERSION_2		(PT_FIXED + 12)
#define TPM_PT_INPUT_BUFFER			(PT_FIXED + 13)
#define TPM_PT_HR_TRANSIENT_MIN			(PT_FIXED + 14)
#define TPM_PT_HR_PERSISTENT_MIN		(PT_FIXED + 15)
#define TPM_PT_HR_LOADED_MIN			(PT_FIXED + 16)
#define TPM_PT_ACTIVE_SESSIONS_MAX		(PT_FIXED + 17)
#define TPM_PT_PCR_COUNT			(PT_FIXED + 18)
#define TPM_PT_PCR_SELECT_MIN			(PT_FIXED + 19)
#define TPM_PT_CONTEXT_GAP_MAX			(PT_FIXED + 20)
#define TPM_PT_NV_COUNTERS_MAX			(PT_FIXED + 22)
#define TPM_PT_NV_INDEX_MAX			(PT_FIXED + 23)
#define TPM_PT_MEMORY				(PT_FIXED + 24)
#define TPM_PT_CLOCK_UPDATE			(PT_FIXED + 25)
#define TPM_PT_CONTEXT_HASH			(PT_FIXED + 26)
#define TPM_PT_CONTEXT_SYM			(PT_FIXED + 27)
#define TPM_PT_CONTEXT_SYM_SIZE			(PT_FIXED + 28)
#define TPM_PT_ORDERLY_COUNT			(PT_FIXED + 29)
#define TPM_PT_MAX_COMMAND_SIZE			(PT_FIXED + 30)
#define TPM_PT_MAX_RESPONSE_SIZE		(PT_FIXED + 31)
#define TPM_PT_MAX_DIGEST			(PT_FIXED + 32)
#define TPM_PT_MAX_OBJECT_CONTEXT		(PT_FIXED + 33)
#define TPM_PT_MAX_SESSION_CONTEXT		(PT_FIXED + 34)
#define TPM_PT_PS_FAMILY_INDICATOR		(PT_FIXED + 35)
#define TPM_PT_PS_LEVEL				(PT_FIXED + 36)
#define TPM_PT_PS_REVISION			(PT_FIXED + 37)
#define TPM_PT_PS_DAY_OF_YEAR			(PT_FIXED + 38)
#define TPM_PT_PS_YEAR				(PT_FIXED + 39)
#define TPM_PT_SPLIT_MAX			(PT_FIXED + 40)
#define TPM_PT_TOTAL_COMMANDS			(PT_FIXED + 41)
#define TPM_PT_LIBRARY_COMMANDS			(PT_FIXED + 42)
#define TPM_PT_VENDOR_COMMANDS			(PT_FIXED + 43)
#define TPM_PT_NV_BUFFER_MAX			(PT_FIXED + 44)
#define TPM_PT_MODES				(PT_FIXED + 45)
#define PT_VAR					(PT_GROUP * 2)
#define TPM_PT_PERMANENT			(PT_VAR + 0)
#define TPM_PT_STARTUP_CLEAR			(PT_VAR + 1)
#define TPM_PT_HR_NV_INDEX			(PT_VAR + 2)
#define TPM_PT_HR_LOADED			(PT_VAR + 3)
#define TPM_PT_HR_LOADED_AVAIL			(PT_VAR + 4)
#define TPM_PT_HR_ACTIVE			(PT_VAR + 5)
#define TPM_PT_HR_ACTIVE_AVAIL			(PT_VAR + 6)
#define TPM_PT_HR_TRANSIENT_AVAIL		(PT_VAR + 7)
#define TPM_PT_HR_PERSISTENT			(PT_VAR + 8)
#define TPM_PT_HR_PERSISTENT_AVAIL		(PT_VAR + 9)
#define TPM_PT_NV_COUNTERS			(PT_VAR + 10)
#define TPM_PT_NV_COUNTERS_AVAIL		(PT_VAR + 11)
#define TPM_PT_ALGORITHM_SET			(PT_VAR + 12)
#define TPM_PT_LOADED_CURVES			(PT_VAR + 13)
#define TPM_PT_LOCKOUT_COUNTER			(PT_VAR + 14)
#define TPM_PT_MAX_AUTH_FAIL			(PT_VAR + 15)
#define TPM_PT_LOCKOUT_INTERVAL			(PT_VAR + 16)
#define TPM_PT_LOCKOUT_RECOVERY			(PT_VAR + 17)
#define TPM_PT_NV_WRITE_RECOVERY		(PT_VAR + 18)
#define TPM_PT_AUDIT_COUNTER_0			(PT_VAR + 19)
#define TPM_PT_AUDIT_COUNTER_1			(PT_VAR + 20)

#define TPM_BUFSIZE				4096
#define EXIT_SUCCESS				0 
#define NULL_POINTER_CHECK(x) if (NULL == x) { ret_val = EINVAL; printf("Error: Invalid argument.\n"); break; }    ///< Argument NULL check.
#define RET_VAL_CHECK(x) if (EXIT_SUCCESS != x) { break; } 

/*
 * Values from last command.
 */

enum tis_access {
        TPM_ACCESS_VALID = 0x80,
        TPM_ACCESS_ACTIVE_LOCALITY = 0x20,
        TPM_ACCESS_REQUEST_PENDING = 0x04,
        TPM_ACCESS_REQUEST_USE = 0x02,
};

enum tis_status {
        TPM_STS_VALID = 0x80,
        TPM_STS_COMMAND_READY = 0x40,
        TPM_STS_GO = 0x20,
        TPM_STS_DATA_AVAIL = 0x10,
        TPM_STS_DATA_EXPECT = 0x08,
};

enum tis_int_flags {
        TPM_GLOBAL_INT_ENABLE = 0x80000000,
        TPM_INTF_BURST_COUNT_STATIC = 0x100,
        TPM_INTF_CMD_READY_INT = 0x080,
        TPM_INTF_INT_EDGE_FALLING = 0x040,
        TPM_INTF_INT_EDGE_RISING = 0x020,
        TPM_INTF_INT_LEVEL_LOW = 0x010,
        TPM_INTF_INT_LEVEL_HIGH = 0x008,
        TPM_INTF_LOCALITY_CHANGE_INT = 0x004,
        TPM_INTF_STS_VALID_INT = 0x002,
        TPM_INTF_DATA_AVAIL_INT = 0x001,
};

enum tis_defaults {
        TIS_MEM_BASE = 0xFED40000,
        TIS_MEM_LEN = 0x5000,
        TIS_SHORT_TIMEOUT = 750,        /* ms */
        TIS_LONG_TIMEOUT = 2000,        /* 2 sec */
};

struct tpm_output_header {
        u16  tag;
        u32 length;
        u32 return_code;
};


enum tpm2_const {
        TPM2_MINOR = 224,        /* officially assigned */
        TPM2_BUFSIZE = 4096,
        TPM2_NUM_DEVICES = 256,
        TPM2_RETRY = 50,         /* 5 seconds */
};


enum tpm2_hierarchy {
        TPM2_OWNER = 0x1,        /* officially assigned */
        TPM2_ENDORSEMENT = 0xb,
        TPM2_PLATFORM = 0xc,

};

enum tpm_timeout {
        TPM_TIMEOUT = 5,        /* msecs */
        TPM_TIMEOUT_RETRY = 100 /* msecs */
};

/* TPM addresses */
enum tpm_addr {
        TPM2_SUPERIO_ADDR = 0x2E,
        TPM2_ADDR = 0x4E,
};

/* Indexes the duration array */
enum tpm2_duration {
        TPM2_SHORT = 0,
        TPM2_MEDIUM = 1,
        TPM2_LONG = 2,
        TPM2_UNDEFINED,
};

enum tpm2_startup_type {
        TPM2_SU_CLEAR            = 0x0001,
        TPM2_SU_STATE            = 0x0002,

};

enum tpm2_capability_properties {
	    TPM2_PROPERTIES_FIXED            = 0x0,
        TPM2_PROPERTIES_VARIABLE		 = 0x1,

};

//=======================================================================
int tpm2_init(void);
int tpm2_startup(int mode);
int tpm2_get_capability(int mode, char *res);
int tpm2_init(void);
int tpm2_probe(void); //R00
int tpm2_force_clear(void);
int tpm2_startup(int mode);
int tpm2_hierarchy(int mode, int enable);
u32 buf_to_u32 (char *buf);
#endif
