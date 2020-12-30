#!/bin/sh

#  fw2bin.sh
#  AirPortOpenBSD
#
#  Created by Zhong-Mac on 2020/7/18.
#  Copyright © 2020 Zhong-Mac. All rights reserved.

target_path="${PROJECT_DIR}/AirPortOpenBSD/firmware/"
fw_files=$(find "${PROJECT_DIR}/AirPortOpenBSD/sources/firmware" ! -name ".*" -maxdepth 1 -type f | tr " " "\?")

for fw in ${fw_files}; do
    fw_file_name=`basename $(echo ${fw} | tr " " "-") `
    
    fw_var_name=${fw_file_name//./_}
    fw_var_name=${fw_var_name//-/_}
    target_file="${target_path}${fw_var_name}.c"
    rm -rf $target_file
    echo "//\n//  ${fw_var_name}.c\n//  AirPortOpenBSD\n\n//  Created by Zhong-Mac on 2020/7/18.\n//  Copyright © 2020 Zhong-Mac. All rights reserved." >$target_file
    echo "\n#include \"firmware.h\"">>$target_file
    
    echo "">>$target_file
    echo "const unsigned char ${fw_var_name}[] = {">>$target_file
    xxd -i <$fw >>$target_file
    echo "};">>$target_file
    echo "">>$target_file
    echo "const unsigned int ${fw_var_name}_size = sizeof(${fw_var_name});">>$target_file
done


target_file="${target_path}firmwarevar.h"

rm -rf $target_file

echo "">>$target_file
echo "//\n//  firmwarevar.h\n//  AirPortOpenBSD\n\n//  Created by Zhong-Mac on 2020/7/18.\n//  Copyright © 2020 Zhong-Mac. All rights reserved." >$target_file
echo "">>$target_file
echo "#ifndef firmwarevar_h\n#define firmwarevar_h">>$target_file
echo "">>$target_file

for fw in ${fw_files}; do
    fw_file_name=`basename $(echo ${fw} | tr " " "-") `
    
    fw_var_name=${fw_file_name//./_}
    fw_var_name=${fw_var_name//-/_}
    
    echo "">>$target_file
    echo "extern const unsigned char ${fw_var_name}[];">>$target_file
    echo "extern const unsigned int ${fw_var_name}_size;">>$target_file
done

echo "">>$target_file
echo "#endif">>$target_file


target_file="${target_path}firmware.c"

rm -rf $target_file

echo "">>$target_file
echo "//\n//  firmware.c\n//  AirPortOpenBSD\n\n//  Created by Zhong-Mac on 2020/7/18.\n//  Copyright © 2020 Zhong-Mac. All rights reserved." >$target_file
echo "\n#include \"firmware.h\"">>$target_file


echo "">>$target_file
echo "const struct firmware firmwares[] = {">>$target_file
i=0;
for fw in $fw_files; do
    fw_file_name=`basename $(echo ${fw} | tr " " "-") `
    fw_var_name=${fw_file_name//./_}
    fw_var_name=${fw_var_name//-/_}
    echo "    {FIRMWARE(\"$fw_file_name\", $fw_var_name, ${fw_var_name}_size)},">>$target_file
    let i+=1
done
echo "};">>$target_file
echo "const int firmwares_total = $i;">>$target_file
