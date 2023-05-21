#!/bin/bash
# 和正确输出比较
TEST_DIR=$1
files=$(find $TEST_DIR -name "*.out")
correct_num=0
output_error_num=0
compiler_error_num=0
for file in $files; do
    run_output_file="${file%.*}.output"
    if [ ! -f $run_output_file ]; then
        compiler_error_num=$(expr $compiler_error_num + 1)
        echo -e " \033[37m   $file not have run output.\033[0m"
        continue
    fi
    diff -bB $file $run_output_file >/dev/null
    ifcorrect=$?
    if [ $ifcorrect -eq 0 ]; then
        correct_num=$(expr $correct_num + 1)
        echo -e " \033[34m success $file == $run_output_file\033[0m"
    else
        output_error_num=$(expr $output_error_num + 1)
        echo -e " \033[31m failed $file != $run_output_file\033[0m"
    fi
done

echo "success num : $correct_num"
echo "output error num : $output_error_num"
echo "compiler error num : $compiler_error_num"