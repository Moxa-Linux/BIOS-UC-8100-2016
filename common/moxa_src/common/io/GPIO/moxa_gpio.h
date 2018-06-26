/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

int do_pca9535_set_direction(unsigned int i2c_bus, unsigned int addr, unsigned int gpio_bit, unsigned int gpio_dir);
int do_pca9535_set_value(unsigned int i2c_bus, unsigned int addr, unsigned int gpio_bit, unsigned int gpio_dir);
int do_pca9535_get_value(unsigned int i2c_bus, unsigned int addr);
void do_pca9535_init(void);

