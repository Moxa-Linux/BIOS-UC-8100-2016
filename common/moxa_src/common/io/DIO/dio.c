/*  Copyright (C) MOXA Inc. All rights reserved.

    This software is distributed under the terms of the
    MOXA License.  See the file COPYING-MOXA for details.
*/

#include <common.h>
#include <command.h>
#include <bios.h>
#include <model.h>
#include <i2c.h>
#include <pca953x.h>
#include <asm/gpio.h>
#include "../cmd_bios.h"
#include "moxa_console.h"
#include "moxa_lib.h"
#include "types.h"
#include "dio.h"

/*
 *	LED test ;
*/
#ifdef DIO_MP
void do_dio_test(void)
{

	unsigned int out;
        int i, err = 0;
        int retry = 1;


        printf ("CPU DI DO test start\n");

retry1:
        out = 1;
        err = 0;
        for (i = 0;i <= PIO_CPU_DO4 - PIO_CPU_DO1; i++){
                gpio_direction_output (PIO_CPU_DO1 + i, out) ;

                if (gpio_get_value (PIO_CPU_DI1 + i ) != out){
                        err++ ;
                }
        }
        
        if(err && retry){
                retry --;
                printf("test failed!.");
                printf("Please check DI/O test cable.\n ");
                printf("If you're ready. Please Press SW button.\n ");
                printf("If you're ready. Please Press SW button to retry.\n ");
                Wait_SW_Button();
                goto retry1;
        }else if(err){
                goto done;
        }

retry2:
        retry = 1;
        out = 0;
        err = 0;

        for (i = 0;i <= PIO_CPU_DO4 - PIO_CPU_DO1; i++){
                gpio_direction_output (PIO_CPU_DO1 + i, out) ;

                if (gpio_get_value (PIO_CPU_DI1 + i ) != out){
                        err++ ;
                }
        }

        if(err && retry){
                retry --;
                printf("test failed!.");
                printf("Please check DI/O test cable.\n ");
                printf("If you're ready. Please Press SW button to retry.\n ");
                Wait_SW_Button();
                goto retry2;
        }else if(err){
                goto done;
        }

done:
        if(err){
                printf("CPU DI DO test failed!!\n");
                halt(DIO_MP, HALT_MODE1);
        }else{
                printf ("\r\nDIO testing ** OK ** \r\n");
        }
}

void DIO_MPTEST (UINT8 led)
{
	
	printf("\r\n");

	printf("[0x%04x]DIO Test...\r\n", led);

        do_dio_test();	


}
#endif /* DIO_MP */

