/*
 * board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include <power/tps65910.h>
#include <environment.h>
#include <watchdog.h>
#include <environment.h>
#include <model.h>
#include <pca953x.h>
#include "board.h"
#include "types.h"
#include <wdt_ds1374.h>

DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_DDR_VTT_EN		7

#if defined(CONFIG_SPL_BUILD) || \
	(defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_DM_ETH))
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
#endif

/*
 * Read header information from EEPROM into global structure.
 */
static int read_eeprom(struct am335x_baseboard_id *header)
{
	int model = 0;

	gpio_request(GPIO_2_13, "gpmc_ad12");
	gpio_direction_input(GPIO_2_13);
	gpio_request(GPIO_2_14, "gpmc_ad13");
	gpio_direction_input(GPIO_2_14);
	gpio_request(GPIO_2_15, "gpmc_ad14");
	gpio_direction_input(GPIO_2_15);
	gpio_request(GPIO_2_16, "gpmc_ad15");
	gpio_direction_input(GPIO_2_16);

        model = gpio_get_value(GPIO_2_13) | gpio_get_value(GPIO_2_14) << 1 | gpio_get_value(GPIO_2_15) << 2 | gpio_get_value(GPIO_2_16) << 3; 
	memset ((header), 0xff, sizeof (header));
	header->magic = 0xEE3355AA;
//	printf("model:%x\n", model);
	if (model == AM335X_UC8131) 
		memcpy ((header->name), "UC8131", 6);
	else if (model == AM335X_UC8132)
		memcpy ((header->name), "UC8132", 6);
	else if (model == AM335X_UC8162)
		memcpy ((header->name), "UC8162", 6);
	else if (model == AM335X_UC8112)
		memcpy ((header->name), "UC8112", 6);
	else
		memcpy ((header->name), "A335X_SK", 8);

//        memset ((header->name), '\0', 8);
//	memcpy ((header->name), "UC5100", 6);
	memcpy ((header->version), "1.2B", 4);
	memcpy ((header->serial), "45124P196288", 12);
	memcpy ((header->config), "SKU#00FFFFFFFFFFFFFFFFFFFFFFFFFF", 32);

	return 0;
}

#ifndef CONFIG_SKIP_LOWLEVEL_INIT

static const struct ddr_data micron_MT41K128M16HA125_ddr3_data = {
        .datardsratio0 = MT41K128M16HA125_RD_DQS,
        .datawdsratio0 = MT41K128M16HA125_WR_DQS,
        .datafwsratio0 = MT41K128M16HA125_PHY_FIFO_WE,
        .datawrsratio0 = MT41K128M16HA125_PHY_WR_DATA,
};

static const struct cmd_control micron_MT41K128M16HA125_ddr3_cmd_ctrl_data = {
        .cmd0csratio = MT41K128M16HA125_RATIO,
        .cmd0iclkout = MT41K128M16HA125_INVERT_CLKOUT,

        .cmd1csratio = MT41K128M16HA125_RATIO,
        .cmd1iclkout = MT41K128M16HA125_INVERT_CLKOUT,

        .cmd2csratio = MT41K128M16HA125_RATIO,
        .cmd2iclkout = MT41K128M16HA125_INVERT_CLKOUT,
};

static struct emif_regs micron_MT41K128M16HA125_ddr3_emif_reg_data = {
        .sdram_config = MT41K128M16HA125_EMIF_SDCFG,
        .ref_ctrl = MT41K128M16HA125_EMIF_SDREF,
        .sdram_tim1 = MT41K128M16HA125_EMIF_TIM1,
        .sdram_tim2 = MT41K128M16HA125_EMIF_TIM2,
        .sdram_tim3 = MT41K128M16HA125_EMIF_TIM3,
        .zq_config = MT41K128M16HA125_ZQ_CFG,
        .emif_ddr_phy_ctlr_1 = MT41K128M16HA125_EMIF_READ_LATENCY,
};

