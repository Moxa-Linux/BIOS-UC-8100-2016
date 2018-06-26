/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
#ifndef _CMD_BIOS_H_
#define _CMD_BIOS_H_


#define DIAG_OK				0
#define DIAG_FAIL			1
#define DIAG_ESC			2
#define DIAG_GET_VALUE_HEX_MODE		0
#define DIAG_GET_VALUE_DEC_MODE		1
#define DIAG_GET_VALUE_NO_ABORT		0x80000000L

#define T0_0_FLAG			(0x1 << 0)
#define T0_1_FLAG			(0x1 << 1)
#define T1_0_FLAG			(0x1 << 2)
#define T1_1_FLAG			(0x1 << 3)
#define T1_2_FLAG			(0x1 << 4)
#define	EOT_FLAG			(0x1 << 5)
#define T2_FLAG				(0x1 << 6)
#define T3_0_FLAG			(0x1 << 7)
#define T3_1_FLAG			(0x1 << 8)
#define T3_2_FLAG			(0x1 << 9)
#define MODULE_T0_FLAG			(0x1 << 10)
#define MODULE_T1_FLAG			(0x1 << 11)
#define T3_3_FLAG			(0x1 << 12)	//R05
#define T3_4_FLAG			(0x1 << 13)	//R06
#define T1_3_FLAG			(0x1 << 14) 
#define T3_5_FLAG			(0x1 << 15)	
#define T3_6_FLAG			(0x1 << 16)	

#define boot_SD_Protected		(0x1 << 0)
#define extend_SD_Protected		(0x1 << 1)
#define extend_USB_port_crtl		(0x1 << 2)

// *** Be careful ! ......... You must modify struct _data_pattern and bios_init_global_value() , too.
#define DATA_PATTERN0			(0x5AA55AA5L)
#define DATA_PATTERN1			(0xA55AA55AL)
#define DATA_PATTERN2			(0xAAAAAAAAL)
#define DATA_PATTERN3			(0x55555555L)
#define DATA_PATTERN4			(0x00000000L)
#define DATA_PATTERN5			(0xFFFFFFFFL)		// *** Be careful !

#define EMMC_COPY_LIMIT_SIZE		31457280        
#define MAX_SIZE_16BYTE			16
#define MAX_SIZE_32BYTE			32
#define MAX_SIZE_64BYTE			64
#define MAX_SIZE_128BYTE		128
#define MAX_SIZE_256BYTE		256

#define MAX_MN_SIZE			80
#define MN_BASE_ADDR			4
#define MAX_SN_SIZE			20
#define SN_BASE_ADDR			84
#define MAX_CV_SIZE			20
#define CV_BASE_ADDR			114
#define EEPROM_I2C_ADDR			0x50

#define DS1374_TOD_CNT_BYTE0_ADDR		0x00 /* TimeOfDay */
#define DS1374_TOD_CNT_BYTE1_ADDR		0x01
#define DS1374_TOD_CNT_BYTE2_ADDR		0x02
#define DS1374_TOD_CNT_BYTE3_ADDR		0x03

#define DS1374_WD_ALM_CNT_BYTE0_ADDR		0x04
#define DS1374_WD_ALM_CNT_BYTE1_ADDR		0x05
#define DS1374_WD_ALM_CNT_BYTE2_ADDR		0x06

#define DS1374_CTL_ADDR				0x07 /* RTC-CoNTrol-register */
#define DS1374_SR_ADDR				0x08 /* RTC-StatusRegister */
#define DS1374_TCS_DS_ADDR			0x09 /* RTC-TrickleChargeSelect DiodeSelect-register */

#define DS1374_CTL_BIT_AIE                 (1<<0) /* Bit 0 - Alarm Interrupt enable */
#define DS1374_CTL_BIT_RS1                 (1<<1) /* Bit 1/2 - Rate Select square wave output */
#define DS1374_CTL_BIT_RS2                 (1<<2) /* Bit 2/2 - Rate Select square wave output */
#define DS1374_CTL_BIT_WDSTR               (1<<3) /* Bit 3 - Watchdog Reset Steering */
#define DS1374_CTL_BIT_BBSQW               (1<<4) /* Bit 4 - Battery-Backed Square-Wave */
#define DS1374_CTL_BIT_WD_ALM              (1<<5) /* Bit 5 - Watchdoc/Alarm Counter Select */
#define DS1374_CTL_BIT_WACE                (1<<6) /* Bit 6 - Watchdog/Alarm Counter Enable WACE*/
#define DS1374_CTL_BIT_EN_OSC              (1<<7) /* Bit 7 - Enable Oscilator */

#define MAX_HW_VERSION_SIZE                 10
#define HW_VERSION_BASE_ADDR    (SN_BASE_ADDR + MAX_SN_SIZE)

enum {
	T1_MODE = 0,
	FG_MODE
};

enum {
	USB_FLASH = 1,
	USB_UART ,
	USB_CELLULAR
};

