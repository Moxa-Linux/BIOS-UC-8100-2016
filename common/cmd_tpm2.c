/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    cmd_tpm2.c

    TPM2 Utilities.

    2017-06-22  HsienWen                              Created it.
*/

//----------------------------------------------------------------------------
//Rev   Date         Name  Description
//----------------------------------------------------------------------------
//R03   06/27/2017   SHA   Added tpm2_probe for mp test.
//R02   06/23/2017   SHA   Hide the message in console.
//R01   06/23/2017   SHA   Added TPM manufacturer ID detection.
//R00   06/22/2017   SHA   First Initial.

#include <common.h>
#include <command.h>
#include <errno.h>
#include <spi.h>
#include "cmd_tpm2.h"
#include <asm/gpio.h>
#include <model.h>

static int tpm2_init_ok = 0;
static int tpm2_startup_ok = 0;
struct spi_slave *slave;
int vendor_locality = 0;


static const char tpm2_getcapability_fixed[] ={
    // TPM2_GetCapability (TPM_CAP_TPM_PROPERTIES, -- )
    0x80, 0x01,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x16,    // commandSize
    0x00, 0x00, 0x01, 0x7A,    // TPM_CC_GetCapability
    0x00, 0x00, 0x00, 0x06,    // TPM_CAP_TPM_PROPERTIES (Property Type: TPM_PT)
    0x00, 0x00, 0x01, 0x00,    // Property: TPM_PT_FAMILY_INDICATOR: PT_GROUP * 1 + 0, Default shift 0x100
    0x00, 0x00, 0x00, 0x2D     // PropertyCount 2D (Group Max = (PT_FIXED + 44) = 45 = 0x2d)
};

static const char tpm2_getcapability_variable[] ={
    // TPM2_GetCapability (TPM_CAP_TPM_PROPERTIES, -- )
    0x80, 0x01,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x16,    // commandSize
    0x00, 0x00, 0x01, 0x7A,    // TPM_CC_GetCapability
    0x00, 0x00, 0x00, 0x06,    // TPM_CAP_TPM_PROPERTIES (Property Type: TPM_PT)
    0x00, 0x00, 0x02, 0x00,    // Property: TPM_PT_PERMANENT: PT_GROUP * 2 + 0, 
    0x00, 0x00, 0x00, 0x14     // PropertyCount 14 (Group = (PT_FIXED + 19) = 14)
};

static const char tpm2_hierarchyControl[] ={
    // TPM2_GetCapability (TPM_CAP_TPM_PROPERTIES, -- )
    0x80, 0x02,                // TPM_ST_SESSIONS
    0x00, 0x00, 0x00, 0x20,    // commandSize
    0x00, 0x00, 0x01, 0x21,    // TPM_CC_HierarchyControl
    0x40, 0x00, 0x00, 0x0c,    
    0x00, 0x00, 0x00, 0x09,
    0x40, 0x00, 0x00, 0x09,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x40, 0x00, 0x00,
};

static const char tpm2_getcapability_first[] ={
    // TPM2_GetCapability (TPM_CAP_TPM_PROPERTIES, -- )
    0x80, 0x01,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x16,    // commandSize
    0x00, 0x00, 0x01, 0x7A,    // TPM_CC_GetCapability
    0x00, 0x00, 0x00, 0x06,    // TPM_CAP_TPM_PROPERTIES (Property Type: TPM_PT)
    0x00, 0x00, 0x01, 0x00,    // Property: TPM_PT_FAMILY_INDICATOR: PT_GROUP * 1 + 0
    0x00, 0x00, 0x00, 0x1     // PropertyCount 2D (from 100 - 201)
};


static const char tpm2_clear_control[] ={
    0x80, 0x02,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x1c,    // commandSize
    0x00, 0x00, 0x01, 0x27,
    0x40, 0x00, 0x00, 0x0C,    // TPM_CAP_TPM_PROPERTIES (Property Type: TPM_PT)
    0x00, 0x00, 0x00, 0x09,
    0x40, 0x00, 0x00, 0x09,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,    
};


static const char tpm2_clear[] ={
    // TPM2_GetCapability (TPM_CAP_TPM_PROPERTIES, -- )
    0x80, 0x02,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x1B,    // commandSize
    0x00, 0x00, 0x01, 0x26,    // TPM_CC_Clear
    0x40, 0x00, 0x00, 0x0c,    // TPM_CAP_TPM_PROPERTIES (Property Type: TPM_PT)
    0x00, 0x00, 0x00, 0x09,
    0x40, 0x00, 0x00, 0x09,
    0x00, 0x00, 0x00, 0x00,
    0x0,
};



static const char tpm2_self_test_full[] ={
    // TPM2_SelfTest(fullTest=YES)
    0x80, 0x01,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x0B,    // commandSize
    0x00, 0x00, 0x01, 0x43,    // TPM_CC_SelfTest
    0x01                       // fullTest=Yes
};


static const char tpm2_startup_clear[] ={
    // TPM2_Startup(SU_CLEAR)
    0x80, 0x01,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x0C,    // commandSize
    0x00, 0x00, 0x01, 0x44,    // TPM_CC_Startup
    0x00, 0x00                 // TPM_ST_CLEAR
};

static const char tpm2_startup_state[] ={
    // TPM2_Startup(SU_STATE)
    0x80, 0x01,                // TPM_ST_NO_SESSIONS
    0x00, 0x00, 0x00, 0x0C,    // commandSize
    0x00, 0x00, 0x01, 0x44,    // TPM_CC_Startup
    0x00, 0x01                 // TPM_ST_STATE
};


static int convert_return_code(uint32_t return_code)
{
        if (return_code)
                return CMD_RET_FAILURE;
        else
                return CMD_RET_SUCCESS;
}

int spi_tpm_init(void)
{
	int ret = 0;
	
	slave = spi_setup_slave(TPM_SPI_BUS, TPM_SPI_CS, TPM_SPI_FREQ, TPM_SPI_MODE);
	
	if (!slave) {
		printf("Invalid device %d:%d\n", TPM_SPI_BUS, TPM_SPI_CS);
		return -EINVAL;
	}
	
//R95	printf("setup: TPM_SPI_BUS %d, TPM_SPI_CS %d, speed: %d, TPM_SPI_MODE:%d\n",
//R95				TPM_SPI_BUS, TPM_SPI_CS, TPM_SPI_FREQ, TPM_SPI_MODE);

	ret = spi_claim_bus(slave);

	if (ret)
		printf("spi_claim_bus fail\n");

	return 0;
}

