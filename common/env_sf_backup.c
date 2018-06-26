/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>

#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS	0
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS	0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ	1000000
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#endif
#ifdef CONFIG_ENV_OFFSET_REDUND
//static ulong env_offset1		= CONFIG_ENV_OFFSET;
static ulong env_new_offset1	= CONFIG_ENV_OFFSET_REDUND;

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0
#endif /* CONFIG_ENV_OFFSET_REDUND */

#define ENV_BAK_OFFSET		0x700000
DECLARE_GLOBAL_DATA_PTR;

static struct spi_flash *env_flash1;

int clear_backup_env(void)
{
	char	*saved_buffer = NULL;
	int	ret;

	if (!env_flash1) {
		env_flash1 = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash1) {
			set_default_env("!spi_flash_probe() failed");
			return 1;
		}
	}

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash1, ENV_BAK_OFFSET,
				0x20000);
	if (ret)
		goto done;

	puts("done\n");

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}


int save_backup_env(void)
{
	env_t	env_new;
	char	*saved_buffer = NULL;
	u32	saved_size, saved_offset, sector = 1;
	int	ret;

	if (!env_flash1) {
		env_flash1 = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash1) {
			printf("!spi_flash_probe() failed\n");
			return 1;
		}
	}

	ret = env_export(&env_new);
	if (ret)
		return ret;

	env_new.flags	= ACTIVE_FLAG;
#if 0
	if (gd->env_valid == 1) {
		env_new_offset1 = ENV_BAK_OFFSET;
		env_offset1 = CONFIG_ENV_OFFSET;
	} else {
		env_new_offset1 = CONFIG_ENV_OFFSET;
		env_offset1 = CONFIG_ENV_OFFSET_REDUND;
	}
#endif
	env_new_offset1 = ENV_BAK_OFFSET;

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = env_new_offset1 + CONFIG_ENV_SIZE;
		saved_buffer = memalign(ARCH_DMA_MINALIGN, saved_size);
		if (!saved_buffer) {
			ret = 1;
			goto done;
		}
		ret = spi_flash_read(env_flash1, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash1, env_new_offset1,
				sector * CONFIG_ENV_SECT_SIZE);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");

	ret = spi_flash_write(env_flash1, env_new_offset1,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

#if 0
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash1, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = spi_flash_write(env_flash1, env_offset1 + offsetof(env_t, flags),
				sizeof(env_new.flags), &flag);
	if (ret)
		goto done;
#endif
	puts("done\n");

//	gd->env_valid = gd->env_valid == 2 ? 1 : 2;

//	printf("Valid environment: %d\n", (int)gd->env_valid);

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

int backup_env_relocate(int import)
{
	int ret = 1;
	int crc_ok = 0;
	env_t *tmp_env = NULL;
	env_t *ep = NULL;
	int env_valid = 0;

	tmp_env = (env_t *)memalign(ARCH_DMA_MINALIGN,
			CONFIG_ENV_SIZE);
	if (!tmp_env) {
		printf("!malloc() failed\n");
		goto out;
	}

	env_flash1 = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);

	if (!env_flash1) {
		printf("!spi_flash_probe() failed\n");
		goto out;
	}

	ret = spi_flash_read(env_flash1, ENV_BAK_OFFSET,
				CONFIG_ENV_SIZE, tmp_env);
	if (ret) {
		printf("!spi_flash_read() failed\n");
		goto err_read;
	}

	if (crc32(0, tmp_env->data, ENV_SIZE) == tmp_env->crc)
		crc_ok = 1;

	if (!crc_ok) {
		printf("!bad CRC\n");
		ret = 1;
		env_valid = 0;
		goto err_read;
	} else if (crc_ok) {
		env_valid = 1;
	} else if (tmp_env->flags == ACTIVE_FLAG) {
		env_valid = 1;
	} else if (tmp_env->flags == 0xFF) {
		env_valid = 1;
	} else {
		/*
		 * this differs from code in env_flash1.c, but I think a sane
		 * default path is desirable.
		 */
		env_valid = 0;
		ret = 1;
		goto err_read;
	}

	if (import && env_valid) {
		ep = tmp_env;
		ret = env_import((char *)ep, 0);
		printf("import\n");
		if (!ret) {
			error("Cannot import environment: errno = %d\n", errno);
			printf("env_import failed\n");
		}
	}

	ret = 0;

err_read:
	spi_flash_free(env_flash1);
	env_flash1 = NULL;
out:
	free(tmp_env);
	return ret;
}

int check_env(void)
{
	int ret = 0;

	ret |= backup_env_relocate(1);

	ret |= setenv("ENV_IS_FAILED", "0");

	ret |= run_command("saveenv", 0);

        return ret;
}


int user_env_recovery(void)
{
	int ret = 0;

	ret |= backup_env_relocate(1);

	ret |= setenv("env_fail", "0");

	ret |= run_command("saveenv", 0);

        return ret;
}
