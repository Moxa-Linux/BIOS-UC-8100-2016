/*
 * ddr_defs.h
 *
 * ddr specific header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DDR_DEFS_H
#define _DDR_DEFS_H

#include <asm/arch/hardware.h>
#include <asm/emif.h>

/* AM335X EMIF Register values */
#define VTP_CTRL_READY		(0x1 << 5)
#define VTP_CTRL_ENABLE		(0x1 << 6)
#define VTP_CTRL_START_EN	(0x1)
#ifdef CONFIG_AM43XX
#define DDR_CKE_CTRL_NORMAL	0x3
#else
#define DDR_CKE_CTRL_NORMAL	0x1
#endif
#define PHY_EN_DYN_PWRDN	(0x1 << 20)

/* Micron MT47H128M16RT-25E */
#define MT47H128M16RT25E_EMIF_READ_LATENCY	0x100005
#define MT47H128M16RT25E_EMIF_TIM1		0x0666B3C9
#define MT47H128M16RT25E_EMIF_TIM2		0x243631CA
#define MT47H128M16RT25E_EMIF_TIM3		0x0000033F
#define MT47H128M16RT25E_EMIF_SDCFG		0x41805332
#define MT47H128M16RT25E_EMIF_SDREF		0x0000081a
#define MT47H128M16RT25E_RATIO			0x80
#define MT47H128M16RT25E_RD_DQS			0x12
#define MT47H128M16RT25E_PHY_WR_DATA		0x40
#define MT47H128M16RT25E_PHY_FIFO_WE		0x80
#define MT47H128M16RT25E_IOCTRL_VALUE		0x18B

/* Micron MT41J128M16JT-125 */
#define MT41J128MJT125_EMIF_READ_LATENCY	0x100006
#define MT41J128MJT125_EMIF_TIM1		0x0888A39B
#define MT41J128MJT125_EMIF_TIM2		0x26337FDA
#define MT41J128MJT125_EMIF_TIM3		0x501F830F
#define MT41J128MJT125_EMIF_SDCFG		0x61C04AB2
#define MT41J128MJT125_EMIF_SDREF		0x0000093B
#define MT41J128MJT125_ZQ_CFG			0x50074BE4
#define MT41J128MJT125_RATIO			0x40
#define MT41J128MJT125_INVERT_CLKOUT		0x1
#define MT41J128MJT125_RD_DQS			0x3B
#define MT41J128MJT125_WR_DQS			0x85
#define MT41J128MJT125_PHY_WR_DATA		0xC1
#define MT41J128MJT125_PHY_FIFO_WE		0x100
#define MT41J128MJT125_IOCTRL_VALUE		0x18B

/* Micron MT41K128M16JT-187E */
#define MT41K128MJT187E_EMIF_READ_LATENCY	0x06
#define MT41K128MJT187E_EMIF_TIM1		0x0888B3DB
#define MT41K128MJT187E_EMIF_TIM2		0x36337FDA
#define MT41K128MJT187E_EMIF_TIM3		0x501F830F
#define MT41K128MJT187E_EMIF_SDCFG		0x61C04AB2
#define MT41K128MJT187E_EMIF_SDREF		0x0000093B
#define MT41K128MJT187E_ZQ_CFG			0x50074BE4
#define MT41K128MJT187E_RATIO			0x40
#define MT41K128MJT187E_INVERT_CLKOUT		0x1
#define MT41K128MJT187E_RD_DQS			0x3B
#define MT41K128MJT187E_WR_DQS			0x85
#define MT41K128MJT187E_PHY_WR_DATA		0xC1
#define MT41K128MJT187E_PHY_FIFO_WE		0x100
#define MT41K128MJT187E_IOCTRL_VALUE		0x18B