static void read_spi_bytes(u32 addr, u8 len, u8 size, u8 *result){
 
	char tx_buf [512] = {0};
	char rx_buf [512] = {0};
	int t_len = 0;
//	int i = 0;
	
	if (len > MAX_SPI_FRAMESIZE){
		printf("too large1\n");
		return; // ADD error
	}
	
//	printf( "read_spi_bytes_1\n");
	
	memset(tx_buf, 0, 4 + len);
	memset(rx_buf, 0, 4 + len);
	
	t_len = 4 + len;
	tx_buf[0] = 0x80 | (len-1); //0x80 = read  | 0=1byte
	tx_buf[1] = (addr>>16) & 0xFF;
	tx_buf[2] = (addr>>8)  & 0xFF;
	tx_buf[3] = (addr)     & 0xFF;
	
	spi_xfer(slave, t_len * 8, &tx_buf, &rx_buf, SPI_XFER_BEGIN);
	
	memcpy(result, &rx_buf[4], len);
	
//	printf("read_spi_bytes_2\n");
	
//	for (i = 0; i < len; i++)
//		printf("result[%d]: %x\n", i, result[i]);
	
	memset(rx_buf, 0, 4 + len);
}

static void write_spi_bytes(u32 addr, u8 len, u8 size, u8 *value){
	
	char tx_buf [512] = {0};
	char rx_buf [512] = {0};
	int t_len = 0;
//	int  i = 0;
	
	if (len > MAX_SPI_FRAMESIZE) {
		printf("too large1\n");
		return; // ADD error
	}
	
//	printf("TPM SPI write  addr: %d, len: %d, size: %d\n", addr, len, size);
	
//	for(i = 0; i < len; i++)
//		printf ("val[%d]:%x, ", i, value[i]);
	
//	printf("\n");
	
	memset(tx_buf, 0, 4 + len);
	memset(rx_buf, 0, 4 + len);
	
	
	t_len = 4 + len ;
	
	tx_buf[0] = 0x00 | (len-1); //0x00 = write | 0 = 1byte
	tx_buf[1] = (addr>>16) & 0xFF;
	tx_buf[2] = (addr>>8)  & 0xFF;
	tx_buf[3] = (addr)     & 0xFF;
	
	memcpy(&tx_buf[4], value, len);
	
	spi_xfer(slave, t_len * 8, &tx_buf, &rx_buf, SPI_XFER_BEGIN);
	
	memset(rx_buf, 0, 4 + len);
}


/* Helpers - read */
static inline void read_tpm_bytes(u32 addr, u8 len, u8 *res)
{
	read_spi_bytes(addr, len, 1, res);
}
static inline void read_tpm_byte(u32 addr, u8 *res)
{
	read_spi_bytes(addr, 1, 1, res);
}

static inline void read_tpm_word(u32 addr, u16 *res)
{
	read_spi_bytes(addr, 1, 2, (u8 *)res);
}

static inline void read_tpm_dword(u32 addr, u32 *res)
{
	read_spi_bytes(addr, 1, 4, (u8 *)res);
}

/* Helpers - write */
static inline void write_tpm_bytes(u32 addr, u8 len, u8 *value)
{
	write_spi_bytes(addr, len, 1, value);
}

static inline void write_tpm_byte(u32 addr, u8 value)
{
	u8 tmp = value;

	write_spi_bytes(addr, 1, 1, &tmp);
}

static inline void write_tpm_dword(u32 addr, u32 value)
{
	u32 tmp = value;

	write_spi_bytes(addr, 1, 4, (u8 *)&tmp);

}

static int wait_startup(void)
{
	u8 access;
	int cnt = 50;
	
	do {
		read_tpm_byte(TPM_ACCESS(0), &access);
	
		if (access & TPM_ACCESS_VALID)
				return 0;
				
		udelay(1000 * 50);
			
	} while (cnt--);
	
	return -1;
}

static int check_locality(int l)
{
	u8 access;
	
	read_tpm_byte(TPM_ACCESS(l), &access);
	
//	printf("%s(%d) val:%x\n", __func__, __LINE__, access);

	if ((access & (TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID)) ==
		(TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID))
			return vendor_locality = l;

	return -1;
}

static void release_locality(int l, int force)
{
	u8 access;
	
	read_tpm_byte(TPM_ACCESS(l), &access);
	
//	printf("release_locality: %x\n", access);
	
	if (force ||
		(access & (TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID)) ==
			(TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID)){

		write_tpm_byte(TPM_ACCESS(l), TPM_ACCESS_ACTIVE_LOCALITY);
	}
	check_locality(0);
}

static int request_locality(int l)
{
	int cnt = 50;
	
//	printf("request_locality_1\n");
	
	if (check_locality(l) >= 0)
		return l;
	
//	printf("request_locality_2: %x\n", l);
		
	write_tpm_byte(TPM_ACCESS(l), TPM_ACCESS_REQUEST_USE);
	
//	printf("request_locality_3\n");
	
	/* wait for burstcount */
//	printf("request_locality_4\n");
	do {
		if (check_locality(l) >= 0)
			return l;
		
		udelay(1000 * 50);
			
	} while (cnt--);
	
//	printf("request_locality_5\n");
	return -1;
		
}


static int get_burstcount(void)
{

	int burstcnt;
	u8 tmp;
	int cnt = 50;
	/* wait for burstcount */
	/* which timeout value, spec has 2 answers (c & d) */

	do {
		read_tpm_byte(TPM_STS(vendor_locality) + 1, &tmp);
		burstcnt = tmp;
//		printf("burstcnt_1:%x\n", burstcnt);
		read_tpm_byte(TPM_STS(vendor_locality) + 2, &tmp);
		burstcnt += tmp << 8;
//		printf("burstcnt2:%x\n", burstcnt);
		//read_tpm_word(chip, TPM_STS(chip->vendor.locality) + 1, &burstcount2);//TODO revisit
		if (burstcnt)
			return min_t(int, burstcnt, 64); //SPI framesize TODO revisit
		
		udelay(1000 * 50);
		
	} while (cnt--);
		
        return -1;
}

u8 tpm_tis_status(void)
{
	u8 status;
	
	read_tpm_byte(TPM_STS(vendor_locality), &status);
	
	return status;
}

void tpm_tis_ready(void)
{
	/* this causes the current command to be aborted */
	write_tpm_byte(TPM_STS(vendor_locality), TPM_STS_COMMAND_READY);
}

int wait_for_tpm_stat(u8 mask)
{
	u8 status;
	int cnt = 50;
	
	/* check current status */
	status = tpm_tis_status();
	
	if ((status & mask) == mask)
		return 0;

	do {
		
		udelay(100*1000);
		status = tpm_tis_status();
		
		if ((status & mask) == mask)
			return 0;
		
		udelay(1000 * 50);
			
	} while (cnt--);
	
	return -1;
}

