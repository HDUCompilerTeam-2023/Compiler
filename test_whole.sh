#########################################################################
# File Name: test_whole.sh
# Author: Deng Zengyong
# mail: 2595650269@qq.com
# Created Time: 2023年08月30日 星期三 01时31分42秒
#########################################################################
#!/bin/bash
if [ $# -ne 2 ]; then
	echo "usage: ./test_whole.sh test_dir test_log_dir"
	exit 1
fi
rm -rf $2
mkdir $2
make clean
make 
echo "start test_compiler (in $2/test_compiler)"
make test -j8 TEST_DIR=$1 > "$2/test_compiler"
echo "end test_compiler (in $2/test_compiler)"
echo "start test_asembler (in $2/test_asembler)"
make test_asm -j8 TEST_DIR=$1 > "$2/test_asembler"
echo "end test_asembler (in $2/test_asembler)"
echo "start test_exe (in $2/test_exe)"
make test_exe TEST_DIR=$1 > "$2/test_exe"
echo "end test_exe (in $2/test_exe)"
echo "start test_output (in $2/test_output)"
make test_output TEST_DIR=$1 > "$2/test_output"
echo "end test_output (in $2/test_output)"
echo "start generate result (in $2/result.md)"
python3 show_result.py "qemu_arm_gcc" $2 "$2/result.md" "local_test of qemu_arm"
echo "generate result success (in $2/result.md)!"