/* Micron MT41K128M16JT-125 */
#define UC813x_MT41K128M16JT125_IOCTRL_VALUE		0x1EF
#define UC813x_MT41K128M16JT125_EMIF_SDCFG		0x61C452B2
#define UC813x_MT41K128M16JT125_EMIF_SDREF		0x00000618
#define UC813x_MT41K128M16JT125_ZQ_CFG			0x50074BE4
#define UC813x_MT41K128M16JT125_EMIF_TIM1                 0x0AAAD4DB
#define UC813x_MT41K128M16JT125_EMIF_TIM2                 0x2A437FDA
#define UC813x_MT41K128M16JT125_EMIF_TIM3                 0x501F83FF
#define UC813x_MT41K128M16JT125_EMIF_READ_LATENCY         0x100007
#define UC813x_MT41K128M16JT125_RD_DQS			0x36
#define UC813x_MT41K128M16JT125_PHY_FIFO_WE		0x9A
#define UC813x_MT41K128M16JT125_WR_DQS			0x3E
#define UC813x_MT41K128M16JT125_PHY_WR_DATA		0x79
#define UC813x_MT41K128M16JT125_RATIO			0x78
#define UC813x_MT41K128M16JT125_INVERT_CLKOUT		0x0
#define UC813x_MT41K128M16JT125_DLL_LOCK_DIFF	        0x1

/* Micron 8112 MT41K256M16TW-107 */
#define UC8112_MT41K256M16TW107_IOCTRL_VALUE		0x1EF
#define UC8112_MT41K256M16TW107_EMIF_SDCFG		0x61C45332
#define UC8112_MT41K256M16TW107_EMIF_SDREF		0x00000618
#define UC8112_MT41K256M16TW107_ZQ_CFG                    0x50074BE4
#define UC8112_MT41K256M16TW107_EMIF_TIM1                 0x0AAAD4DB
#define UC8112_MT41K256M16TW107_EMIF_TIM2                 0x266B7FDA
#define UC8112_MT41K256M16TW107_EMIF_TIM3                 0x501F867F
#define UC8112_MT41K256M16TW107_EMIF_READ_LATENCY         0x100007
#define UC8112_MT41K256M16TW107_RD_DQS                    0x37
#define UC8112_MT41K256M16TW107_PHY_FIFO_WE		0x97
#define UC8112_MT41K256M16TW107_WR_DQS		        0x3B
#define UC8112_MT41K256M16TW107_PHY_WR_DATA		0x88
#define UC8112_MT41K256M16TW107_RATIO			0x79
#define UC8112_MT41K256M16TW107_INVERT_CLKOUT		0x0
#define UC8112_MT41K256M16TW107_DLL_LOCK_DIFF	        0x1

/* Micron 8162 MT41K256M16TW-107 */
#define UC8162_MT41K256M16TW107_IOCTRL_VALUE		0x1EF
#define UC8162_MT41K256M16TW107_EMIF_SDCFG		0x61C45332
#define UC8162_MT41K256M16TW107_EMIF_SDREF		0x00000618
#define UC8162_MT41K256M16TW107_ZQ_CFG                    0x50074BE4
#define UC8162_MT41K256M16TW107_EMIF_TIM1                 0x0AAAD4DB
#define UC8162_MT41K256M16TW107_EMIF_TIM2                 0x266B7FDA
#define UC8162_MT41K256M16TW107_EMIF_TIM3                 0x501F867F
#define UC8162_MT41K256M16TW107_EMIF_READ_LATENCY         0x100007
#define UC8162_MT41K256M16TW107_RD_DQS                    0x38
#define UC8162_MT41K256M16TW107_PHY_FIFO_WE		0x97
#define UC8162_MT41K256M16TW107_WR_DQS		        0x3B
#define UC8162_MT41K256M16TW107_PHY_WR_DATA		0x88
#define UC8162_MT41K256M16TW107_RATIO			0x7D
#define UC8162_MT41K256M16TW107_INVERT_CLKOUT		0x0
#define UC8162_MT41K256M16TW107_DLL_LOCK_DIFF	        0x1

/* Micron MT41J64M16JT-125 */
#define MT41J64MJT125_EMIF_SDCFG		0x61C04A32

/* Micron MT41J256M16JT-125 */
#define MT41J256MJT125_EMIF_SDCFG		0x61C04B32

/* Micron MT41J256M8HX-15E */
#define MT41J256M8HX15E_EMIF_READ_LATENCY	0x100006
#define MT41J256M8HX15E_EMIF_TIM1		0x0888A39B
#define MT41J256M8HX15E_EMIF_TIM2		0x26337FDA
#define MT41J256M8HX15E_EMIF_TIM3		0x501F830F
#define MT41J256M8HX15E_EMIF_SDCFG		0x61C04B32
#define MT41J256M8HX15E_EMIF_SDREF		0x0000093B
#define MT41J256M8HX15E_ZQ_CFG			0x50074BE4
#define MT41J256M8HX15E_RATIO			0x40
#define MT41J256M8HX15E_INVERT_CLKOUT		0x1
#define MT41J256M8HX15E_RD_DQS			0x3B
#define MT41J256M8HX15E_WR_DQS			0x85
#define MT41J256M8HX15E_PHY_WR_DATA		0xC1
#define MT41J256M8HX15E_PHY_FIFO_WE		0x100
#define MT41J256M8HX15E_IOCTRL_VALUE		0x18B