static int tpm_tis_send_data(u8 *buf, size_t len, int itpm)
{
	int rc, status;
	size_t count = 0, burstcnt, transfer_size;
	
//	printf("tpm_tis_send_data_1\n");
	
	if (request_locality(0) < 0)
		return -EBUSY;
	
//	printf("tpm_tis_send_data_2\n");
	status = tpm_tis_status();
//	printf("tpm_tis_send_data_3\n");
	
	if ((status & TPM_STS_COMMAND_READY) == 0) {
//		printf("tpm_tis_send_data_3.1\n");
		tpm_tis_ready();
		
		if (wait_for_tpm_stat(TPM_STS_COMMAND_READY) < 0) {
			rc = -ETIME;
			goto out_err;
		}
	}
	
//	printf("tpm_tis_send_data_4\n");
	
	while (count < len - 1) {
		
		burstcnt = get_burstcount();
		
//		printf("tpm_tis_send_data_4.1: 0x%x\n", burstcnt);
		
		transfer_size = min_t (size_t, len - count - 1, burstcnt);
		
//		printf("tpm_tis_send_data_4.2: 0x%x\n", transfer_size);
		
		write_tpm_bytes(TPM_DATA_FIFO(vendor_locality), transfer_size, &buf[count]);
		
//		printf("tpm_tis_send_data_4.3: 0x%x\n", count);
		
		count +=  transfer_size;
		
		wait_for_tpm_stat(TPM_STS_VALID);
//		printf("tpm_tis_send_data_4.4 \n");
		status = tpm_tis_status();
//		printf("tpm_tis_send_data_4.5: 0x%x \n", status);
		if (!itpm && (status & TPM_STS_DATA_EXPECT) == 0) {
			rc = -EIO;
			goto out_err;
		}
	}
	
//	printf("tpm_tis_send_data_5\n");
	/* write last byte */
	write_tpm_byte(TPM_DATA_FIFO(vendor_locality), buf[count]);
	
	wait_for_tpm_stat(TPM_STS_VALID);
	
	status = tpm_tis_status();
	
//	printf("tpm_tis_send_data_6\n");
	
	if ((status & TPM_STS_DATA_EXPECT) != 0) {
			rc = -EIO;
			goto out_err;
	}
	
//	printf("tpm_tis_send_data_7\n");
	return 0;

out_err:
//	printf("tpm_tis_send_data_8\n");
	tpm_tis_ready();
	release_locality(vendor_locality, 0);
	return rc;
}

static int tpm_tis_send_main(u8 *buf, size_t len, int itpm)
{
	int rc;

//	printf("tpm_tis_send_main_1\n");

	rc = tpm_tis_send_data(buf, len, itpm);

	if (rc < 0)
		return rc;

//	printf("tpm_tis_send_main_2\n");
	/* go and do it */
	write_tpm_byte(TPM_STS(vendor_locality), TPM_STS_GO);
	
//	printf("tpm_tis_send_main_3\n");

//	printf("tpm_tis_send_main_4\n");
	
	return len;
		
}

int buf_to_uint64(char *input_buffer, int offset, char length, unsigned long long *output_value)
{
        int ret_val = EXIT_SUCCESS; // Return value.
        uint32_t i = 0;             // Loop variable.
        unsigned long long tmp = 0; // Temporary variable for value calculation.

        do
        {
                NULL_POINTER_CHECK(input_buffer);
                NULL_POINTER_CHECK(output_value);

                if (8 >= length)
                {
                        for (i = 0; i < length; i++)
                        {
                                tmp = (tmp << 8) + input_buffer[offset + i];
                        }
                        *output_value = tmp;
                }
                else
                {
                        ret_val = EINVAL;
                        fprintf(stderr, "Bad parameter. Requested conversion amount of %i is to high. The maximum possible amount is 8 bytes.\n", length);
                }
        } while (0);

        return ret_val;
}

void dump_permanent_attrs (u32 attrs)
{
    printf ("TPM_PT_PERSISTENT:\n");
    printf ("  ownerAuthSet:              %s\n", prop_str (attrs & OWNERAUTHSET));
    printf ("  endorsementAuthSet:        %s\n", prop_str (attrs & ENDORSEMENTAUTHSET));
    printf ("  lockoutAuthSet:            %s\n", prop_str (attrs & LOCKOUTAUTHSET));
    printf ("  reserved1:                 %s\n", prop_str (attrs & PERSISTENT_RESERVED1));
    printf ("  disableClear:              %s\n", prop_str (attrs & DISABLECLEAR));
    printf ("  inLockout:                 %s\n", prop_str (attrs & INLOCKOUT));
    printf ("  tpmGeneratedEPS:           %s\n", prop_str (attrs & TPMGENERATEDEPS));
    printf ("  reserved2:                 %s\n", prop_str (attrs & PERSISTENT_RESERVED2));
}
/*
 * Print string representations of the TPMA_STARTUP_CLEAR attributes.
 */
 
void dump_startup_clear_attrs (u32 attrs)
{
    printf ("TPM_PT_STARTUP_CLEAR:\n");
    printf ("  phEnable:                  %s\n", prop_str (attrs & PHENABLE));
    printf ("  shEnable:                  %s\n", prop_str (attrs & SHENABLE));
    printf ("  ehEnable:                  %s\n", prop_str (attrs & EHENABLE));
    printf ("  phEnableNV:                %s\n", prop_str (attrs & PHENABLENV));
    printf ("  reserved1:                 %s\n", prop_str (attrs & STARTUP_RESERVED1));
    printf ("  orderly:                   %s\n", prop_str (attrs & ORDERLY));
}

u32 buf_to_u32 (char *buf) {

	return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] ;
}

void get_uint32_as_chars (u32 value, char *buf)
{
    sprintf (buf, "%c%c%c%c",
             ((u8*)&value)[3],
             ((u8*)&value)[2],
             ((u8*)&value)[1],
             ((u8*)&value)[0]);
}

