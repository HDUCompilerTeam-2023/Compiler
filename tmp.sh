#########################################################################
# File Name: tmp.sh
# Author: Deng Zengyong
# mail: 2595650269@qq.com
# Created Time: 2023年08月30日 星期三 01时49分58秒
#########################################################################
#!/bin/bash
mkdir raspi_gcc
make clean
make test optim=-O0 -j4
make test_asm -j4
make test_exe > raspi_gcc/gcc_o0_time
make clean
make test optim=-O1 -j4
make test_asm -j4
make test_exe > raspi_gcc/gcc_o1_time
make clean
make test optim=-O2 -j4
make test_asm -j4
make test_exe > raspi_gcc/gcc_o2_time
make clean
make test optim=-O3 -j4
make test_asm -j4
make test_exe > raspi_gcc/gcc_o3_time