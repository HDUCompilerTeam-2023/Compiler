#!/bin/bash
# 和正确输出比较
TEST_DIR=$1
files=$(find $TEST_DIR -name "*.out")
for file in $files; do
    run_output_file="${file%.*}.output"
    if [ ! -f $run_output_file ]; then
        continue
    fi
    diff -bB $file $run_output_file >/dev/null
    ifcorrect=$?
    if [ $ifcorrect -eq 0 ]; then
        echo -e "* $file"
    else
        echo -e "x $file"
    fi
done