void dump_tpm_properties_fixed (char *properties)
{
	int i;
	char buf[5] = { 0, };

	u32 count = buf_to_u32(properties + RES_SIZE_ADDR);
	u32 value = 0;
	u32 property = 0;
//	printf("count_1: %d\n", count);
	count = ((count - GROUP_ADDR) / 8);
//	printf("count_2: %d\n", count);
	
	
	for (i = 0; i < count; ++i) {
			property = buf_to_u32(properties + GROUP_ADDR + (i * 8));
			value = buf_to_u32(properties + GROUP_ADDR + (i * 8) + 4);
			
			switch (property) {
			case TPM_PT_FAMILY_INDICATOR:
				get_uint32_as_chars (value, buf);
				printf ("TPM_PT_FAMILY_INDICATOR:\n"
				"  as UINT32:                0x08%x\n"
				"  as string:                \"%s\"\n",
				value,
				buf);
				break;
			case TPM_PT_LEVEL:
				printf ("TPM_PT_LEVEL:               %d\n", value);
				break;
			case TPM_PT_REVISION:
				printf ("TPM_PT_REVISION:            %d\n", (value / 100));
				break;
			case TPM_PT_DAY_OF_YEAR:
				printf ("TPM_PT_DAY_OF_YEAR:         0x%08x\n", value);
				break;
			case TPM_PT_YEAR:
				printf ("TPM_PT_YEAR:                0x%08x\n", value);
				break;
			case TPM_PT_MANUFACTURER:
				printf ("TPM_PT_MANUFACTURER:        0x%08x\n", value);
				break;
			case TPM_PT_VENDOR_STRING_1:
				get_uint32_as_chars (value, buf);
				printf ("TPM_PT_VENDOR_STRING_1:\n"
						"  as UINT32:                0x%08x\n"
						"  as string:                \"%s\"\n",
						value,
						buf);
				break;
			case TPM_PT_VENDOR_STRING_2:
				get_uint32_as_chars (value, buf);
				printf ("TPM_PT_VENDOR_STRING_2:\n"
						"  as UINT32:                0x%08x\n"
						"  as string:                \"%s\"\n",
						value,
						buf);
				break;
			case TPM_PT_VENDOR_STRING_3:
				get_uint32_as_chars (value, buf);
				printf ("TPM_PT_VENDOR_STRING_3:\n"
					"  as UINT32:                0x%08x\n"
					"  as string:                \"%s\"\n",
					value,
					buf);
				break;
			case TPM_PT_VENDOR_STRING_4:
				get_uint32_as_chars (value, buf);
				printf ("TPM_PT_VENDOR_STRING_4:\n"
					"  as UINT32:                0x%08x\n"
					"  as string:                \"%s\"\n",
					value,
					buf);
				break;
			case TPM_PT_VENDOR_TPM_TYPE:
				printf ("TPM_PT_VENDOR_TPM_TYPE:     0x%08x\n", value);
				break;
			case TPM_PT_FIRMWARE_VERSION_1:
				printf ("TPM_PT_FIRMWARE_VERSION_1:  0x%08x\n", value);
				break;
			case TPM_PT_FIRMWARE_VERSION_2:
				printf ("TPM_PT_FIRMWARE_VERSION_2:  0x%08x\n", value);
				break;
			case TPM_PT_INPUT_BUFFER:
				printf ("TPM_PT_INPUT_BUFFER:        0x%08x\n", value);
				break;
			case TPM_PT_HR_TRANSIENT_MIN:
				printf ("TPM_PT_HR_TRANSIENT_MIN:    0x%08x\n", value);
				break;
			case TPM_PT_HR_PERSISTENT_MIN:
				printf ("TPM_PT_HR_PERSISTENT_MIN:   0x%08x\n", value);
				break;
			case TPM_PT_HR_LOADED_MIN:
				printf ("TPM_PT_HR_LOADED_MIN:       0x%08x\n", value);
				break;
			case TPM_PT_ACTIVE_SESSIONS_MAX:
				printf ("TPM_PT_ACTIVE_SESSIONS_MAX: 0x%08x\n", value);
				break;
			case TPM_PT_PCR_COUNT:
				printf ("TPM_PT_PCR_COUNT:           0x%08x\n", value);
				break;
			case TPM_PT_PCR_SELECT_MIN:
				printf ("TPM_PT_PCR_SELECT_MIN:      0x%08x\n", value);
				break;
			case TPM_PT_CONTEXT_GAP_MAX:
				printf ("TPM_PT_CONTEXT_GAP_MAX:     0x%08x\n", value);
				break;
			case TPM_PT_NV_COUNTERS_MAX:
				printf ("TPM_PT_NV_COUNTERS_MAX:     0x%08x\n", value);
				break;
			case TPM_PT_NV_INDEX_MAX:
				printf ("TPM_PT_NV_INDEX_MAX:        0x%08x\n", value);
				break;
			case TPM_PT_MEMORY:
				printf ("TPM_PT_MEMORY:              0x%08x\n", value);
				break;
			case TPM_PT_CLOCK_UPDATE:
				printf ("TPM_PT_CLOCK_UPDATE:        0x%08x\n", value);
				break;
			case TPM_PT_CONTEXT_HASH: /* this may be a TPM_ALG_ID type */
				printf ("TPM_PT_CONTEXT_HASH:        0x%08x\n", value);
				break;
			case TPM_PT_CONTEXT_SYM: /* this is a TPM_ALG_ID type */
				printf ("TPM_PT_CONTEXT_SYM:         0x%08x\n", value);
				break;
			case TPM_PT_CONTEXT_SYM_SIZE:
				printf ("TPM_PT_CONTEXT_SYM_SIZE:    0x%08x\n", value);
				break;
			case TPM_PT_ORDERLY_COUNT:
				printf ("TPM_PT_ORDERLY_COUNT:       0x%08x\n", value);
				break;
			case TPM_PT_MAX_COMMAND_SIZE:
				printf ("TPM_PT_MAX_COMMAND_SIZE:    0x%08x\n", value);
				break;
			case TPM_PT_MAX_RESPONSE_SIZE:
				printf ("TPM_PT_MAX_RESPONSE_SIZE:   0x%08x\n", value);
				break;
			case TPM_PT_MAX_DIGEST:
				printf ("TPM_PT_MAX_DIGEST:          0x%08x\n", value);
				break;
			case TPM_PT_MAX_OBJECT_CONTEXT:
				printf ("TPM_PT_MAX_OBJECT_CONTEXT:  0x%08x\n", value);
				break;
			case TPM_PT_MAX_SESSION_CONTEXT:
				printf ("TPM_PT_MAX_SESSION_CONTEXT: 0x%08x\n", value);
				break;
			case TPM_PT_PS_FAMILY_INDICATOR:
				printf ("TPM_PT_PS_FAMILY_INDICATOR: 0x%08x\n", value);
				break;
			case TPM_PT_PS_LEVEL:
				printf ("TPM_PT_PS_LEVEL:            0x%08x\n", value);
				break;
			case TPM_PT_PS_REVISION:
				printf ("TPM_PT_PS_REVISION:         0x%08x\n", value);
				break;
			case TPM_PT_PS_DAY_OF_YEAR:
				printf ("TPM_PT_PS_DAY_OF_YEAR:      0x%08x\n", value);
				break;
			case TPM_PT_PS_YEAR:
				printf ("TPM_PT_PS_YEAR:             0x%08x\n", value);
				break;
			case TPM_PT_SPLIT_MAX:
				printf ("TPM_PT_SPLIT_MAX:           0x%08x\n", value);
				break;
			case TPM_PT_TOTAL_COMMANDS:
				printf ("TPM_PT_TOTAL_COMMANDS:      0x%08x\n", value);
				break;
			case TPM_PT_LIBRARY_COMMANDS:
				printf ("TPM_PT_LIBRARY_COMMANDS:    0x%08x\n", value);
				break;
			case TPM_PT_VENDOR_COMMANDS:
				printf ("TPM_PT_VENDOR_COMMANDS:     0x%08x\n", value);
				break;
			case TPM_PT_NV_BUFFER_MAX:
				printf ("TPM_PT_NV_BUFFER_MAX:       0x%08x\n", value);
				break;
			case TPM_PT_MODES:
				printf("TPM_PT_MODES: 0x%08x\n", value);
				break;
        }
    }
}


