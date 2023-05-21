#!/bin/bash
# 运行可执行文件输出结果
test_if_dead_circle(){
    # 超时时间（以秒为单位）
    TIMEOUT=120

    # 执行命令，并记录开始时间
    if [ $# -eq 1 ]; then
        qemu-arm $1 &
    elif [ $# -eq 2 ]; then
        cat $1 | qemu-arm $2 &
    else
        echo "wrong args"
        exit 1
    fi

    PID=$!
    START_TIME=$(date +%s)

    # 等待命令执行完成或超时
    while kill -0 $PID >/dev/null 2>&1; do
        CURRENT_TIME=$(date +%s)
        ELAPSED_TIME=$((CURRENT_TIME-START_TIME))
        if [ $ELAPSED_TIME -gt $TIMEOUT ]; then
            kill -9 $PID >/dev/null 2>&1
            return 1
        fi
    done
    return 0
}

test_file=$1
run_out_file="${test_file%.*}.output"
input_file="${test_file%.*}.in"

if [ -f $input_file ]; then
    test_if_dead_circle $input_file $test_file
else
    test_if_dead_circle $test_file
fi

if_dead=$?
if [ $if_dead -eq 1 ]; then
    echo "  dead circle "
    exit 1
else
    if [ -f $input_file ]; then
        qemu-arm "$test_file" < "$input_file" > "$run_out_file"
    else
        qemu-arm "$test_file" > "$run_out_file"
    fi
    output=$?
    sed -i -e '$a\' "$run_out_file"
    echo "$output" >> "$run_out_file"
    if [ -s "$run_error_file" ]; then
        line_count=$(wc -l < "$run_error_file")
        first_line=$(sed 's/^\(.\{7\}\).*/\1/' "$run_error_file")
        if [[ "$line_count" -le 2 && "$first_line" = "TOTAL: " ]]; then
            mv "$run_error_file" "${run_error_file%.*}".tm
            exit 0
        fi
        exit 1
    fi
fi

exit 0