/*UC-8131 & UC-8132 ddr config*/
static const struct ddr_data UC813x_MT41K128M16JT125_ddr3_data = {
	.datardsratio0 = UC813x_MT41K128M16JT125_RD_DQS,
	.datawdsratio0 = UC813x_MT41K128M16JT125_WR_DQS,
	.datafwsratio0 = UC813x_MT41K128M16JT125_PHY_FIFO_WE,
	.datawrsratio0 = UC813x_MT41K128M16JT125_PHY_WR_DATA,
};

static const struct cmd_control UC813x_MT41K128M16JT125_ddr3_cmd_ctrl_data = {
	.cmd0csratio = UC813x_MT41K128M16JT125_RATIO,
        .cmd0iclkout = UC813x_MT41K128M16JT125_INVERT_CLKOUT,

        .cmd1csratio = UC813x_MT41K128M16JT125_RATIO,
        .cmd1iclkout = UC813x_MT41K128M16JT125_INVERT_CLKOUT,

        .cmd2csratio = UC813x_MT41K128M16JT125_RATIO,
        .cmd2iclkout = UC813x_MT41K128M16JT125_INVERT_CLKOUT,
};

static struct emif_regs UC813x_MT41K128M16JT125_ddr3_emif_reg_data = {
        .sdram_config = UC813x_MT41K128M16JT125_EMIF_SDCFG,
        .ref_ctrl = UC813x_MT41K128M16JT125_EMIF_SDREF,
        .sdram_tim1 = UC813x_MT41K128M16JT125_EMIF_TIM1,
        .sdram_tim2 = UC813x_MT41K128M16JT125_EMIF_TIM2,
        .sdram_tim3 = UC813x_MT41K128M16JT125_EMIF_TIM3,
        .zq_config = UC813x_MT41K128M16JT125_ZQ_CFG,
        .emif_ddr_phy_ctlr_1 = UC813x_MT41K128M16JT125_EMIF_READ_LATENCY,
};


/*UC-8162 ddr config*/
static const struct ddr_data UC8162_MT41K256M16TW107_ddr3_data = {
	.datardsratio0 = UC8162_MT41K256M16TW107_RD_DQS,
	.datawdsratio0 = UC8162_MT41K256M16TW107_WR_DQS,
	.datafwsratio0 = UC8162_MT41K256M16TW107_PHY_FIFO_WE,
	.datawrsratio0 = UC8162_MT41K256M16TW107_PHY_WR_DATA,
};

static const struct cmd_control UC8162_MT41K256M16TW107_ddr3_cmd_ctrl_data = {
	.cmd0csratio = UC8162_MT41K256M16TW107_RATIO,
        .cmd0iclkout = UC8162_MT41K256M16TW107_INVERT_CLKOUT,

        .cmd1csratio = UC8162_MT41K256M16TW107_RATIO,
        .cmd1iclkout = UC8162_MT41K256M16TW107_INVERT_CLKOUT,

        .cmd2csratio = UC8162_MT41K256M16TW107_RATIO,
        .cmd2iclkout = UC8162_MT41K256M16TW107_INVERT_CLKOUT,
};

static struct emif_regs UC8162_MT41K256M16TW107_ddr3_emif_reg_data = {
        .sdram_config = UC8162_MT41K256M16TW107_EMIF_SDCFG,
        .ref_ctrl = UC8162_MT41K256M16TW107_EMIF_SDREF,
        .sdram_tim1 = UC8162_MT41K256M16TW107_EMIF_TIM1,
        .sdram_tim2 = UC8162_MT41K256M16TW107_EMIF_TIM2,
        .sdram_tim3 = UC8162_MT41K256M16TW107_EMIF_TIM3,
        .zq_config = UC8162_MT41K256M16TW107_ZQ_CFG,
        .emif_ddr_phy_ctlr_1 = UC8162_MT41K256M16TW107_EMIF_READ_LATENCY,
};

/*UC-8112 ddr config*/
static const struct ddr_data UC8112_MT41K256M16TW107_ddr3_data = {
        .datardsratio0 = UC8112_MT41K256M16TW107_RD_DQS,
        .datawdsratio0 = UC8112_MT41K256M16TW107_WR_DQS,
        .datafwsratio0 = UC8112_MT41K256M16TW107_PHY_FIFO_WE,
        .datawrsratio0 = UC8112_MT41K256M16TW107_PHY_WR_DATA,
};