void dump_tpm_properties_var (char *properties)
{
	int i;

	u32 count = buf_to_u32(properties + RES_SIZE_ADDR);
	u32 value = 0;
	u32 property = 0;
	
	count = ((count - GROUP_ADDR) / 8);
	printf("count: %d\n", count);

	
	for (i = 0; i < count; i++) {
		
		property = buf_to_u32(properties + GROUP_ADDR + (i * 8));
		value = buf_to_u32(properties + GROUP_ADDR + (i * 8) + 4);
		
		switch (property) {
			case TPM_PT_PERMANENT:
				dump_permanent_attrs (value);
				break;
			case TPM_PT_STARTUP_CLEAR:
				dump_startup_clear_attrs (value);
				printf("dump_tpm_properties_var:%x\n", value);
				break;
			case TPM_PT_HR_NV_INDEX:
				printf ("TPM_PT_HR_NV_INDEX:          0x%08x\n", value);
				break;
			case TPM_PT_HR_LOADED:
				printf ("TPM_PT_HR_LOADED:            0x%08x\n", value);
				break;
			case TPM_PT_HR_LOADED_AVAIL:
				printf ("TPM_PT_HR_LOADED_AVAIL:      0x%08x\n", value);
				break;
			case TPM_PT_HR_ACTIVE:
				printf ("TPM_PT_HR_ACTIVE:            0x%08x\n", value);
				break;
			case TPM_PT_HR_ACTIVE_AVAIL:
				printf ("TPM_PT_HR_ACTIVE_AVAIL:      0x%08x\n", value);
				break;
			case TPM_PT_HR_TRANSIENT_AVAIL:
				printf ("TPM_PT_HR_TRANSIENT_AVAIL:   0x%08x\n", value);
				break;
			case TPM_PT_HR_PERSISTENT:
				printf ("TPM_PT_HR_PERSISTENT:        0x%08x\n", value);
				break;
			case TPM_PT_HR_PERSISTENT_AVAIL:
				printf ("TPM_PT_HR_PERSISTENT_AVAIL:  0x%08x\n", value);
				break;
			case TPM_PT_NV_COUNTERS:
				printf ("TPM_PT_NV_COUNTERS:          0x%08x\n", value);
				break;
			case TPM_PT_NV_COUNTERS_AVAIL:
				printf ("TPM_PT_NV_COUNTERS_AVAIL:    0x%08x\n", value);
				break;
			case TPM_PT_ALGORITHM_SET:
				printf ("TPM_PT_ALGORITHM_SET:        0x%08x\n", value);
				break;
			case TPM_PT_LOADED_CURVES:
				printf ("TPM_PT_LOADED_CURVES:        0x%08x\n", value);
				break;
			case TPM_PT_LOCKOUT_COUNTER:
				printf ("TPM_PT_LOCKOUT_COUNTER:      0x%08x\n", value);
				break;
			case TPM_PT_MAX_AUTH_FAIL:
				printf ("TPM_PT_MAX_AUTH_FAIL:        0x%08x\n", value);
				break;
			case TPM_PT_LOCKOUT_INTERVAL:
				printf ("TPM_PT_LOCKOUT_INTERVAL:     0x%08x\n", value);
				break;
			case TPM_PT_LOCKOUT_RECOVERY:
				printf ("TPM_PT_LOCKOUT_RECOVERY:     0x%08x\n", value);
				break;
			case TPM_PT_NV_WRITE_RECOVERY:
				printf ("TPM_PT_NV_WRITE_RECOVERY:    0x%08x\n", value);
				break;
			case TPM_PT_AUDIT_COUNTER_0:
				printf ("TPM_PT_AUDIT_COUNTER_0:      0x%08x\n", value);
				break;
			case TPM_PT_AUDIT_COUNTER_1:
				printf ("TPM_PT_AUDIT_COUNTER_1:      0x%08x\n", value);
				break;
			default:
				fprintf (stderr, "Unknown property:   0x%08x\n", property); //R01
		//R01		printf ("QQQQQQQQQQQQQQQQQQQQQQ:      0x%08x\n", value);
				break;
		}
	}
}

int tpm_tis_send(u8 *buf, size_t len)
{
//	printf("tpm_tis_send_1\n");
	
	return tpm_tis_send_main(buf, len, 0);
}


static int recv_data(u8 *buf, size_t count)
{
	size_t size = 0, burstcnt, transfer_size;
	
//	printf("recv_data_1\n");
	
	while (size < count && wait_for_tpm_stat(TPM_STS_DATA_AVAIL | TPM_STS_VALID) == 0) {
//		printf("recv_data_2\n");
		burstcnt = get_burstcount();
//		printf("recv_data_3:0x%x\n", burstcnt);
		transfer_size = min_t (size_t, (count - size), burstcnt);
//		printf("size %d count %d bct %d tsize %d\n", size, count, burstcnt, transfer_size);
		read_tpm_bytes(TPM_DATA_FIFO(vendor_locality), transfer_size, &buf[size]);
		
		size += transfer_size;
	}
	
	return size;
}


int tpm_tis_recv(u8 *buf, size_t count)
{
	int size = 0;
	int expected, status;

//	printf("tpm_tis_recv_1\n");
	
	if (count < TPM_HEADER_SIZE) {
		size = -1;
		goto out;
	}

//	printf("tpm_tis_recv_2: 0x%x\n", count);

	/* read first 10 bytes, including tag, paramsize, and result */
	size = recv_data(buf, TPM_HEADER_SIZE);

	if (size < TPM_HEADER_SIZE) {
		printf( "Unable to read header\n");
		goto out;
	}
	
//	printf("tpm_tis_recv_3:0x%x\n", size);
	
	expected = ((u32)buf[2] << 24 | (u32)buf[3] << 16 | (u32)buf[4] << 8  | (u32)buf[5]);
	
//	printf("tpm_tis_recv_4:0x%x\n", expected);
	
	if (expected > count) {
		size = -1;
		goto out;
	}
	
//	printf("tpm_tis_recv_5\n");
	
	
	size += recv_data(&buf[TPM_HEADER_SIZE], expected - TPM_HEADER_SIZE);
	
//	printf("tpm_tis_recv_6: 0x%x\n", size);
	
	if (size < expected) {
		printf("Unable to read remainder of result\n");
		size = -1;
		goto out;
	}
	
//	printf("tpm_tis_recv_7\n");
	
	wait_for_tpm_stat(TPM_STS_VALID);
//	printf("tpm_tis_recv_8\n");
	
	status = tpm_tis_status();
	
	if (status & TPM_STS_DATA_AVAIL) {      /* retry? */
		printf("Error left over data\n");
		size = -1;
		goto out;
	}

//	printf("tpm_tis_recv_9\n");
out:
//	printf("tpm_tis_recv_10 : size: %d\n", size);
	tpm_tis_ready();
	release_locality(vendor_locality, 0);
	return size;

}

