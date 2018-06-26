/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    model.h

    2017-09-09	HsienWen tsai.			Create it.
*/

#ifndef _MODEL_H
#define _MODEL_H

#define K4B4G1646B 1
#define K4B2G1646B 2
#define K4B4G1646D 3

#define VersionMajor 1
#define VersionMinor 0

enum {
	AM335X_UC5101 = 0,
	AM335X_UC5102,
	AM335X_UC5111,
	AM335X_UC5112,
};

enum {
	AM335X_UC8131 = 0,
	AM335X_UC8132,
	AM335X_UC8162,
	AM335X_UC8112,
};

#define DDR_TYPE       K4B4G1646D
#include <asm/arch/ddr_defs.h>

#define	Update_BIOS_Ver	                1
const char __weak BIOS_version_string[] = "1.0.0S09";

#define PCA9535_LED_UC8100ME 

#define PCA9535_UART_MODE_W6000

#define PCA9535_UART_MODE_SHIFT_1 (0)
#define PCA9535_UART_MODE_SHIFT_2 (4)
#define PCA9535_UART_MODE_SHIFT_3 (8)
#define PCA9535_UART_MODE_SHIFT_4 (12)

/*int _serial_tstc(const int port);
void _serial_puts(const char *s, const int port);
int _serial_getc(const int port);
void _serial_setbrg(const int port);*/

#define UART_PORT4                  4
#define UART_PORT3                  3

/*********************************************************
 *			MPTEST DEFINE
 *********************************************************/
#define CPU_MP                                  1
#define SDRAM_MP				2
#define LAN_10M_CROSSCABLE_MP			3
#define LAN_100M_CROSSCABLE_MP			4
#define RTC_MP					5
#define	SD_MP					6
#define	USBH_MP					7
//#define	RS232_MP				7
//#define	RS422_MP				8
//#define	RS4852W_MP				9
#define	USB_CELLULAR_MP				8
//#define	TPM_MP					12
#define	TPM2_MP					9
//#define DIO_MP                                  13
#define	LED_MP					10
#define WDT_MP					11
#define DCAN_MP                                 12
#define CELLULAR_GPS_MP			1
#define CELLULAR_GSM_MP			2

#define SD_0					1
#define LAN1_10M				1
#define LAN2_10M				1
#define LAN1_100M				1
#define LAN2_100M				1
#define RTC_DS1374				3
#define RTC_TEST_CHIP				RTC_DS1374
#define	SD_LED          			1
#define	Zigbee_LED				1
#define	USB_PLUG_LED				1
#define BIOS_DARAM_TEST				1
#define BIOS_RTC_TEST				1
#define BIOS_USBHOST_TEST			1
#define BIOS_10M_TEST				1
#define BIOS_100M_TEST				1

#define SIM_NUM                                 1

/*****************************************************
 *			GPIO DEFINE
 ****************************************************/
#define AM335x_GPIO(a,b)                        ( (a * 32) + b) 

#define PIO_LED_SIG_1				67
#define PIO_LED_SIG_2				(1 << 9)
#define PIO_LED_SIG_3				(1 << 10)
#define PIO_LED_CEL_1				AM335x_GPIO(1,4)
#define PIO_LED_CEL_2				AM335x_GPIO(1,5)
#define PIO_LED_CEL_3				AM335x_GPIO(1,6)
#define PIO_LED_DIA_1			        AM335x_GPIO(1,12)
#define PIO_LED_DIA_2				AM335x_GPIO(1,13)
#define PIO_LED_DIA_3				AM335x_GPIO(1,14)
#define PIO_LED_USR_1				(1 << 8)
#define PIO_LED_USR_2				(1 << 9)
#define PIO_LED_USR_3				(1 << 10)
#define PIO_SW_BUTTON				AM335x_GPIO(2,1)
#define PIO_LED_ZIGBEE				AM335x_GPIO(2,2)
#define PIO_LED_SD				AM335x_GPIO(2,3)
#define	PIO_LED_USB				AM335x_GPIO(2,4)

#define PIO_LED_MP_0                            AM335x_GPIO(2,6)	
#define PIO_LED_MP_1                            AM335x_GPIO(2,7)	
#define PIO_LED_MP_2                            AM335x_GPIO(2,8)	
#define PIO_LED_MP_3                            AM335x_GPIO(2,9)	
#define PIO_LED_MP_4                            AM335x_GPIO(2,10)	

#define	GPIO_2_13				AM335x_GPIO(2,13)
#define	GPIO_2_14				AM335x_GPIO(2,14)
#define	GPIO_2_15				AM335x_GPIO(2,15)
#define	GPIO_2_16			        AM335x_GPIO(2,16)

#define PIO_WLAN_EN				AM335x_GPIO(2,22)
#define PIO_WLAN_IRQ				AM335x_GPIO(2,23)
#define PIO_WLAN_RST				AM335x_GPIO(2,24)
#define PIO_WLAN_DIS				AM335x_GPIO(2,25)

#define PIO_BUZZER			        60
#define PIO_UART_NUMBER				61
#define PIO_MPE_PWR_EN				30
#define PIO_MPE_PG				87
#define PIO_MPE_RST				88
#define PIO_MPE_DIS				89
#define PIO_TPM_FORCE_CLEAR			AM335x_GPIO(0,8)
#define PIO_TPM_INT				AM335x_GPIO(3,16)
#define PIO_TPM_RST				AM335x_GPIO(3,17)
#define PIO_PMIC_WAKE_UP			AM335x_GPIO(3,18)
#define PIO_PMIC_INT1				AM335x_GPIO(3,19)
#define PIO_PMIC_SLEEP				AM335x_GPIO(3,20)
#define PIO_FASTBOOT				AM335x_GPIO(3,21)
/*#define PIO_USER_LED				20
#define PIO_LED_READY				PIO_USER_LED*/
//#define PIO_CPU_DO1                             54
//#define PIO_CPU_DO2                             55
//#define PIO_CPU_DO3                             56
//#define PIO_CPU_DO4                             57
//#define PIO_CPU_DI1                             86
//#define PIO_CPU_DI2                             87
//#define PIO_CPU_DI3                             88
//#define PIO_CPU_DI4                             89

#endif //_MODEL_H