static const struct cmd_control UC8112_MT41K256M16TW107_ddr3_cmd_ctrl_data = {
        .cmd0csratio = UC8112_MT41K256M16TW107_RATIO,
        .cmd0iclkout = UC8112_MT41K256M16TW107_INVERT_CLKOUT,

        .cmd1csratio = UC8112_MT41K256M16TW107_RATIO,
        .cmd1iclkout = UC8112_MT41K256M16TW107_INVERT_CLKOUT,

        .cmd2csratio = UC8112_MT41K256M16TW107_RATIO,
        .cmd2iclkout = UC8112_MT41K256M16TW107_INVERT_CLKOUT,
};

static struct emif_regs UC8112_MT41K256M16TW107_ddr3_emif_reg_data = {
        .sdram_config = UC8112_MT41K256M16TW107_EMIF_SDCFG,
        .ref_ctrl = UC8112_MT41K256M16TW107_EMIF_SDREF,
        .sdram_tim1 = UC8112_MT41K256M16TW107_EMIF_TIM1,
        .sdram_tim2 = UC8112_MT41K256M16TW107_EMIF_TIM2,
        .sdram_tim3 = UC8112_MT41K256M16TW107_EMIF_TIM3,
        .zq_config = UC8112_MT41K256M16TW107_ZQ_CFG,
        .emif_ddr_phy_ctlr_1 = UC8112_MT41K256M16TW107_EMIF_READ_LATENCY,
};

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
#if 0
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_relocate_spec();
	if (getenv_yesno("boot_os") != 1)
		return 1;
#endif
#endif

	*(volatile unsigned int *)(0x4804c134) = 0xefffffff;
	*(volatile unsigned int *)(0x481ae134) = 0xffedffff;
	return !((*(volatile unsigned int *)(0x481ae138))&0x200000);
}
#endif

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		266, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_evm_sk = {
		303, OSC-1, 1, -1, -1, -1, -1};
const struct dpll_params dpll_ddr_bone_black = {
		400, OSC-1, 1, -1, -1, -1, -1};