ssize_t tpm_transmit(const char *buf, size_t bufsiz)
{
	ssize_t rc;
	u32 count;
	int cnt = 100;
	
//	printf("tpm_transmit_1\n");
	
	if (bufsiz > TPM_BUFSIZE)
		bufsiz = TPM_BUFSIZE;
		
	count = buf[5];
	
	if (count == 0)
		return -1;
	
	if (count > bufsiz) {
		printf("invalid count value %x %zx\n", count, bufsiz);
		return -1;
	}
	
//	printf("tpm_transmit_2\n");
	
	rc = tpm_tis_send((u8 *) buf, count);
	
//	printf("tpm_transmit_3\n");
	
	if (rc < 0) {
		printf("tpm_transmit: tpm_send: error %zd\n", rc);
		goto out;
	}
	
//	printf("tpm_transmit_4\n");
//	printf("tpm_transmit_5\n");
//	printf("tpm_transmit_6\n");
	
	do {
		u8 status = tpm_tis_status();
//		printf("status_1:%x\n", status);
		
		if ((status & 0x90) == 0x90)
			goto out_recv;
			
//		printf("status_2\n");

		if (status == TPM_STS_COMMAND_READY) {
			rc = -1;
			goto out;
		}

		udelay(1000 * 50);

	} while (cnt--);
	
//	printf("tpm_transmit_7\n");
	
	tpm_tis_ready();
	
	rc = -1;
//	printf("tpm_transmit_8\n");
	goto out;
	
out_recv:
//	printf("tpm_transmit_9\n");
	rc = tpm_tis_recv((u8 *) buf, bufsiz);
	
	if (rc < 0)
		printf("tpm_transmit: tpm_recv: error %zd\n", rc);
	
out:
//	printf("tpm_transmit_10, rc:%d\n", rc);
	return rc;
}


int tpm_write(const char *cmd, char *res)
{
	int out_size = 0;
//	int i = 0;

	memset(res, 0, 4096);
	memcpy(res, cmd, cmd[5]);

	out_size = tpm_transmit(res, 4096);

	if (out_size < 0) {
		printf("out_size\n");
		return out_size;
	}
		
//	for(i = 0; i < out_size; i++)
//		printf("res[%d]:0x%x\n", i, res[i]);

	return out_size;

}

#if 1
int tpm2_get_capability(int mode, char *res){

	struct tpm_output_header out = {0, 0, 0};
	int ret = 0;

	if (!tpm2_init_ok){
		printf("The tpm2 has not been initialized. Please call \'tpm2 init\' command  first.\n");
		return -1;
	}
	
	if (!tpm2_startup_ok) {
		printf("The tpm2 has not been startup. Please call \'tpm2 startup TPM2_SU_CLEAR\' command first.\n");
		return -1;
	}
	
	if (mode == TPM2_PROPERTIES_FIXED)
		ret = tpm_write(tpm2_getcapability_fixed, res);
	else
		ret = tpm_write(tpm2_getcapability_variable, res);

	if (ret < 0 )
		printf("tpm write fail\n");

	out.tag = res[0] << 8 | res[1];
	out.length = res[2] << 24 | res[3] << 16 | res[4] << 8 | res[5];
	out.return_code = res[6] << 24 | res[7] << 16 | res[8] << 8 | res[9];
	
//	printf("**********************************************************\n");
//	printf("*                   tpm2_get_capability                   \n");
//	printf("out.tag: 0x%x\n", out.tag);
//	printf("out.length: 0x%x\n", out.length);
//	printf("out.return_code: 0x%x\n", out.return_code);
//	printf("**********************************************************\n");

	return out.return_code;
}
#endif

static int do_tpm2_get_capability(cmd_tbl_t *cmdtp, int flag,
                int argc, char * const argv[])
{
        int mode;
	char res [4096] = {0};
	int ret = 0;	
	
        if (argc != 2)
                return CMD_RET_USAGE;
        if (!strcasecmp("properties-fixed", argv[1])) {
                mode = TPM2_PROPERTIES_FIXED;
        } else if (!strcasecmp("properties-variable", argv[1])) {
                mode = TPM2_PROPERTIES_VARIABLE;
        } else {
                printf("Couldn't recognize mode string: %s\n", argv[1]);
                return CMD_RET_FAILURE;
        }
	
	
	ret = tpm2_get_capability(mode, res);

	if (ret == 0) {
		if (mode == TPM2_PROPERTIES_VARIABLE)
			dump_tpm_properties_var(res);
		else
			dump_tpm_properties_fixed(res);
	}
	
        return convert_return_code(ret);
}

//R03 - Start
int tpm2_probe(void){

	u32 vendor = 0;
	int rc = 0;
	u8 rid = 0;
	u8 tmp = 0;

	if (wait_startup() != 0) {
		rc = -1;
		goto out_err;
	}
	
	if (request_locality(0) != 0) {
		rc = -ENODEV;
		goto out_err;
	}

	read_tpm_byte(TPM_DID_VID(0), &tmp);
	vendor |= (tmp << 0);
	
	read_tpm_byte(TPM_DID_VID(1), &tmp);
	vendor |= (tmp << 8);
	
	read_tpm_byte(TPM_DID_VID(2), &tmp);
	vendor |= (tmp << 16);
	
	read_tpm_byte(TPM_DID_VID(3), &tmp);
	vendor |= (tmp << 24);
	
	read_tpm_byte(TPM_RID(0), &rid);
	
	printf("%s TPM (device-id 0x%X, rev-id %d)\n",
					"2.0",
					vendor & 0xffff, rid);
	if ((vendor & 0xffff) != 0x15d1){
		rc = -1;
		goto out_err;
	}
					
out_err:
	return rc;
}

