#!/bin/bash

cd ../src

OUT_DIR=../

###### ST40 #######
# TOOLCHAIN_DIR=/opt/softs/compilers/st/st40/LINUX_4.2.4.jan0711/STLinux-2.3/devkit/sh4_uclibc
# GCC=${TOOLCHAIN_DIR}/bin/sh4-linux-uclibc-gcc
# G=${TOOLCHAIN_DIR}/bin/sh4-linux-uclibc-g++
# QTLIB=../../../WEBKITQT48/cisco-4.8-cmc-openmha-qt4.8.4.3-wk116000.10_b_WEBKITQT48_MAIN_MHA_Int/ST40_CC14_0_1/release/lib/
# QTINC=../../../WEBKITQT48/cisco-4.8-cmc-openmha-qt4.8.4.3-wk116000.10_b_WEBKITQT48_MAIN_MHA_Int/ST40_CC14_0_1/release/include/
# OUT_DIR=../../ST40_CC14_0_1/

###### BCM #######
# TOOLCHAIN_DIR=/opt/softs/compilers/others/Broadcom/BCM_LNX_1_0_0
# GCC=${TOOLCHAIN_DIR}/bin/mipsel-linux-uclibc-gcc
# G=${TOOLCHAIN_DIR}/bin/mipsel-linux-uclibc-g++
# QTLIB=../../../WEBKITQT48/cisco-4.8-cmc-openmha-qt4.8.4.2-wk116000.9_d_WEBKITQT48_MAIN_MHA_Int/BCM_LNX_1_0_2/release/lib/
# QTINC=../../../WEBKITQT48/cisco-4.8-cmc-openmha-qt4.8.4.2-wk116000.9_d_WEBKITQT48_MAIN_MHA_Int/BCM_LNX_1_0_2/release/include/
# OUT_DIR=../../BCM_LNX_1_0_2/

OUT_LIB_DIR=${OUT_DIR}/lib

rm -rf ${OUT_LIB_DIR}
mkdir -p ${OUT_DIR}
mkdir -p ${OUT_LIB_DIR}
make clean
make
 # CC=${GCC} CXX=${G} QT_LIB_PATH=${QTLIB} QT_INC_PATH=${QTINC} OUTLIBDIR=${OUT_LIB_DIR}
# cp ../mhap.mk ${OUT_DIR}/mhap.mk
cp -r ../inc /opt/webdriver/linux2_2/release
cp -r ../lib /opt/webdriver/linux2_2/release