/*Micron MT41K256M16HA125E */
#define MT41K256M16HA125E_IOCTRL_VALUE		0x18B
#define MT41K256M16HA125E_EMIF_SDCFG		0x61C05332
#define MT41K256M16HA125E_EMIF_SDREF		0x00000618
#define MT41K256M16HA125E_ZQ_CFG		0x50074BE4
#define MT41K256M16HA125E_EMIF_TIM1		0x0AAAD4DB
#define MT41K256M16HA125E_EMIF_TIM2		0x266B7FDA
#define MT41K256M16HA125E_EMIF_TIM3		0x501F867F
#define MT41K256M16HA125E_EMIF_READ_LATENCY	0x100007
#define MT41K256M16HA125E_RD_DQS		0x39
#define MT41K256M16HA125E_WR_DQS		0x4a
#define MT41K256M16HA125E_PHY_FIFO_WE		0x9d
#define MT41K256M16HA125E_PHY_WR_DATA		0x85
/*************************************
#define MT41K256M16HA125E_RD_DQS		0x39
#define MT41K256M16HA125E_PHY_FIFO_WE		0x9c
#define MT41K256M16HA125E_WR_DQS		0x49
#define MT41K256M16HA125E_PHY_WR_DATA		0x85
*************************************/
#define MT41K256M16HA125E_RATIO			0x80
#define MT41K256M16HA125E_INVERT_CLKOUT		0x0
#define MT41K256M16HA125E_DLL_LOCK_DIFF		0x1

#if 0
/* Micron MT41K256M16HA-125E */
#define MT41K256M16HA125E_EMIF_READ_LATENCY	0x100007
#define MT41K256M16HA125E_EMIF_TIM1		0x0AAAD4DB
#define MT41K256M16HA125E_EMIF_TIM2		0x266B7FDA
#define MT41K256M16HA125E_EMIF_TIM3		0x501F867F
#define MT41K256M16HA125E_EMIF_SDCFG		0x61C05332
#define MT41K256M16HA125E_EMIF_SDREF		0xC30
#define MT41K256M16HA125E_ZQ_CFG		0x50074BE4
#define MT41K256M16HA125E_RATIO			0x80
#define MT41K256M16HA125E_INVERT_CLKOUT		0x0
#define MT41K256M16HA125E_RD_DQS		0x38
#define MT41K256M16HA125E_WR_DQS		0x44
#define MT41K256M16HA125E_PHY_WR_DATA		0x7D
#define MT41K256M16HA125E_PHY_FIFO_WE		0x94
#define MT41K256M16HA125E_IOCTRL_VALUE		0x18B
#endif
/* Micron MT41J512M8RH-125 on EVM v1.5 */
#define MT41J512M8RH125_EMIF_READ_LATENCY	0x100006
#define MT41J512M8RH125_EMIF_TIM1		0x0888A39B
#define MT41J512M8RH125_EMIF_TIM2		0x26517FDA
#define MT41J512M8RH125_EMIF_TIM3		0x501F84EF
#define MT41J512M8RH125_EMIF_SDCFG		0x61C04BB2
#define MT41J512M8RH125_EMIF_SDREF		0x0000093B
#define MT41J512M8RH125_ZQ_CFG			0x50074BE4
#define MT41J512M8RH125_RATIO			0x80
#define MT41J512M8RH125_INVERT_CLKOUT		0x0
#define MT41J512M8RH125_RD_DQS			0x3B
#define MT41J512M8RH125_WR_DQS			0x3C
#define MT41J512M8RH125_PHY_FIFO_WE		0xA5
#define MT41J512M8RH125_PHY_WR_DATA		0x74
#define MT41J512M8RH125_IOCTRL_VALUE		0x18B