//OK
int tpm2_init(void) {
	
//	u32 vendor = 0;
	int rc = 0;
//	u8 rid = 0;
//	u8 tmp = 0;
//	int ret = 0;
	
	if (tpm2_init_ok) {
		printf("The tpm2 has been initialized.\n");
		return -1;
	}
	
//	gpio_request (PIO_TPM_RST, "TPM_RST");
//	gpio_direction_output (PIO_TPM_RST, 1);
//	udelay(500 * 1000);
//	gpio_direction_output (PIO_TPM_RST, 0);
//	udelay(500 * 1000);

	spi_tpm_init();

# if 0
	if (wait_startup() != 0) {
		rc = -1;
		goto out_err;
	}
	
	if (request_locality(0) != 0) {
		rc = -ENODEV;
		goto out_err;
	}
	
	
	read_tpm_byte(TPM_DID_VID(0), &tmp);
	vendor |= (tmp << 0);
	
	read_tpm_byte(TPM_DID_VID(1), &tmp);
	vendor |= (tmp << 8);
	
	read_tpm_byte(TPM_DID_VID(2), &tmp);
	vendor |= (tmp << 16);
	
	read_tpm_byte(TPM_DID_VID(3), &tmp);
	vendor |= (tmp << 24);
	
//R02	printf("vendor id: 0x%x\n", vendor);
	
	read_tpm_byte(TPM_RID(0), &rid);
	
	printf("%s TPM (device-id 0x%X, rev-id %d)\n",
					"2.0",
					vendor & 0xffff, rid);
//R01 - Start
	if ((vendor & 0xffff) != 0x15d1){
		rc = -1;
		ret = -1;
		goto out_err;
	}
//R01 - End					
#endif	
	rc = tpm2_probe();

	if (rc)
		goto out_err;

		
	tpm2_init_ok = 1;
	
out_err:
	
	if (rc)
		printf("TPM2 Init Fail![%d]\n", rc);
	else
		printf("TPM2 Init OK!\n");
	
    return rc;
}
//R03 - End


int tpm2_force_clear(void) {

	struct tpm_output_header out = {0,0,0};
	int ret = 0;
	char res [4096] = {0};

	if (!tpm2_init_ok){
		printf("The tpm2 has not been initialized. Please call \'tpm2 init\' command  first.\n");
		return -1;
	}
	
	if (!tpm2_startup_ok) {
		printf("The tpm2 has not been startup. Please call \'tpm2 startup TPM2_SU_CLEAR\' command first.\n");
		return -1;
	}
	
	
	ret = tpm_write(tpm2_clear_control, res);
	
	if (ret < 0)
		printf("tpm write fail\n");

	out.tag = res[0] << 8 | res[1];
	out.length = res[2] << 24 | res[3] << 16 | res[4] << 8 | res[5];
	out.return_code = res[6] << 24 | res[7] << 16 | res[8] << 8 | res[9];
	
//	printf("**********************************************************\n");
//	printf("*                   tpm2_clear_control                    \n");
//	printf("out.tag: 0x%x\n", out.tag);
//	printf("out.length: 0x%x\n", out.length);
//	printf("out.return_code: 0x%x\n", out.return_code);
//	printf("**********************************************************\n");

	if (out.return_code){
		printf("tpm2_clear_control Fail\n");
		goto EXIT;
	}
		
	ret = tpm_write(tpm2_clear, res);
	
	out.tag = res[0] << 8 | res[1];
	out.length = res[2] << 24 | res[3] << 16 | res[4] << 8 | res[5];
	out.return_code = res[6] << 24 | res[7] << 16 | res[8] << 8 | res[9];
	
//	printf("**********************************************************\n");
//	printf("*                      tpm2_clear                         \n");
//	printf("out.tag: 0x%x\n", out.tag);
//	printf("out.length: 0x%x\n", out.length);
//	printf("out.return_code: 0x%x\n", out.return_code);
//	printf("**********************************************************\n");
	
	if (out.return_code)
		printf("TPM2 Force Clear Fail![%d]\n", out.return_code);
	else
		printf("TPM2 Force Clear OK!\n");
	
EXIT:
	return out.return_code;

}

int tpm2_startup(int mode) {

	char res [4096] = {0};
	int ret = 0;
	struct tpm_output_header out = {0,0,0};
	
	if (!tpm2_init_ok){
		printf("The tpm2 has not been initialized. Please call \'tpm2 init\' command  first.\n");
		return -1;
	}
	
	if (tpm2_startup_ok) {
		printf("The tpm2 has been startup.\n");
		return -1;
	}

	if (mode == TPM2_SU_CLEAR)
		ret = tpm_write(tpm2_startup_clear, res);
	else
		ret = tpm_write(tpm2_startup_state, res);

	if (ret < 0)
		printf("tpm write fail\n");

	
	out.tag = res[0] << 8 | res[1];
	out.length = res[2] << 24 | res[3] << 16 | res[4] << 8 | res[5];
	out.return_code = res[6] << 24 | res[7] << 16 | res[8] << 8 | res[9];
	
//	printf("**********************************************************\n");
//	printf("out.tag: 0x%x\n", out.tag);
//	printf("out.length: 0x%x\n", out.length);
//	printf("out.return_code: 0x%x\n", out.return_code);
//	printf("**********************************************************\n");
	
	if (out.return_code == 0)
		tpm2_startup_ok++;
	
	
	return out.return_code;
}

static int do_tpm2_startup(cmd_tbl_t *cmdtp, int flag,
                int argc, char * const argv[])
{
	int ret = 0;
        int mode = 0;

        if (argc != 2)
                return CMD_RET_USAGE;
        if (!strcasecmp("TPM2_SU_CLEAR", argv[1])) {
                mode = TPM2_SU_CLEAR;
        } else if (!strcasecmp("TPM2_SU_STATE", argv[1])) {
                mode = TPM2_SU_STATE;
        } else {
                printf("Couldn't recognize mode string: %s\n", argv[1]);
                return CMD_RET_FAILURE;
        }

	ret = tpm2_startup(mode);
		
	if (ret)
		printf("TPM2 Startup (%d) Fail! [%d]\n", mode, ret);
	else
		printf("TPM2 Startup (%d) OK!\n", mode);

	return convert_return_code(ret);
}

