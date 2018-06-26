/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    moxa_lib.c

	Functions of common used library.
    
*/

#include <common.h>
#include <command.h>
#include <asm/gpio.h>
#include <bios.h>
#include <model.h>
#include "../cmd_bios.h"
#include "types.h"
#include "moxa_lib.h"
#include <i2c.h>
#include <pca953x.h>
#include <environment.h>

unsigned long inet_addr (char *ptr)
{
	int				len,i;
	unsigned short	ch;
	unsigned long	val;
	char			buffer[10];

	for (i = 0, len = 0, val = 0, ch = 0; i < 4; i++) {
		len = get_digit (ptr, &len);
		if ((ptr[len] != '.') && (i < 3)) {
			//printf("inet_addr() return none\r\n");
			return 0;
		}

		memcpy (buffer, ptr, len+1);
		buffer[len] = 0;

		if ((ch = (unsigned short) Atos (buffer)) >= 0xff) {
			//printf("inet_addr() return none\r\n");
			return 0;
		}
		val |= (unsigned char)ch << (i*8);
		ptr += (len+1);
	}
	return val;
}

int get_digit (char *buf, int *len)
{
	int i = 0;
	char ch;

	*len = 0;
	while (1) {
	    ch = *buf++;
	    if ((ch < '0') || (ch >'9')) {
			break;
		}
	    *len = (*len) * 10 + ch - '0';
	    i++;
	}
	return (i);
}

int skip_atoi (const char **s)
{
	int i = 0;

	while (is_digit (**s)) {
		i = i*10 + *((*s)++) - '0';
	}
	return i;
}

int	StrLength (const char *ptr)
{
	int	i = 0;

	while (*ptr++ != 0) {
	    i++;
	}
	return(i);
}

