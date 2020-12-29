#!/bin/sh

#copy ota file sdk
prj_path=$(pwd)

frq_sdk_path_line=$(cat Makefile| grep ^SDK_ROOT)
frq_sdk_path_=${frq_sdk_path_line#*=}
frq_sdk_path_=`eval echo $frq_sdk_path_|tr -d '\\r'`
frq_sdk_path=`eval echo $frq_sdk_path_`
pack_file_path="/../hal/ota_pack"

ota_file_path_c_tail="/components/ble/profiles/ble_ota/ota.c"
pack_file_path_c_tail="/../hal/ota_pack/ota.c"

ota_file_path_h_tail="/components/ble/profiles/ble_ota/ota.h"
pack_file_path_h_tail="/../hal/ota_pack/ota.h"

cp -r $frq_sdk_path$pack_file_path_c_tail $frq_sdk_path$ota_file_path_c_tail
cp -r $frq_sdk_path$pack_file_path_h_tail $frq_sdk_path$ota_file_path_h_tail

#build frq
#build frq
if [ $# != 1 ];then
    echo "make"
    make
else
    echo "make clean && make"
    make clean&&make
fi

prj_file_os_head="./build/"
prj_file_os_tail='.bin'

prj_name_find_line=$(cat Makefile| grep ^TARGETS)
prj_file_name_=${prj_name_find_line#*=}
prj_file_name=`eval echo $prj_file_name_`
prj_file_name=`eval echo $prj_file_name|tr -d '\\r'`
prj_file_os_name=$prj_file_os_head$prj_file_name$prj_file_os_tail

# add crc
if [ ! -d "$prj_file_os_name" ]; then
    cd $frq_sdk_path$pack_file_path  && gcc mk_RF801_crc.c -o mk_RF801_crc
    cp mk_RF801_crc $prj_path && rm -rf mk_RF801_crc && cd $prj_path
    ./mk_RF801_crc $prj_file_os_name
    hexdump -n 4 $prj_file_os_name 
    rm -rf mk_RF801_crc
fi
