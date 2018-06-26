/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    console.h

    Headers for Console of MOXAUART.
 */

//R00   10/03/2013   DON   First initial
 
#ifndef _CONSOLE_H
#define _CONSOLE_H

/*------------------------------------------------------ Macro / Define -----------------------------*/
#define CR		0x0D
#define NL		0x0A
#define BR		0x08

/*------------------------------------------------------ Structure ----------------------------------*/

/*------------------------------------------------------ Extern / Function Declaration -----------------*/
unsigned char Getch (void);
int	Kbhit (void);
void Write (char *buf,int len);
void WriteHex (unsigned short value);
void WriteByte (unsigned char ch);
int con_get_string (char *str, int len);
void PrintStr (char *str);
void PrintHex (unsigned long value);
void PrintChar (char value);
unsigned char check_if_press_character (unsigned char check_word);
int getch (void);

#endif