/* Samsung K4B2G1646E-BIH9 */
#define K4B2G1646EBIH9_EMIF_READ_LATENCY	0x100007
#define K4B2G1646EBIH9_EMIF_TIM1		0x0AAAE51B
#define K4B2G1646EBIH9_EMIF_TIM2		0x2A1D7FDA
#define K4B2G1646EBIH9_EMIF_TIM3		0x501F83FF
#define K4B2G1646EBIH9_EMIF_SDCFG		0x61C052B2
#define K4B2G1646EBIH9_EMIF_SDREF		0x00000C30
#define K4B2G1646EBIH9_ZQ_CFG			0x50074BE4
#define K4B2G1646EBIH9_RATIO			0x80
#define K4B2G1646EBIH9_INVERT_CLKOUT		0x0
#define K4B2G1646EBIH9_RD_DQS			0x35
#define K4B2G1646EBIH9_WR_DQS			0x3A
#define K4B2G1646EBIH9_PHY_FIFO_WE		0x97
#define K4B2G1646EBIH9_PHY_WR_DATA		0x76
#define K4B2G1646EBIH9_IOCTRL_VALUE		0x18B

#define  LPDDR2_ADDRCTRL_IOCTRL_VALUE   0x294
#define  LPDDR2_ADDRCTRL_WD0_IOCTRL_VALUE 0x00000000
#define  LPDDR2_ADDRCTRL_WD1_IOCTRL_VALUE 0x00000000
#define  LPDDR2_DATA0_IOCTRL_VALUE   0x20000294
#define  LPDDR2_DATA1_IOCTRL_VALUE   0x20000294
#define  LPDDR2_DATA2_IOCTRL_VALUE   0x20000294
#define  LPDDR2_DATA3_IOCTRL_VALUE   0x20000294

#define  DDR3_ADDRCTRL_WD0_IOCTRL_VALUE 0x00000000
#define  DDR3_ADDRCTRL_WD1_IOCTRL_VALUE 0x00000000
#define  DDR3_ADDRCTRL_IOCTRL_VALUE   0x84
#define  DDR3_DATA0_IOCTRL_VALUE   0x84
#define  DDR3_DATA1_IOCTRL_VALUE   0x84
#define  DDR3_DATA2_IOCTRL_VALUE   0x84
#define  DDR3_DATA3_IOCTRL_VALUE   0x84


/* Micron MT41K128M16HA-125E*/
#define MT41K128M16HA125_IOCTRL_VALUE           0x18B
#define MT41K128M16HA125_EMIF_SDCFG             0x61C052b2
#define MT41K128M16HA125_EMIF_SDREF             0x00000618
#define MT41K128M16HA125_ZQ_CFG                 0x50074BE4
#define MT41K128M16HA125_EMIF_TIM1              0x0AAAD4DB
#define MT41K128M16HA125_EMIF_TIM2              0x266B7FDA
#define MT41K128M16HA125_EMIF_TIM3              0x501F867F
#define MT41K128M16HA125_EMIF_READ_LATENCY      0x100007
#define MT41K128M16HA125_RD_DQS                 0x39
#define MT41K128M16HA125_WR_DQS                 0x47
#define MT41K128M16HA125_PHY_FIFO_WE            0x9b
#define MT41K128M16HA125_PHY_WR_DATA            0x81
/*************************************
#define MT41K128M16HA125_RD_DQS                 0x39
#define MT41K128M16HA125_PHY_FIFO_WE            0x9c
#define MT41K128M16HA125_WR_DQS                 0x49
#define MT41K128M16HA125_PHY_WR_DATA            0x85
*************************************/
#define MT41K128M16HA125_RATIO                  0x80
#define MT41K128M16HA125_INVERT_CLKOUT          0x0
#define MT41K128M16HA125_DLL_LOCK_DIFF          0x1

/* Micron MT41K256M16HA-125E - Yimin*/
#if 0
#define MT41K256M16HA125E_EMIF_READ_LATENCY     0x100006
#define MT41K256M16HA125E_EMIF_SDREF            0x00000C30 //400(MHz)*tREFI=400(MHz)*7.8(us)=3120=0xC30
#define MT41K256M16HA125E_EMIF_TIM1             0x0AAAE51B
#define MT41K256M16HA125E_EMIF_TIM2             0x266B7FDA
#define MT41K256M16HA125E_EMIF_TIM3             0x501F867F
#define MT41K256M16HA125E_ZQ_CFG                0x50074BE4
#define MT41K256M16HA125E_DLL_LOCK_DIFF         0x1
#define MT41K256M16HA125E_RATIO                 0x40
#define MT41K256M16HA125E_INVERT_CLKOUT         0x0
#define MT41K256M16HA125E_RD_DQS                0x3C
#define MT41K256M16HA125E_WR_DQS                0x45
#define MT41K256M16HA125E_PHY_WR_DATA           0x83
#define MT41K256M16HA125E_PHY_FIFO_WE           0x9E
#define MT41K256M16HA125E_IOCTRL_VALUE          0x18B
#define MT41K256M16HA125E_EMIF_SDCFG            0x61C04BB2
#endif

