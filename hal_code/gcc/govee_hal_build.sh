#!/bin/sh
#copy ota file sdk
prj_path=$(pwd)

frq_sdk_path_line=$(cat Makefile| grep ^SDK_ROOT\ )
echo $frq_sdk_path_line
frq_sdk_path_=${frq_sdk_path_line#*=}
frq_sdk_path_=`eval echo $frq_sdk_path_|tr -d '\\r'`
frq_sdk_path=`eval echo $frq_sdk_path_`

echo $frq_sdk_path
pack_file_path="../ota_pack"

ota_file_path_c_tail="/components/ble/profiles/ble_ota/ota.c"
pack_file_path_c_tail="/ota.c"

ota_file_path_h_tail="/components/ble/profiles/ble_ota/ota.h"
pack_file_path_h_tail="/ota.h"
hal_build_path="/../hal/gcc"

cp -r $pack_file_path$pack_file_path_c_tail $frq_sdk_path$ota_file_path_c_tail
cp -r $pack_file_path$pack_file_path_h_tail $frq_sdk_path$ota_file_path_h_tail

#build frq
#build frq
if [ $# != 1 ];then
    echo "make"
    make
else
    echo "make clean"
    make clean && make
fi
