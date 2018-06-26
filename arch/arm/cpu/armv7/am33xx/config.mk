#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#
# R00  05/16/2016    SHA   Build a SPI ROM .

ifdef CONFIG_SPL_BUILD
ALL-y	+= MLO
ALL-y	+= MLO.byteswap #R00
else
ALL-y	+= u-boot.img
endif
