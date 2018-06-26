/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    console.c

    Routines for Console of AM335X.
    
*/

//R00   10/03/2013   DON   First initial

#include <common.h>
#include "moxa_console.h"
/*
 *	subroutine Getch
 *	function :
 *	    get a byte of date from console port
 *	input :
 *	    none
 *	return :
 *	    data read from console port
 */
unsigned char Getch (void)
{
	int ch;

	while (1) {
		ch = getc ();

		if (ch == 0x1000) {
			printf("getc[0x%x]", ch);
			continue;
		}

    		return (unsigned char)ch;
	}
}

/*
 *	subroutine Kbhit
 *	function :
 *	    return if data available on console rx buffer
 *	input :
 *	    none
 *	return :
 *	    0	--> no data available
 *	    1	--> Yes, data is available
 */
int Kbhit (void)
{
	return serial_tstc ();
}

/*
 *	subroutine Write
 *	function :
 *	    write data to console
 *	input :
 *	    buf     --> data pointer
 *	    len     --> length to be write
 *	return :
 *	    none
 */
void Write (char *buf, int len)
{
	int i;
	
	for (i = 0; i < len; i++) {
		serial_putc (*buf++);
	}
	return;
}

/*
 *	subroutine WriteHex
 *	function :
 *	    convert a ushort data to string format and write to console
 *	input :
 *	    value   --> data to be converted
 *	return :
 *	    none
 */
void WriteHex (unsigned short value)
{
	unsigned char		ch;
	static char	buf[6];

	ch = value & 0xffff;
	buf[0] = ch >> 4;
	if (buf[0] < 10) {
	    buf[0] += '0';
	} else {
	    buf[0] += ('A' - 10);
	}
	buf[1] = ch & 0x0f;
	if (buf[1] < 10) {
	    buf[1] += '0';
	} else {
	    buf[1] += ('A' - 10);
	}
	ch = value >> 8;
	buf[2] = ch >> 4;
	if (buf[2] < 10) {
	    buf[2] += '0';
	} else {
	    buf[2] += ('A' - 10);
	}
	buf[3] = ch & 0x0f;
	if (buf[3] < 10) {
	    buf[3] += '0';
	} else {
	    buf[3] += ('A' - 10);
	}
	buf[4] = ' ';
	buf[5] = ' ';
	Write (buf, 6);
	return;
}

/*
 *	subroutine WriteByte
 *	function :
 *	    convert a unsigned char data to string format and write to console
 *	input :
 *	    ch	    --> data to be converted
 *	return :
 *	    none
 */
void WriteByte (unsigned char ch)
{
	static char	buf[2];

	buf[0] = ch >> 4;
	if (buf[0] < 10) {
	    buf[0] += '0';
	} else {
	    buf[0] += ('A' - 10);
	}
	buf[1] = ch & 0x0f;
	if (buf[1] < 10) {
	    buf[1] += '0';
	} else {
	    buf[1] += ('A' - 10);
	}
	Write (buf,2);
	return;
}

static char *BreakString="\b \b";
int con_get_string (char *str, int len)
{
	short	ret=0;
	char	ch;
	
	while (ret < len) {
	    ch = Getch ();
	    if (ch == BR) {
			if ( ret > 0 ) {
		    	printf(BreakString);
		    	ret--;
			}
			continue;
	    } else if (ch == 27) {
			return (-1);
		}
	    Write ((char *)&ch,1);
	    if ( ch == CR || ch == NL ){
			break;
		}
	    str[ret++] = ch;
	}
	str[ret] = 0;
	return (ret);
}

void PrintStr (char *str)
{
	Write (str, strlen (str));
	return;
}

static char digits[]="0123456789ABCDEF";
void PrintHex (unsigned long value)
{
	int i, idx;

	for (i = 7; i >= 0; i--) {
		idx = value >> (i*4);
		serial_putc (digits[idx&0xf]);
	}
	return;
}

void PrintChar (char value)
{
	int i, idx;

	for (i = 1; i >= 0; i--) {
		idx = value >> (i*4);
		serial_putc (digits[idx&0xf]);
	}
	return;
}

/**	\brief
 *
 *	Check The Input Character From Console
 *
 * 	\param[in]	check_word : The Character Which Need To Be Checked
 * 	\retval		0 : Not Match, 1 : Match
 *
 */
unsigned char check_if_press_character (unsigned char check_word)
{
	int ch;

	if (Kbhit ()) {
		ch = Getch ();
		if (ch == check_word) {
			return 1;
		}
	}
	return 0;
}

int getch (void)
{
	if (Kbhit ())
	{
		return Getch ();
	}
	return 0x1000;
}