void am33xx_spl_board_init(void)
{
	struct am335x_baseboard_id header;
	int mpu_vdd;

	if (read_eeprom(&header) < 0)
		puts("Could not get board ID.\n");

	/* Get the frequency */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

	if (board_is_bone(&header) || board_is_bone_lt(&header)) {
		/* BeagleBone PMIC Code */
		int usb_cur_lim;

		/*
		 * Only perform PMIC configurations if board rev > A1
		 * on Beaglebone White
		 */
		if (board_is_bone(&header) && !strncmp(header.version,
						       "00A1", 4))
			return;

		if (i2c_probe(TPS65217_CHIP_PM))
			return;

		/*
		 * On Beaglebone White we need to ensure we have AC power
		 * before increasing the frequency.
		 */
		if (board_is_bone(&header)) {
			uchar pmic_status_reg;
			if (tps65217_reg_read(TPS65217_STATUS,
					      &pmic_status_reg))
				return;
			if (!(pmic_status_reg & TPS65217_PWR_SRC_AC_BITMASK)) {
				puts("No AC power, disabling frequency switch\n");
				return;
			}
		}

		/*
		 * Override what we have detected since we know if we have
		 * a Beaglebone Black it supports 1GHz.
		 */
		if (board_is_bone_lt(&header))
			dpll_mpu_opp100.m = MPUPLL_M_1000;

		/*
		 * Increase USB current limit to 1300mA or 1800mA and set
		 * the MPU voltage controller as needed.
		 */
		if (dpll_mpu_opp100.m == MPUPLL_M_1000) {
			usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
			mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
		} else {
			usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
			mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
		}

		if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
				       TPS65217_POWER_PATH,
				       usb_cur_lim,
				       TPS65217_USB_INPUT_CUR_LIMIT_MASK))
			puts("tps65217_reg_write failure\n");

		/* Set DCDC3 (CORE) voltage to 1.125V */
		if (tps65217_voltage_update(TPS65217_DEFDCDC3,
					    TPS65217_DCDC_VOLT_SEL_1125MV)) {
			puts("tps65217_voltage_update failure\n");
			return;
		}

		/* Set CORE Frequencies to OPP100 */
		do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

		/* Set DCDC2 (MPU) voltage */
		if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
			puts("tps65217_voltage_update failure\n");
			return;
		}

		/*
		 * Set LDO3, LDO4 output voltage to 3.3V for Beaglebone.
		 * Set LDO3 to 1.8V and LDO4 to 3.3V for Beaglebone Black.
		 */
		if (board_is_bone(&header)) {
			if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
					       TPS65217_DEFLS1,
					       TPS65217_LDO_VOLTAGE_OUT_3_3,
					       TPS65217_LDO_MASK))
				puts("tps65217_reg_write failure\n");
		} else {
			if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
					       TPS65217_DEFLS1,
					       TPS65217_LDO_VOLTAGE_OUT_1_8,
					       TPS65217_LDO_MASK))
				puts("tps65217_reg_write failure\n");
		}

		if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
				       TPS65217_DEFLS2,
				       TPS65217_LDO_VOLTAGE_OUT_3_3,
				       TPS65217_LDO_MASK))
			puts("tps65217_reg_write failure\n");
	} else {
		int sil_rev;

		/*
		 * The GP EVM, IDK and EVM SK use a TPS65910 PMIC.  For all
		 * MPU frequencies we support we use a CORE voltage of
		 * 1.1375V.  For MPU voltage we need to switch based on
		 * the frequency we are running at.
		 */
		if (i2c_probe(TPS65910_CTRL_I2C_ADDR))
			return;

		/*
		 * Depending on MPU clock and PG we will need a different
		 * VDD to drive at that speed.
		 */
		sil_rev = readl(&cdev->deviceid) >> 28;
		mpu_vdd = am335x_get_tps65910_mpu_vdd(sil_rev,
						      dpll_mpu_opp100.m);

		/* Tell the TPS65910 to use i2c */
		tps65910_set_i2c_control();

		/* First update MPU voltage. */
		if (tps65910_voltage_update(MPU, mpu_vdd))
			return;

		/* Second, update the CORE voltage. */
		if (tps65910_voltage_update(CORE, TPS65910_OP_REG_SEL_1_1_3))
			return;

		/* Set CORE Frequencies to OPP100 */
		do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);
	}

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	struct am335x_baseboard_id header;

        u32 ID;
        i2c_set_bus_num(0);

	if (read_eeprom(&header) < 0)
		puts("Could not get board ID.\n");

	/*if (board_is_evm_sk(&header))
		return &dpll_ddr_evm_sk;
	else if (board_is_bone_lt(&header))
		return &dpll_ddr_bone_black;
	else if (board_is_evm_15_or_later(&header))
		return &dpll_ddr_evm_sk;
	else*/
	ID = readl(0x44E10604);
	if ( ID == 0x00FC0382 ){
	        printf("CPU:TI AM3352 ");
	        printf("DDR:400MHZ\n");
		return &dpll_ddr_bone_black;
        }else if ( ID == 0x20FD0383 ){
	        printf("CPU:TI AM3358 ");
	        printf("DDR:400MHZ\n");
		return &dpll_ddr_bone_black;
	}else{
	        printf("CPU:TI AM335x series ");
	        printf("DDR:266MHZ\n");
		return &dpll_ddr;
	}
}

void set_uart_mux_conf(void)
{
#if CONFIG_CONS_INDEX == 1
	enable_uart0_pin_mux();
#elif CONFIG_CONS_INDEX == 2
	enable_uart1_pin_mux();
#elif CONFIG_CONS_INDEX == 3
	enable_uart2_pin_mux();
#elif CONFIG_CONS_INDEX == 4
	enable_uart3_pin_mux();
#elif CONFIG_CONS_INDEX == 5
	enable_uart4_pin_mux();
#elif CONFIG_CONS_INDEX == 6
	enable_uart5_pin_mux();
#endif
}

void set_mux_conf_regs(void)
{
	__maybe_unused struct am335x_baseboard_id header;

	if (read_eeprom(&header) < 0)
		puts("Could not get board ID.\n");

	enable_board_pin_mux(&header);
}