#define K4B4G1646D_IOCTRL_VALUE		0x1EF
#define K4B4G1646D_EMIF_SDCFG		0x61C45332
#define K4B4G1646D_EMIF_SDREF		0x00000618 /* 400MHz*3.9us=1560(0x618) */
#define K4B4G1646D_ZQ_CFG		0x50074BE4
#define K4B4G1646D_EMIF_TIM1		0x0AAAE51B
#define K4B4G1646D_EMIF_TIM2		0x2A6B7FDA
#define K4B4G1646D_EMIF_TIM3		0x501F867F
#define K4B4G1646D_EMIF_READ_LATENCY	0x100007 //RD_Latency = (CL + 2) - 1, CL=6
#define K4B4G1646D_RD_DQS		0x3A
#define K4B4G1646D_WR_DQS		0x36
#define K4B4G1646D_PHY_FIFO_WE		0x9B
#define K4B4G1646D_PHY_WR_DATA		0x70
#define K4B4G1646D_INVERT_CLKOUT	0x0
#define K4B4G1646D_RATIO		0x80
#define K4B4G1646D_DLL_LOCK_DIFF	0x1

/**
 * Configure DMM
 */
void config_dmm(const struct dmm_lisa_map_regs *regs);

/**
 * Configure SDRAM
 */
void config_sdram(const struct emif_regs *regs, int nr);
void config_sdram_emif4d5(const struct emif_regs *regs, int nr);

/**
 * Set SDRAM timings
 */
void set_sdram_timings(const struct emif_regs *regs, int nr);

/**
 * Configure DDR PHY
 */
void config_ddr_phy(const struct emif_regs *regs, int nr);

struct ddr_cmd_regs {
	unsigned int resv0[7];
	unsigned int cm0csratio;	/* offset 0x01C */
	unsigned int resv1[3];
	unsigned int cm0iclkout;	/* offset 0x02C */
	unsigned int resv2[8];
	unsigned int cm1csratio;	/* offset 0x050 */
	unsigned int resv3[3];
	unsigned int cm1iclkout;	/* offset 0x060 */
	unsigned int resv4[8];
	unsigned int cm2csratio;	/* offset 0x084 */
	unsigned int resv5[3];
	unsigned int cm2iclkout;	/* offset 0x094 */
	unsigned int resv6[3];
};

struct ddr_data_regs {
	unsigned int dt0rdsratio0;	/* offset 0x0C8 */
	unsigned int resv1[4];
	unsigned int dt0wdsratio0;	/* offset 0x0DC */
	unsigned int resv2[4];
	unsigned int dt0wiratio0;	/* offset 0x0F0 */
	unsigned int resv3;
	unsigned int dt0wimode0;	/* offset 0x0F8 */
	unsigned int dt0giratio0;	/* offset 0x0FC */
	unsigned int resv4;
	unsigned int dt0gimode0;	/* offset 0x104 */
	unsigned int dt0fwsratio0;	/* offset 0x108 */
	unsigned int resv5[4];
	unsigned int dt0dqoffset;	/* offset 0x11C */
	unsigned int dt0wrsratio0;	/* offset 0x120 */
	unsigned int resv6[4];
	unsigned int dt0rdelays0;	/* offset 0x134 */
	unsigned int dt0dldiff0;	/* offset 0x138 */
	unsigned int resv7[12];
};

/**
 * This structure represents the DDR registers on AM33XX devices.
 * We make use of DDR_PHY_BASE_ADDR2 to address the DATA1 registers that
 * correspond to DATA1 registers defined here.
 */
