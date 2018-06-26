/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <memalign.h>
#include <usb.h>
#include <asm/gpio.h>
#include <environment.h>
#include <bios.h> 
#include <i2c.h> 
#include <pca953x.h> 
#include <model.h>
#include <cli.h>
#include <rf.h>

int do_pca9535_set_direction(unsigned int i2c_bus, unsigned int addr, unsigned int gpio_bit, unsigned int gpio_dir)
{
	int ret = 0;

	i2c_set_bus_num(i2c_bus);

	ret = pca953x_set_dir (addr, (1 << gpio_bit), (gpio_dir << gpio_bit));

	i2c_set_bus_num(0);

        return ret;
}

int do_pca9535_set_value(unsigned int i2c_bus, unsigned int addr, unsigned int gpio_bit, unsigned int gpio_val)
{
	int ret = 0;

	i2c_set_bus_num(i2c_bus);
	
	ret = pca953x_set_val (addr, (1 << gpio_bit), (gpio_val << gpio_bit));
	
	i2c_set_bus_num(0);
	
	return ret;
}

int do_pca9535_get_value(unsigned int i2c_bus, unsigned int addr)
{
	int value = 0;
	
	i2c_set_bus_num(i2c_bus);

	value = pca953x_get_val(addr);

	i2c_set_bus_num(0);

	return value;
}

void do_pca9535_init(void)
{

	i2c_set_bus_num(2);

	//pca953x_set_dir(PCA953X_ADDR2, 0xffff, 0xf880);
	pca953x_set_dir(PCA953X_ADDR2, 0x0002, 0x0000);
	pca953x_set_val(PCA953X_ADDR2, 0x0002, 0x0002); //mpcie power keep high as default
	//pca953x_set_val(PCA953X_ADDR2, 0x077f, 0x002d); //mpcie power keep low as default
	i2c_set_bus_num(0);
}

