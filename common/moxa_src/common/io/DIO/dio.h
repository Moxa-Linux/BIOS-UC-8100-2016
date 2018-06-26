/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

//R00   06/29/2015   DON   Add DIO MP-Test.

#ifndef __LED_H__
#define __LED_H__

#include "types.h"

void do_dio_test(void);
void DIO_MPTEST(UINT8 led);

#endif /* __LED_H__ */