struct ddr_regs {
	unsigned int resv0[3];
	unsigned int cm0config;		/* offset 0x00C */
	unsigned int cm0configclk;	/* offset 0x010 */
	unsigned int resv1[2];
	unsigned int cm0csratio;	/* offset 0x01C */
	unsigned int resv2[3];
	unsigned int cm0iclkout;	/* offset 0x02C */
	unsigned int resv3[4];
	unsigned int cm1config;		/* offset 0x040 */
	unsigned int cm1configclk;	/* offset 0x044 */
	unsigned int resv4[2];
	unsigned int cm1csratio;	/* offset 0x050 */
	unsigned int resv5[3];
	unsigned int cm1iclkout;	/* offset 0x060 */
	unsigned int resv6[4];
	unsigned int cm2config;		/* offset 0x074 */
	unsigned int cm2configclk;	/* offset 0x078 */
	unsigned int resv7[2];
	unsigned int cm2csratio;	/* offset 0x084 */
	unsigned int resv8[3];
	unsigned int cm2iclkout;	/* offset 0x094 */
	unsigned int resv9[12];
	unsigned int dt0rdsratio0;	/* offset 0x0C8 */
	unsigned int resv10[4];
	unsigned int dt0wdsratio0;	/* offset 0x0DC */
	unsigned int resv11[4];
	unsigned int dt0wiratio0;	/* offset 0x0F0 */
	unsigned int resv12;
	unsigned int dt0wimode0;	/* offset 0x0F8 */
	unsigned int dt0giratio0;	/* offset 0x0FC */
	unsigned int resv13;
	unsigned int dt0gimode0;	/* offset 0x104 */
	unsigned int dt0fwsratio0;	/* offset 0x108 */
	unsigned int resv14[4];
	unsigned int dt0dqoffset;	/* offset 0x11C */
	unsigned int dt0wrsratio0;	/* offset 0x120 */
	unsigned int resv15[4];
	unsigned int dt0rdelays0;	/* offset 0x134 */
	unsigned int dt0dldiff0;	/* offset 0x138 */
};

/**
 * Encapsulates DDR CMD control registers.
 */
struct cmd_control {
	unsigned long cmd0csratio;
	unsigned long cmd0csforce;
	unsigned long cmd0csdelay;
	unsigned long cmd0iclkout;
	unsigned long cmd1csratio;
	unsigned long cmd1csforce;
	unsigned long cmd1csdelay;
	unsigned long cmd1iclkout;
	unsigned long cmd2csratio;
	unsigned long cmd2csforce;
	unsigned long cmd2csdelay;
	unsigned long cmd2iclkout;
};

/**
 * Encapsulates DDR DATA registers.
 */
struct ddr_data {
	unsigned long datardsratio0;
	unsigned long datawdsratio0;
	unsigned long datawiratio0;
	unsigned long datagiratio0;
	unsigned long datafwsratio0;
	unsigned long datawrsratio0;
};

/**
 * Configure DDR CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd, int nr);

/**
 * Configure DDR DATA registers
 */
void config_ddr_data(const struct ddr_data *data, int nr);

/**
 * This structure represents the DDR io control on AM33XX devices.
 */
struct ddr_cmdtctrl {
	unsigned int cm0ioctl;
	unsigned int cm1ioctl;
	unsigned int cm2ioctl;
	unsigned int resv2[12];
	unsigned int dt0ioctl;
	unsigned int dt1ioctl;
	unsigned int dt2ioctrl;
	unsigned int dt3ioctrl;
	unsigned int resv3[4];
	unsigned int emif_sdram_config_ext;
};

struct ctrl_ioregs {
	unsigned int cm0ioctl;
	unsigned int cm1ioctl;
	unsigned int cm2ioctl;
	unsigned int dt0ioctl;
	unsigned int dt1ioctl;
	unsigned int dt2ioctrl;
	unsigned int dt3ioctrl;
	unsigned int emif_sdram_config_ext;
};

/**
 * Configure DDR io control registers
 */
void config_io_ctrl(const struct ctrl_ioregs *ioregs);

struct ddr_ctrl {
	unsigned int ddrioctrl;
	unsigned int resv1[325];
	unsigned int ddrckectrl;
};

void config_ddr(unsigned int pll, const struct ctrl_ioregs *ioregs,
		const struct ddr_data *data, const struct cmd_control *ctrl,
		const struct emif_regs *regs, int nr);
void emif_get_ext_phy_ctrl_const_regs(const u32 **regs, u32 *size);

#endif  /* _DDR_DEFS_H */
