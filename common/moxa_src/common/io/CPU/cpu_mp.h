#ifndef CPU_MP_H
#define CPU_MP_H

#define EFUSE_SMA_REG			0x07FC

#define ZCZ_300Mhz			0x1FEF
#define ZCZ_600Mhz			0x1FAF
#define ZCZ_720Mhz			0x1F2F
#define ZCZ_800Mhz			0x1E2F
#define ZCZ_1Ghz			0x1C2F

int Check_CPU_Type(u8 show);
void CPU_MPTEST(u8 led, int model);

#endif /*CPU_MP_H*/