unsigned long bios_lib_simple_strtoul (const char *cp, char **endp, unsigned int base)
{
	unsigned long result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit (cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit (*cp) &&
		(value = isdigit (*cp) ? *cp-'0' : (islower (*cp) ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp) {
		*endp = (char *)cp;
	}
	return result;
}

long bios_lib_simple_strtol (const char *cp, char **endp, unsigned int base)
{
	if (*cp=='-') {
		return -(bios_lib_simple_strtoul (cp+1,endp,base));
	}
	return bios_lib_simple_strtoul (cp,endp,base);
}

int atoi_simple (char *string)
{
	int res = 0;
	while (*string>='0' && *string <='9') {
		res *= 10;
		res += *string-'0';
		string++;
	}

	return res;
}

int atoi (const char *str) 
{
	return ((int) (bios_lib_simple_strtoul (str, (char **) NULL, 10)));
}

/*
 *	ASCII string to byte binary value  (unsigned char)
 */
unsigned char Atob (char *buf)
{
	unsigned char value = 0;
	char ch;
	int	i = 0;

	for (;;) {
	    ch = *buf++;
	    if ((ch >= '0') && (ch <= '9')) {
			value = (value << 4) | (ch - '0');
		} else if ((ch >= 'a') && (ch <= 'f')) {
			value = (value << 4) | (ch - 'a' + 10);
		} else if ((ch >= 'A') && (ch <= 'F')) {
			value = (value << 4) | (ch - 'A' + 10);
		} else {
			break;
		}
	    i++;
	    if (i >= 2) {
			break;
		}
	}
	return (value);
}
/*
 *	ASCII string to word binary value  (unsigned short)
 */
unsigned short Atow (char *buf)
{
	unsigned short value = 0;
	char ch;
	int	i = 0;

	for (;;) {
	    ch = *buf++;
	    if ((ch >= '0') && (ch <= '9')){
			value = (value << 4) | (ch - '0');
		} else if ((ch >= 'a') && (ch <= 'f')) {
			value = (value << 4) | (ch - 'a' + 10);
		} else if ((ch >= 'A') && (ch <= 'F')) {
			value = (value << 4) | (ch - 'A' + 10);
		} else {
			break;
		}
	    i++;
	    if (i >= 4) {
			break;
		}
	}
	return (value);
}

unsigned long Atoh(char *buf)
{
	unsigned long value = 0;
	char ch;
	int	i=0;

	for (;;) {
	    ch = *buf++;
	    if ((ch >= '0') && (ch <= '9')) {
			value = (value << 4) | (ch - '0');
		} else if ((ch >= 'a') && (ch <= 'f')) {
			value = (value << 4) | (ch - 'a' + 10);
		} else if ((ch >= 'A') && (ch <= 'F')) {
			value = (value << 4) | (ch - 'A' + 10);
		} else {
			break;
		}
	    i++;
	    if (i >= 8) {
			break;
		}
	}
	return (value);
}

/*
 *	ASCII string to unsigned short	(unsigned short)
 */
unsigned short Atos (char *buf)
{
	unsigned short value = 0;
	char ch;

	for (;;) {
	    ch = *buf++;
	    if ((ch >= '0') && (ch <= '9')) {
			value = value * 10 + ch - '0';
		} else {
			break;
		}
	}
	return(value);
}

/*
 *	ASCII string to unsigned short	(unsigned short)
 */
unsigned long Atol (char *buf)
{
	unsigned long value = 0;
	char ch;

	for (;;) {
	    ch = *buf++;
	    if ((ch >= '0') && (ch <= '9')) {
			value = value * 10 + ch - '0';
		} else {
			break;
		}
	}
	return (value);
}

int _is_hex (char c)
{
	return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||            
            ((c >= 'a') && (c <= 'f')));
}

int _from_hex (char c) 
{
	int ret = 0;

	if ((c >= '0') && (c <= '9')) {
		ret = (c - '0');
	} else if ((c >= 'a') && (c <= 'f')) {
		ret = (c - 'a' + 0x0a);
	} else if ((c >= 'A') && (c <= 'F')) {
		ret = (c - 'A' + 0x0A);
	}
	return ret;
}

char _tolower (char c)
{
	if ((c >= 'A') && (c <= 'Z')) {
		c = (c - 'A') + 'a';
	}
	return c;
}

int	IsDigit (char *buf)
{
	char ch;

	for (;;) {
		ch = *buf++;
		if (ch == 0) {
			break;
		}
		if ((ch >= '0') && (ch <= '9')) {
			continue;
		}
		return (0);
	}
	return (1);
}

int	IsHex (char *buf)
{
	char ch;

	for (;;) {
		ch = *buf++;
		if (ch == 0) {
			break;
		}
		if(_is_hex (ch) == 0) {
			return (0);
		}
	}
	return (1);
}

void Strcpy(unsigned char *dbuf, unsigned char *sbuf)
{
	int	i = 0;

	while (sbuf[i] != 0) {
	    dbuf[i] = sbuf[i];
	    i++;
	}
	dbuf[i] = 0;
	return;
}

unsigned long Inw(unsigned long addr)
{
    return *(volatile unsigned long *)(addr);
}

void Outw(unsigned long addr,unsigned long data)
{
    *(volatile unsigned long *)(addr)=data;
}

unsigned short Inhw(unsigned long addr)
{
    return *(volatile unsigned short *)(addr);
}

void Outhw(unsigned long addr,unsigned short data)
{
    *(volatile unsigned short *)(addr)=data;
}

unsigned char Inb(unsigned long addr)
{
    return *(volatile unsigned char *)(addr);
}

void Outb(unsigned long addr,unsigned char data)
{
    *(volatile unsigned char *)(addr) = data;
}

void * mem_cpy(void * dest, const void *src, long count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--){
		*tmp++ = *s++;
	}
	return dest;
}

void * mem_set(void * s, char c, long count)
{
	char *xs = (char *) s;

	while (count--){
		*xs++ = c;
	}
	return s;
}

void ShowLED(unsigned char led)
{
	led &= 0x1f;


	gpio_direction_output (PIO_LED_MP_4, (((led & 0x10) == 0x10) ? 1 : 0));
	gpio_direction_output (PIO_LED_MP_3, (((led & 0x8) == 0x8) ? 1 : 0));
	gpio_direction_output (PIO_LED_MP_2, (((led & 0x4) == 0x4) ? 1 : 0));
	gpio_direction_output (PIO_LED_MP_1, (((led & 0x2) == 0x2) ? 1 : 0));
	gpio_direction_output (PIO_LED_MP_0, (((led & 0x1) == 0x1) ? 1 : 0));
}

void flag_set_ok(int model)
{
	int rdyled = PIO_LED_SD;

	while(1) {
		ReadyLed(rdyled, 1);
		SetBeeperStatus(1);
		udelay (500000);
		ReadyLed(rdyled, 0);
		SetBeeperStatus(0);
		udelay (500000);
	}

}

//R03 void ReadyLed(unsigned char Gpio_pin,unsigned char status)
void ReadyLed(unsigned int Gpio_pin,unsigned char status) //R03
{
        gpio_direction_output (Gpio_pin, status);
}

