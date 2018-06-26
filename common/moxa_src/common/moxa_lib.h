/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    moxa_lib.h

	Functions of common used library.
    
    2013-10-03	Donald Hsu				Created it.
*/
#ifndef _LIB_H
#define _LIB_H

#define _U	0x01	/* upper */
#define _L	0x02	/* lower */
#define _D	0x04	/* digit */
#define _C	0x08	/* cntrl */
#define _P	0x10	/* punct */
#define _S	0x20	/* white space (space/lf/tab) */
#define _X	0x40	/* hex digit */
#define _SP	0x80	/* hard space (0x20) */

enum {
	HALT_MODE1 = 0,
	HALT_MODE2,
	HALT_MODE3
};


//R03 - Start
struct _EEPROMFORMAT
{
	unsigned char   szMagic[2];
	unsigned char   nFormatVersionMajor;
	unsigned char   nFormatVersionMinor;
	unsigned char   szModelName[80];
	unsigned char   szSerialNumber[20];
	unsigned char   nHwMajorVersion1;
	unsigned char   nHwMinorVersion1;
	unsigned char   nHwMajorVersion2;
	unsigned char   nHwMinorVersion2;
	unsigned char   nHwMajorVersion3;
	unsigned char   nHwMinorVersion3;
	unsigned char   nHwMajorVersion4;
	unsigned char   nHwMinorVersion4;
	unsigned char   nHwMajorVersion5;
	unsigned char   nHwMinorVersion5;
	unsigned char   szCusUID[20];
	unsigned char   szReserved[14];
	unsigned char   szBIOSRegion[100];
	unsigned char   szSWRegion[100];
	unsigned char   szAPRegion[100];
	unsigned char   szCustomer[64];
}__attribute__((packed));
typedef struct _EEPROMFORMAT *PEEPROMFORMAT;
//R03 - End


extern unsigned char _ctype[];
#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)	((__ismask(c)&(_C)) != 0)
#define isdigit(c)	((__ismask(c)&(_D)) != 0)
#define isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)	((__ismask(c)&(_L)) != 0)
#define isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)	((__ismask(c)&(_P)) != 0)
#define isspace(c)	((__ismask(c)&(_S)) != 0)
#define isupper(c)	((__ismask(c)&(_U)) != 0)
#define isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)
#define is_digit(c)	((c) >= '0' && (c) <= '9')
#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

static inline unsigned char __tolower(unsigned char c)
{
	if (isupper(c)){
		c -= 'A'-'a';
	}
	return c;
}

static inline unsigned char __toupper(unsigned char c)
{
	if (islower(c)){
		c -= 'a'-'A';
	}
	return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

/*------------------------------------------------------ Extern / Function Declaration -----------------*/
unsigned long inet_addr(char *ptr);
int get_digit(char *buf, int *len);
int skip_atoi(const char **s);
int	StrLength(const char *ptr);
unsigned long bios_lib_simple_strtoul(const char *cp,char **endp,unsigned int base);
long bios_lib_simple_strtol(const char *cp,char **endp,unsigned int base);
int atoi_simple(char *string);
int atoi(const char *str) ;

unsigned char	Atob(char *buf);
unsigned short	Atow(char *buf);
unsigned long	Atoh(char *buf);
unsigned short	Atos(char *buf);
unsigned long	Atol(char *buf);

int _is_hex(char c);
int _from_hex(char c);
char _tolower(char c);
int	IsDigit(char *buf);
int	IsHex(char *buf);
//R01 unsigned short Htons(unsigned short val);
//R01 unsigned long Htonl(unsigned long val);
void Strcpy(unsigned char *dbuf,unsigned char *sbuf);
unsigned long Inw(unsigned long addr);
void Outw(unsigned long addr,unsigned long data);
unsigned short Inhw(unsigned long addr);
void Outhw(unsigned long addr,unsigned short data);
unsigned char Inb(unsigned long addr);
void Outb(unsigned long addr,unsigned char data);

void * mem_cpy(void * dest,const void *src,long count);
void * mem_set(void * s, char c, long count);

void ReadyLed(unsigned int Gpio_pin,unsigned char status); //R02
void flag_set_ok(int model);
void ShowLED(unsigned char led);
void halt(unsigned int value, int mode);
void Wait_SW_Button(void);
void SetBeeperStatus(unsigned char val);
int GetJpStatus(unsigned char Gpio_pin);
unsigned long randkey(void);
unsigned int diag_get_env_conf_u32(const char *name);
unsigned long diag_get_env_conf_u32_mem(const char *name);
void diag_set_env_conf_u32(const char *name, unsigned int value);
void diag_print_env_conf(const char *name);
int bios_parse_num(char *s, unsigned int *val, char **es, char *delim);
#endif
