/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/
/*
    predefine_table.h

    secure boot

    2014-03-31	Sun Lee				Created it.
*/

#ifndef _PREDEFINE_TABLE_H_
#define _PREDEFINE_TABLE_H_
                                     //0     , 1      , 2      , 3      , 4      , 5      , 6      , 7      , 8      , 9	  , 10     , 11     , 12     , 13     , 14      , 15   
unsigned long Start_Offset[16] = {0x5DC00, 0x5EB00, 0x60900, 0x73500, 0x86100, 0xAB900, 0xD1100, 0xF6900, 0x25800, 0x4B000, 0x96000, 0xBB800, 0xE1000, 0xD7A00, 0xCE400, 0x79E00};
//0x55 -> UP, pxAA -> DOWN
unsigned char Direction_Offset[3] = {0x00, 0x55, 0xAA};
unsigned char Jump_Offset[16] = {0x04, 0x02, 0x08, 0x06, 0x0A, 0x08, 0x06, 0x04, 0x02, 0x02, 0x08, 0x06, 0x04, 0x06, 0x08, 0x0A};

#endif  //_PREDEFINE_TABLE