/*
 *	subroutine Halt()
 *	function : Halt System and Show Debug LEN for Error Number
 *	input  : Show value on Debug LED
 *	return : None
 */

void halt(unsigned int value, int mode)
{

	switch (mode){
	case HALT_MODE1:
		for (;;) {
			ShowLED(0);
			udelay(1000*1000);
			ShowLED(value);
			udelay(1000*1000);
		}
		break;
	case HALT_MODE2:
		SetBeeperStatus(1);
			
		for (;;) {
			ShowLED(0);
			udelay(1000*1000);
			ShowLED(value);
			udelay(1000*1000);
		}
		break;
	case HALT_MODE3:
		for (;;) {
			ShowLED(0);
			SetBeeperStatus(1);
			udelay(1000*1000);
			ShowLED(value);
			SetBeeperStatus(0);
			udelay(1000*1000);
		}
		break;
	default:
		for (;;) {
			ShowLED(0);
			udelay(1000*1000);
			ShowLED(value);
			udelay(1000*1000);
		}
		break;
	}


}

void Wait_SW_Button(void)
{
	unsigned long count=0;

	while(1) {
		if (GetJpStatus(PIO_SW_BUTTON) == 0) {
			while(1){
				if (GetJpStatus(PIO_SW_BUTTON) == 1) {
					udelay(1000);
					break;
				}
			}
			udelay(1000);
			break;
		}

		udelay(1000);
        	if(count < 1000){
			SetBeeperStatus(1);
			//ReadyLed (PIO_LED_SD, 1); //R02A
			count++;
        	} else if ((count >= 1000) && (count < 2000)) {
			SetBeeperStatus(0);
			//ReadyLed (PIO_LED_SD, 0);	//R02A
			count++;
        	} else {
			count = 0;
		}

	}

	SetBeeperStatus(0);

	ReadyLed (PIO_LED_SD, 0);
	udelay (1000000);
}

void SetBeeperStatus(unsigned char val)
{
	gpio_direction_output (PIO_BUZZER, val);
}

unsigned long randkey(void)
{
	return get_timer(0);
}

int GetJpStatus(unsigned char Gpio_pin)
{
	gpio_direction_input (Gpio_pin);
	return gpio_get_value (Gpio_pin);
}

unsigned int diag_get_env_conf_u32(const char *name)
{
	char *s;
	unsigned int value=0;

	s = getenv (name);
	bios_parse_num (s, &value, 0, 0);

	return value;
}

unsigned long diag_get_env_conf_u32_mem(const char *name)
{
	char sbuf[32];
	unsigned int value = 0;

	sprintf(sbuf,"%s",getenv(name));
	bios_parse_num(sbuf, &value, 0, 0);
	return value;
}

void diag_set_env_conf_u32(const char *name, unsigned int value)
{
	char cmd_msg[64];
	int ret = DIAG_OK;

	sprintf(cmd_msg, "setenv %s %d", name, value);

	if ((ret = run_command(cmd_msg, 0)) != 0){
		printf("Set %s Fail [%d] \r\n",name, ret);
		return;
	}

	run_command("saveenv", 0);
	return;
}

void diag_print_env_conf(const char *name)
{
	int j, k, nxt;

	for (j = 0; env_get_char(j) != '\0'; j = nxt + 1) {
		for (nxt = j; env_get_char(nxt) != '\0'; ++nxt){
  			;
		}

		k = envmatch((uchar *)name, j);

		if (k < 0) {
			continue;
		}

		while (k < nxt){
			putc(env_get_char(k++));
		}

		break;
	}
}

int bios_parse_num(char *s, unsigned int *val, char **es, char *delim)
{
	int first = 1;
	int radix = 10;
	char c;
	unsigned long result = 0;
	int digit;

	while (*s == ' '){
		s++;
	}

	while (*s) {
		if (first && (s[0] == '0') && (_tolower(s[1]) == 'x')) {
			radix = 16;
			s += 2;
		}

		first = DIAG_FAIL;
		c = *s++;

		if (_is_hex(c) && ((digit = _from_hex(c)) < radix)) {
			result = (result * radix) + digit;
		}else{

			if (delim != (char *)0) {
                                // See if this character is one of the delimiters
				char *dp = delim;

				while (*dp && (c != *dp)) {
					dp++;
				}

				if(*dp)
					break;
			}

			return DIAG_FAIL;
		}
	}

	*val = result;
	
	if (es != (char **)0) {
		*es = s;
	}

	return DIAG_OK;
}
