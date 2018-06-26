#include <common.h>
#include <types.h>
#include "model.h"
#include "moxa_lib.h"
#include "sys_info.h"
#include "cpu_mp.h"
#include "moxa_boot.h"

int Check_CPU_Type(u8 show)
{
	unsigned int val;
	int freq = 0;
	int ret = 0;	

	val = (VPlong(0x44E10000 + EFUSE_SMA_REG) & 0x00001FFF);

	switch (val) {
		case ZCZ_300Mhz:
			ret = 1;
			freq = 300;
			break;
		case ZCZ_600Mhz:
			ret = 2;
			freq = 600;
			break;
		case ZCZ_720Mhz:
			ret = 3;
			freq = 720;
			break;
		case ZCZ_800Mhz:
			ret = 4;
			freq = 800;
			break;
		case ZCZ_1Ghz:
			ret = 5;
			freq = 1000;
			break;
		default :
			ret = 0;
			freq = 0;
		        break;
	
	}
	
	if (show)
		printf("CPU TYPE: %dMHz", freq);

	return ret;
}

void CPU_MPTEST(u8 led, int model){
        int cpu_type;
        u8 flag_8131;
        u8 flag_8132;
        u8 flag_8162;
        u8 flag_8112;

        printf("\r\n");

	printf("[0x%04x]CPU Test...\r\n",led);		
	//printf("wait sw button to pass test...\r\n");		

	cpu_type = Check_CPU_Type(0);
        
        flag_8131 = (cpu_type == 1) && (model == UC8131); 
        flag_8132 = (cpu_type == 1) && (model == UC8132); 
        flag_8162 = (cpu_type == 2) && (model == UC8162); 
        flag_8112 = (cpu_type == 5) && (model == UC8112); 
        //printf("cpu_type 0x%x, model %d, flag_8112 %d\n\n", cpu_type, model, flag_8112);

        if (flag_8131 || flag_8132 || flag_8162 || flag_8112){
                printf(" The CPU is correct, pass to next test!\n");
        } else {
                printf(" Incorrected CPU type, please check the mounted CPU!\n");
                halt(CPU_MP, HALT_MODE2); 
        }
        
        return ;
}