enum {
	CELLULAR_SIERRA = 1,
	CELLULAR_UBLOX,
	CELLULAR_PEM
};

enum {
	UART_RS232 = 0,
	UART_RS422 = 0,
	UART_RS485_2W = 0,

};

enum {
	HW_MAJ1 = 0,
	HW_MIN1,
	HW_MAJ2,
	HW_MIN2,
	HW_MAJ3,
	HW_MIN3,
	HW_MAJ4,
	HW_MIN4,
	HW_MAJ5,
	HW_MIN5,
};

enum {
	BACKUP_ENV_OK = 0,
	BACKUP_ENV_FAIL,
};
/*------------------------------------------------------ Structure ----------------------------------*/
struct burnin_cmd_T{
	int index;
	char symbol;
	char *string;		/* command name */
	void (*burnin_routine)(void);		/* implementing routine */
	char exist;          /* IP exist */
};//__attribute__((packed));
typedef struct burnin_cmd_T burnin_cmd;

struct _packet_info {
	long length;
	unsigned char wbuf[2048];
	unsigned char rbuf[2048];
}__attribute__((packed));

typedef struct _packet_info packet_info;
typedef struct _packet_info *packet_info_p;

typedef struct _data_pattern{
	unsigned long pat0;
	unsigned long pat1;
	unsigned long pat2;
	unsigned long pat3;
	unsigned long pat4;
	unsigned long pat5;
}data_pattern, *data_pattern_p;

struct _RFModulelInfo
{
   	unsigned short		vendor;
   	unsigned short		product;
   	unsigned char		out_bulk;
   	unsigned char		in_bulk;
   	unsigned char		gps_out_bulk; //R11B
   	unsigned char		gps_in_bulk; //R11B
	char			modelName[32];
}__attribute__((packed));
typedef struct _RFModulelInfo RFModulelInfo;

int sys_board_init(void);

// ----------- Utilities ---------------
void bios_print_item_msg(burnin_cmd *cmd_list);
int bios_parse_cmd(char* Buffer);
char bios_parse_cmd_char(char* buffer);
int bios_parse_num(char *s, unsigned int *val, char **es, char *delim);
int diag_get_value(char *title, unsigned int *val, unsigned int min, unsigned int max, int mode);
void bios_init_global_value(void);
int bios_manual_testing(burnin_cmd *cmd_list);

// ----------- Function --------------
void diag_do_set_clear_flag_func(void);
void diag_do_download_upload_func(void);
void diag_do_sn_mac_config_func(void);
void diag_do_tftp_setting_func(void);
void diag_do_run_kernel_func(void);
void diag_do_uboot(void);
void MP_PROGRAM(int mode);
void Factory_Test_Flow(void);

void diag_do_save_backup_env(void);
void diag_do_clear_backup_env(void);
void diag_do_recovery_env_from_backup(void);
void diag_do_check_backup_env(void);
void diag_do_import_backup_env(void);

// ----------- FLAG Menu --------------
unsigned int diag_get_mm_flag(unsigned int flag);
void diag_set_mm_flag(unsigned long flag);
void diag_print_env_conf(const char *name);
unsigned int diag_get_env_conf_u32(const char *name);
void diag_set_env_conf_u32(const char *name, unsigned int value);

// ----------- ETH Menu --------------
void diag_do_eth_func(void);

// ----------- UART Menu -----------------
void diag_do_uart_func(void);

// ----------- IO Menu --------------
void diag_do_io_func(void);

// ----------- RTC Menu --------------
void diag_do_rtc_func(void);

// ----------- USB HOST Menu --------------
void diag_do_usb_host_func(void);

// ----------- MEMORY/STORAGE Menu --------------
void diag_do_memory_storage_func(void);

// ----------- SDRAM Menu --------------
void diag_do_sdram_func(void);

// ----------- QA Test --------------
void diag_do_qa_test_func(void);

// ----------- TPM MENU --------------
// diag_do_tpm2_config
void diag_do_tpm2_func(void);

// ----------- SD MENU --------------- 
void diag_do_sd_func(void);
void diag_do_set_clear_backup_env_func(void);

int Check_CPU_Type(unsigned char show);
void diag_do_tpm_config(void);
void diag_do_led_func(void);
int diag_do_tpm_disabled_func(unsigned char flag);
int diag_do_sd_card_detect_func(unsigned char port,unsigned char flag);
void diag_do_run_mmc_func(void);
void diag_do_sd_protected_config(void);
void diag_do_extend_usb_port_control(void);
int diag_do_uart_232_RTS_CTS_test (void);
void diag_do_copy_download_firmware_to_emmc(void);
void diag_do_tftp_download_firmware(void);
// ----------- WDT Menu --------------
void diag_do_wdt_func (void);
int OLED_Test(void);
int tpm2_cfg(void);
void diag_do_tpm2_config(void);
void boot_counter(void);
void count_tester(void);
void pca9535_init(void);
int do_get_hw_verison(char *s);
void diag_do_usb_uart_test(uint8_t mode);
#endif

