#########################################################################
# File Name: tmp.sh
# Author: Deng Zengyong
# mail: 2595650269@qq.com
# Created Time: 2023年08月30日 星期三 01时49分58秒
#########################################################################
#!/bin/bash
mkdir qemu_arm_gcc
make clean
make test optim=-O0 -j8
make test_asm -j8
make test_exe > qemu_arm_gcc/gcc_o0_time
make clean
make test optim=-O1 -j8
make test_asm -j8
make test_exe > qemu_arm_gcc/gcc_o1_time
make clean
make test optim=-O2 -j8
make test_asm -j8
make test_exe > qemu_arm_gcc/gcc_o2_time
make clean
make test optim=-O3 -j8
make test_asm -j8
make test_exe > qemu_arm_gcc/gcc_o3_time
