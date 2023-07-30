#!/bin/bash
TIMEOUT="timeout 150s"

test_file=$1
run_out_file="${test_file%.*}.output"
time_file="${test_file%.*}.time"
input_file="${test_file%.*}.in"

if [ -f $input_file ]; then
    cat $input_file | $TIMEOUT qemu-arm $test_file > $run_out_file 2> $time_file &
else
    $TIMEOUT qemu-arm $test_file > $run_out_file 2> $time_file &
fi

wait $!
output=$?
sed -i -e '$a\' "$run_out_file"
echo "$output" >> "$run_out_file"

exit 0