int tpm2_hierarchy(int mode, int enable){

	char res [4096] = {0};
	__maybe_unused	int ret = 0;
	struct tpm_output_header out = {0,0,0};
	char auth = 0;
//	int  i = 0;
	
	char hierarchy_cmd [32] = {
		0x80, 0x02,    
		0x00, 0x00, 0x00, 0x20,
		0x00, 0x00, 0x01, 0x21,
		0x40, 0x00, 0x00, 0x0c,
		0x00, 0x00, 0x00, 0x09,
		0x40, 0x00, 0x00, 0x09,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x40, 0x00, 0x00,
		0x00, 0x00
	};

	if (!tpm2_init_ok){
		printf("The tpm2 has not been initialized. Please call \'tpm2 init\' command  first.\n");
		return -1;
	}
	
	if (!tpm2_startup_ok) {
		printf("The tpm2 has not been startup. Please call \'tpm2 startup TPM2_SU_CLEAR\' command first.\n");
		return -1;
	}

	if (mode == TPM2_OWNER)
		auth = TPM2_OWNER;
	else if(mode == TPM2_ENDORSEMENT) 
		auth = TPM2_ENDORSEMENT;
	else
		auth = TPM2_PLATFORM;
	
	hierarchy_cmd[30] = auth;
	hierarchy_cmd[31] = enable;
	
//	for (i = 0; i < 32; i++)
//		printf("hierarchy_cmd[%d]:%x\n", i, hierarchy_cmd[i]);
	
	ret = tpm_write(hierarchy_cmd, res);
	
	out.tag = res[0] << 8 | res[1];
	out.length = res[2] << 24 | res[3] << 16 | res[4] << 8 | res[5];
	out.return_code = res[6] << 24 | res[7] << 16 | res[8] << 8 | res[9];
	
//	printf("**********************************************************\n");
//	printf("out.tag: 0x%x\n", out.tag);
//	printf("out.length: 0x%x\n", out.length);
//	printf("out.return_code: 0x%x\n", out.return_code);
//	printf("**********************************************************\n");
	

	return out.return_code;

};

#if 1
static int do_tpm2_hierarchy_enable(cmd_tbl_t *cmdtp, int flag,
                int argc, char * const argv[])
{
        int mode = 0;
		int ret = 0;
		
        if (argc != 2)
                return CMD_RET_USAGE;
        if (!strcasecmp("TPM2_OWNER", argv[1])) {
                mode = TPM2_OWNER;
        } else if (!strcasecmp("TPM2_ENDORSEMENT", argv[1])) {
                mode = TPM2_ENDORSEMENT;
        } else if (!strcasecmp("TPM2_PLATFORM", argv[1])) {
                mode = TPM2_PLATFORM;
        } else {
                printf("Couldn't recognize mode string: %s\n", argv[1]);
                return CMD_RET_FAILURE;
        }
		
	ret = tpm2_hierarchy(mode, 1);
		
	if (ret) 
		printf("TPM2 hierarchy enable (%d) Fail! [%d]\n", mode, ret);
	else 
		printf("TPM2 hierarchy enable (%d) OK!\n", mode);

        return convert_return_code(ret);
}
#endif

#if 1
static int do_tpm2_hierarchy_disable(cmd_tbl_t *cmdtp, int flag,
                int argc, char * const argv[])
{
        int mode;
		int ret = 0;
		
        if (argc != 2)
                return CMD_RET_USAGE;
        if (!strcasecmp("TPM2_OWNER", argv[1])) {
                mode = TPM2_OWNER;
        } else if (!strcasecmp("TPM2_ENDORSEMENT", argv[1])) {
                mode = TPM2_ENDORSEMENT;
        } else if (!strcasecmp("TPM2_PLATFORM", argv[1])) {
                mode = TPM2_PLATFORM;
        } else {
                printf("Couldn't recognize mode string: %s\n", argv[1]);
                return CMD_RET_FAILURE;
        }
		
       	ret = tpm2_hierarchy(mode, 0);
		
	if (ret)
		printf("TPM2 hierarchy disable (%d) Fail! [%d]\n", mode, ret);
	else
		printf("TPM2 hierarchy disable (%d) OK!\n", mode);
		
        return convert_return_code(ret);
}
#endif


#define TPM_COMMAND_NO_ARG(cmd)                         \
static int do_##cmd(cmd_tbl_t *cmdtp, int flag,         \
                int argc, char * const argv[])          \
{                                                       \
        if (argc != 1)                                  \
                return CMD_RET_USAGE;                   \
        return convert_return_code(cmd());              \
}

TPM_COMMAND_NO_ARG(tpm2_init)
TPM_COMMAND_NO_ARG(tpm2_force_clear)


static cmd_tbl_t tpm2_commands[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, 
			do_tpm2_init, "", ""),
	U_BOOT_CMD_MKENT(startup, 0, 1, 
			do_tpm2_startup, "", ""),
	U_BOOT_CMD_MKENT(force_clear, 0, 1,  				
			do_tpm2_force_clear, "", ""),
	U_BOOT_CMD_MKENT(hierarchy_enable, 0, 1,
			do_tpm2_hierarchy_enable, "", ""),
	U_BOOT_CMD_MKENT(hierarchy_disable, 0, 1,
			do_tpm2_hierarchy_disable, "", ""),
	U_BOOT_CMD_MKENT(get_capability, 0, 1,
			do_tpm2_get_capability, "", ""),
};

static int do_tpm2(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *tpm2_cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	tpm2_cmd = find_cmd_tbl(argv[1], tpm2_commands, ARRAY_SIZE(tpm2_commands));
	
	if (!tpm2_cmd)
		return CMD_RET_USAGE;
	
	return tpm2_cmd->cmd(cmdtp, flag, argc - 1, argv + 1);
}

U_BOOT_CMD(tpm2, CONFIG_SYS_MAXARGS, 1, do_tpm2,
"Issue a TPM2 command",
"cmd args...\n"
"    - Issue TPM2 command <cmd> with arguments <args...>.\n"
"\n"
"Admin Startup and State Commands:\n"
"	init\n"
"  		- Put TPM2 into a state where it waits for 'startup' command.\n"
"  	startup <startup_type>\n"
"		- send startup command to the TPM chip. <startup type>. The value is either\n"
"		   TPM2_SU_CLEAR or TPM2_SU_STATE .\n"
"\n"
"Admin Opt-in Commands:\n"
"	hierarchy_disable <hierarchy>\n "
"       - The TPM2 will disable use of any persistent entity associated with the disabled hierarchy.\n"
"		   <hierarchy> is one of TPM2_OWNER, TPM2_ENDORSEMENT and TPM2_PLATFORM .\n"
"	hierarchy_enable <hierarchy>\n"
"       - The TPM2 will enable use of any persistent entity associated with the enabled hierarchy\n"
"		   <hierarchy> is one of TPM2_OWNER, TPM2_ENDORSEMENT and TPM2_PLATFORM .\n"
"\n"
"Admin Ownership Commands:\n"
"	force_clear\n"
"       - Issue TPM2_ForceClear command.\n"
"\n"
"The Capability Commands:\n"
"	get_capability <property> \n"
"       - This function takes a capability_opts_t structure as a parameter.\n"
"          Issue TPM2_Capability command.  <property> The value is either\n"
"          properties-fixed and properties-variable .\n"
);
