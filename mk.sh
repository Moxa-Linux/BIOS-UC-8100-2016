#!/bin/bash

make clean 

rm -rf common/moxa_bios

mkdir -p common/moxa_bios/

file=(`find common/moxa_src/common/ -type f`)

for ((i=0; i<${#file[@]}; i++)); do
      cp -a ${file[$i]} common/moxa_bios
done

cp common/cmd_bios.h common/moxa_bios
cp common/moxa_src/common/model.h include/

make -o ./am335x CROSS_COMPILE=arm-linux-gnueabihf- ARCH=arm  -j30
chmod 777 MLO

/usr/local/fsl_ls1043_1703_toolchain/sysroots/x86_64-fslsdk-linux/usr/bin/python2.7 build_spi.py MLO.byteswap u-boot.img .