const struct ctrl_ioregs ioregs_evmsk = {
	.cm0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_bonelt = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_evm15 = {
	.cm0ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_mxcore = {
	.cm0ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_UC813x = {
	.cm0ioctl               = UC813x_MT41K128M16JT125_IOCTRL_VALUE,
	.cm1ioctl               = UC813x_MT41K128M16JT125_IOCTRL_VALUE,
	.cm2ioctl               = UC813x_MT41K128M16JT125_IOCTRL_VALUE,
	.dt0ioctl               = UC813x_MT41K128M16JT125_IOCTRL_VALUE,
	.dt1ioctl               = UC813x_MT41K128M16JT125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_UC8162 = {
	.cm0ioctl               = UC8162_MT41K256M16TW107_IOCTRL_VALUE,
	.cm1ioctl               = UC8162_MT41K256M16TW107_IOCTRL_VALUE,
	.cm2ioctl               = UC8162_MT41K256M16TW107_IOCTRL_VALUE,
	.dt0ioctl               = UC8162_MT41K256M16TW107_IOCTRL_VALUE,
	.dt1ioctl               = UC8162_MT41K256M16TW107_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_UC8112 = {
	.cm0ioctl               = UC8112_MT41K256M16TW107_IOCTRL_VALUE,
	.cm1ioctl               = UC8112_MT41K256M16TW107_IOCTRL_VALUE,
	.cm2ioctl               = UC8112_MT41K256M16TW107_IOCTRL_VALUE,
	.dt0ioctl               = UC8112_MT41K256M16TW107_IOCTRL_VALUE,
	.dt1ioctl               = UC8112_MT41K256M16TW107_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= K4B4G1646D_IOCTRL_VALUE,
	.cm1ioctl		= K4B4G1646D_IOCTRL_VALUE,
	.cm2ioctl		= K4B4G1646D_IOCTRL_VALUE,
	.dt0ioctl		= K4B4G1646D_IOCTRL_VALUE,
	.dt1ioctl		= K4B4G1646D_IOCTRL_VALUE,
};

void wdt_enable(int ms)
{
        int s;
	u8 wdt_tmp = 0;

	wdt_tmp = i2c_reg_read(DS1374_WDT_ADDR, DS1374_CONTROL_ADDR);

	wdt_tmp &= ~DS1374_WDT_ENABLE;
	i2c_reg_write(DS1374_WDT_ADDR, DS1374_CONTROL_ADDR, wdt_tmp);

        s = ms * 4096 / 1000;

	i2c_reg_write(DS1374_WDT_ADDR, DS1374_WDT_COUNTER_0_ADDR, s & 0xff);
	i2c_reg_write(DS1374_WDT_ADDR, DS1374_WDT_COUNTER_1_ADDR, (s >> 8) & 0xff);
	i2c_reg_write(DS1374_WDT_ADDR, DS1374_WDT_COUNTER_2_ADDR, (s >> 16) & 0xff);
        i2c_reg_write(DS1374_WDT_ADDR, DS1374_CONTROL_ADDR, wdt_tmp | DS1374_WDT_ENABLE | 0x20);

}

void sdram_init(void)
{
	__maybe_unused struct am335x_baseboard_id header;
	u8 wdt_tmp = 0;

	if (read_eeprom(&header) < 0)
		puts("Could not get board ID.\n");

//        printf("DO TPM RESET\n");
	gpio_request (PIO_TPM_RST, "TPM_RST");
	gpio_direction_output (PIO_TPM_RST, 0);
	udelay(500 * 1000);
	gpio_direction_output (PIO_TPM_RST, 1);
	udelay(500 * 1000);
	gpio_direction_output (PIO_TPM_RST, 0);

	gpio_request (PIO_LED_MP_0, "DB0_LED");
	gpio_request (PIO_LED_MP_1, "DB1_LED");
	gpio_request (PIO_LED_MP_3, "DB2_LED");
	gpio_request (PIO_LED_MP_3, "DB3_LED");
	gpio_request (PIO_LED_MP_4, "DB4_LED");

	gpio_direction_output(PIO_LED_MP_0, 0);
	gpio_direction_output(PIO_LED_MP_1, 0);
	gpio_direction_output(PIO_LED_MP_2, 0);
	gpio_direction_output(PIO_LED_MP_3, 0);
	gpio_direction_output(PIO_LED_MP_4, 0);

	gpio_request (PIO_LED_ZIGBEE, "ZIGBEE_LED");
	gpio_request (PIO_LED_USB, "USB_LED");
	gpio_request (PIO_LED_SD, "SD_LED");

	gpio_direction_output(PIO_LED_ZIGBEE, 0);
	gpio_direction_output(PIO_LED_USB, 0);
	gpio_direction_output(PIO_LED_SD, 0);

	gpio_request (PIO_WLAN_EN, "WLAN_EN");
	gpio_direction_output(PIO_WLAN_EN, 1);

        wdt_enable(15000);

	if (board_is_evm_sk(&header)) {
		/*
		 * EVM SK 1.2A and later use gpio0_7 to enable DDR3.
		 * This is safe enough to do on older revs.
		 */
		gpio_request(GPIO_DDR_VTT_EN, "ddr_vtt_en");
		gpio_direction_output(GPIO_DDR_VTT_EN, 1);
		
		config_ddr(400, &ioregs_UC813x,
			&micron_MT41K128M16HA125_ddr3_data,
			&micron_MT41K128M16HA125_ddr3_cmd_ctrl_data,
			&micron_MT41K128M16HA125_ddr3_emif_reg_data, 0);
	}
	
	if (board_is_uc8131(&header)) {
		puts("Board: UC8131... DDR: MT41K128M16JT125...");
		config_ddr(400, &ioregs_UC813x,
			&UC813x_MT41K128M16JT125_ddr3_data,
			&UC813x_MT41K128M16JT125_ddr3_cmd_ctrl_data,
			&UC813x_MT41K128M16JT125_ddr3_emif_reg_data, 0);
	} else if (board_is_uc8132(&header)) {
		puts("Board: UC8132... DDR: MT41K128M16JT125...");
		config_ddr(400, &ioregs_UC813x,
			&UC813x_MT41K128M16JT125_ddr3_data,
			&UC813x_MT41K128M16JT125_ddr3_cmd_ctrl_data,
			&UC813x_MT41K128M16JT125_ddr3_emif_reg_data, 0);
	} else if (board_is_uc8162(&header)) {
		puts("Board: UC8162... DDR: MT41K256M16TW107...");
		config_ddr(400, &ioregs_UC8162,
			&UC8162_MT41K256M16TW107_ddr3_data,
			&UC8162_MT41K256M16TW107_ddr3_cmd_ctrl_data,
			&UC8162_MT41K256M16TW107_ddr3_emif_reg_data, 0);
	} else if (board_is_uc8112(&header)) {
		puts("Board: UC8112... DDR: MT41K256M16TW107...");
		config_ddr(400, &ioregs_UC8112,
			&UC8112_MT41K256M16TW107_ddr3_data,
			&UC8112_MT41K256M16TW107_ddr3_cmd_ctrl_data,
			&UC8112_MT41K256M16TW107_ddr3_emif_reg_data, 0);
	}

	puts("OK\n");

	am33xx_spl_board_init();
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#if defined(CONFIG_NOR) || defined(CONFIG_NAND)
	gpmc_init();
#endif
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char safe_string[HDR_NAME_LEN + 1];
	struct am335x_baseboard_id header;

	if (read_eeprom(&header) < 0)
		puts("Could not get board ID.\n");

	/* Now set variables based on the header. */
	strncpy(safe_string, (char *)header.name, sizeof(header.name));
	safe_string[sizeof(header.name)] = 0;
	setenv("board_name", safe_string);

	/* BeagleBone Green eeprom, board_rev: 0x1a 0x00 0x00 0x00 */
	if ( (header.version[0] == 0x1a) && (header.version[1] == 0x00) &&
	     (header.version[2] == 0x00) && (header.version[3] == 0x00) ) {
		setenv("board_rev", "BBG1");
	} else {
		strncpy(safe_string, (char *)header.version, sizeof(header.version));
		safe_string[sizeof(header.version)] = 0;
		setenv("board_rev", safe_string);
	}
#endif

	return 0;
}
#endif

#ifndef CONFIG_DM_ETH

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0x5,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 0x0,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};
#endif

/*
 * This function will:
 * Read the eFuse for MAC addresses, and set ethaddr/eth1addr/usbnet_devaddr
 * in the environment
 * Perform fixups to the PHY present on certain boards.  We only need this
 * function in:
 * - SPL with either CPSW or USB ethernet support
 * - Full U-Boot, with either CPSW or USB ethernet
 * Build in only these cases to avoid warnings about unused variables
 * when we build an SPL that has neither option but full U-Boot will.
 */
#if ((defined(CONFIG_SPL_ETH_SUPPORT) || defined(CONFIG_SPL_USBETH_SUPPORT)) \
		&& defined(CONFIG_SPL_BUILD)) || \
	((defined(CONFIG_DRIVER_TI_CPSW) || \
	  defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET)) && \
	 !defined(CONFIG_SPL_BUILD))
int board_eth_init(bd_t *bis)
{
	int rv, n = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	__maybe_unused struct am335x_baseboard_id header;
	int type = 0;
	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
	if (!getenv("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

#ifdef CONFIG_DRIVER_TI_CPSW

	mac_lo = readl(&cdev->macid1l);
	mac_hi = readl(&cdev->macid1h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!getenv("eth1addr")) {
		if (is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("eth1addr", mac_addr);
	}

	if (read_eeprom(&header) < 0)
		puts("Could not get board ID.\n");

	if (board_is_bone(&header) || board_is_bone_lt(&header) ||
	    board_is_idk(&header)) {
		writel(MII_MODE_ENABLE, &cdev->miisel);
		cpsw_slaves[0].phy_if = cpsw_slaves[1].phy_if =
				PHY_INTERFACE_MODE_MII;
	} else if (board_is_uc81(&header)) {
		cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_RMII;
		cpsw_slaves[1].phy_if = PHY_INTERFACE_MODE_RMII;
		type = (GMII1_SEL_RMII | GMII2_SEL_RMII | RMII2_IO_CLK_EN | RMII1_IO_CLK_EN);
		writel(type, &cdev->miisel);
	//	writel((GMII1_SEL_RMII | GMII2_SEL_RGMII | RGMII2_IDMODE | RMII1_IO_CLK_EN), &cdev->miisel);
		cpsw_slaves[0].phy_addr = 4;
		cpsw_slaves[1].phy_addr = 5;
	}

	rv = cpsw_register(&cpsw_data);

	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif

	/*
	 *
	 * CPSW RGMII Internal Delay Mode is not supported in all PVT
	 * operating points.  So we must set the TX clock delay feature
	 * in the AR8051 PHY.  Since we only support a single ethernet
	 * device in U-Boot, we only do this for the first instance.
	 */
#define AR8051_PHY_DEBUG_ADDR_REG	0x1d
#define AR8051_PHY_DEBUG_DATA_REG	0x1e
#define AR8051_DEBUG_RGMII_CLK_DLY_REG	0x5
#define AR8051_RGMII_TX_CLK_DLY		0x100

	if (board_is_evm_sk(&header) || board_is_gp_evm(&header)) {
		const char *devname;
		devname = miiphy_get_current_dev();

		miiphy_write(devname, 0x0, AR8051_PHY_DEBUG_ADDR_REG,
				AR8051_DEBUG_RGMII_CLK_DLY_REG);
		miiphy_write(devname, 0x0, AR8051_PHY_DEBUG_DATA_REG,
				AR8051_RGMII_TX_CLK_DLY);
	}
#endif
#if defined(CONFIG_USB_ETHER) && \
	(!defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_USBETH_SUPPORT))
	if (is_valid_ethaddr(mac_addr))
		eth_setenv_enetaddr("usbnet_devaddr", mac_addr);

	rv = usb_eth_initialize(bis);
	if (rv < 0)
		printf("Error %d registering USB_ETHER\n", rv);
	else
		n += rv;
#endif
	return n;
}
#endif

#endif /* CONFIG_DM_ETH */
