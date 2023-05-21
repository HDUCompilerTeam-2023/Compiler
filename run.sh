#!/bin/bash
# 为方便测试一个.sy 文件的正确性
#./run.sh *.sy: 运行我们的文件
#./run.sh *.sy arm: 用交叉编译器运行
LIB_DIR=lib
LIB_FILE=sysy
INCLUDE=sylib.h

cp "$1" ./tmp.c || exit 1

if [ $# -eq 1 ]; then
    ./build/SysYParser-debug ./tmp.c -o ./tmp.s || exit 1
elif [[ $# -eq 2 && $2 = "arm1" ]]; then
    arm-linux-gnueabihf-gcc -march=armv7-a -mfpu=neon-vfpv4 -S -O1 ./tmp.c -include $LIB_DIR/sylib.h -o ./tmp.s || exit 1
elif [[ $# -eq 2 && $2 = "arm2" ]]; then
    arm-linux-gnueabihf-gcc -march=armv7-a -mfpu=neon-vfpv4 -S -O2 ./tmp.c -include $LIB_DIR/sylib.h -o ./tmp.s || exit 1
elif [[ $# -eq 2 && $2 = "arm3" ]]; then
    arm-linux-gnueabihf-gcc -march=armv7-a -mfpu=neon-vfpv4 -S -O3 ./tmp.c -include $LIB_DIR/sylib.h -o ./tmp.s || exit 1
else
    echo "wrong arg"
    exit 1
fi
cat tmp.s
arm-linux-gnueabihf-gcc -static ./tmp.s -L$LIB_DIR -l$LIB_FILE -o ./tmp.out || exit 1
qemu-arm ./tmp.out || exit 1
echo "exit: $?"