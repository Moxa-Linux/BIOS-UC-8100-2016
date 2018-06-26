/*
 * (C) Copyright 2016
 */

/*
 * OLED Display Utilities
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <spi.h>

/*-----------------------------------------------------------------------
 * Definitions
 *----------------------------------------------------------------------*/

/*
 * Values from last command.
 */

int mptest_oled(int bus, int cs, int mode)
{
	struct spi_slave *slave;
	int ret = 0;
	u16 params = 0;
	int cnt;

	slave = spi_setup_slave(bus, cs, 1000000, mode);
	
	if (!slave) {
		printf("Invalid device %d:%d\n", bus, cs);
		return -EINVAL;
	}

	if (bus == 1)
		spi_set_wordlen(slave, 10);

	ret = spi_claim_bus(slave);

	if (ret)
		goto done;
	
	if (bus == 1){
		// init OLED
		params = 0x38;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
		
		params = 0x06;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);

		params = 0x01;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
	
		params = 0x0c;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
	
		params = 0x1f;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
		//init end
		//command - start
		params = 0x01;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
		//command - end
		//show screen - start
		params = 0x80;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);

		params = 0x40;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);

		params = 0x2ff;
		ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
			
		for(cnt = 0; cnt < 100; cnt++){
		
			params = 0x80 | cnt;
			ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
			
			params = 0x40;
			ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
		
			params = 0x2ff;
			ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
			
			params = 0x80 | cnt;
			ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);

			params = 0x41;
			ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
		
			params = 0x2ff;
			ret = spi_xfer(slave, 10, &params, NULL, SPI_XFER_ONCE);
		}

	}
	
done:
	spi_release_bus(slave);

	return ret;
}
