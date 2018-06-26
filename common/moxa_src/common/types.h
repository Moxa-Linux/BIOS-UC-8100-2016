/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    types.h

	Basic typedef used in MOXA IXP BIOS.
    
    2010-03-25	Chin-Fu Yang.			Create it.
*/

#ifndef __TYPES_H
#define __TYPES_H


typedef volatile unsigned char	vuchar;
typedef volatile unsigned long	vulong;
typedef volatile unsigned short	vushort;
#define VPlong    		*(vulong *)
#define VPshort  		*(vushort *)
#define VPchar   		*(vuchar *)


typedef unsigned long long 	UINT64;
typedef long long		INT64;
typedef	unsigned int		UINT32;
typedef	int			INT32;
typedef	unsigned short		UINT16;
typedef	short			INT16;
typedef unsigned char		UINT8;
typedef char			INT8;
typedef unsigned char		BOOL;

/* ASCII symbol define */

#define CR			0x0D
#define LF			0x0A
#define BS			0x08
#define ESC 			27

/* These defines are in a common coding practices header file */

#ifndef	FALSE
#define FALSE			0
#endif
#ifndef	TRUE
#define TRUE			1
#endif
#ifndef	NULL
#define	NULL			0
#endif


#ifndef ON
#define ON			1
#endif

#ifndef OFF
#define	OFF			0
#endif


#ifndef ENABLE
#define ENABLE			1
#endif

#ifndef DISABLE
#define	DISABLE			0
#endif

#ifndef PARITY_NONE
#define PARITY_NONE		0
#endif

#ifndef PARITY_ODD
#define PARITY_ODD		1
#endif

#ifndef PARITY_EVEN
#define PARITY_EVEN		2
#endif

#ifndef PARITY_MARK
#define PARITY_MARK		3
#endif

#ifndef PARITY_SPACE
#define PARITY_SPACE		4
#endif


#ifndef HIGH
#define HIGH			1
#endif


#ifndef LOW
#define LOW 			0
#endif


#ifndef MASK
#define MASK 			1
#endif


#ifndef UNMASK
#define UNMASK 			0
#endif


#ifndef NONE
#define NONE 			0
#endif


#ifndef WRITE
#define WRITE 			1
#endif


#ifndef READ
#define READ 			0
#endif


#ifndef SET
#define SET 			1
#endif


#ifndef GET
#define GET 			0
#endif


//
// Generic masks.
//

#define BIT0			0x00000001
#define BIT1			0x00000002
#define BIT2			0x00000004
#define BIT3			0x00000008
#define BIT4			0x00000010
#define BIT5			0x00000020
#define BIT6			0x00000040
#define BIT7			0x00000080
#define BIT8			0x00000100
#define BIT9			0x00000200
#define BIT10			0x00000400
#define BIT11			0x00000800
#define BIT12			0x00001000
#define BIT13			0x00002000
#define BIT14			0x00004000
#define BIT15			0x00008000
#define BIT16			0x00010000
#define BIT17			0x00020000
#define BIT18			0x00040000
#define BIT19			0x00080000
#define BIT20			0x00100000
#define BIT21			0x00200000
#define BIT22			0x00400000
#define BIT23			0x00800000
#define BIT24			0x01000000
#define BIT25			0x02000000
#define BIT26			0x04000000
#define BIT27			0x08000000
#define BIT28			0x10000000
#define BIT29			0x20000000
#define BIT30			0x40000000
#define BIT31			0x80000000

#define sBIT0			0x0001
#define sBIT1			0x0002
#define sBIT2			0x0004
#define sBIT3			0x0008
#define sBIT4			0x0010
#define sBIT5			0x0020
#define sBIT6			0x0040
#define sBIT7			0x0080
#define sBIT8			0x0100
#define sBIT9			0x0200
#define sBIT10			0x0400
#define sBIT11			0x0800
#define sBIT12			0x1000
#define sBIT13			0x2000
#define sBIT14			0x4000
#define sBIT15			0x8000

#define cBIT0			0x01
#define cBIT1			0x02
#define cBIT2			0x04
#define cBIT3			0x08
#define cBIT4			0x10
#define cBIT5			0x20
#define cBIT6			0x40
#define cBIT7			0x80

typedef union {
	volatile unsigned long long bit64;
	struct {
		volatile unsigned long word0;
		volatile unsigned long word1;
	} word;
	struct {
		volatile unsigned short short0;
		volatile unsigned short short1;
		volatile unsigned short short2;
		volatile unsigned short short3;
	} half_word;
	struct {
		volatile unsigned char byte0;
		volatile unsigned char byte1;
		volatile unsigned char byte2;
		volatile unsigned char byte3;
		volatile unsigned char byte4;
		volatile unsigned char byte5;
		volatile unsigned char byte6;
		volatile unsigned char byte7;
	} byte;	
} struct_bit64 , *struct_bit64_p;

#endif
