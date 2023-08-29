from datetime import datetime
from datetime import timedelta
import re

def parse_time_string(time_string):
    parts = time_string.split('-')
    hours = int(parts[0].replace('H', ''))
    minutes = int(parts[1].replace('M', ''))
    seconds = int(parts[2].replace('S', ''))
    microseconds = int(parts[3].replace('us', ''))
    return timedelta(hours=hours, minutes=minutes, seconds=seconds, microseconds=microseconds)

def time_string(time):
    s = time.seconds
    h, s = divmod(s, 3600)
    m, s = divmod(s, 60)
    return '{}H-{}M-{}S-{}us'.format(h, m, s, time.microseconds)

class judge_object:
    def __init__(self, files, compiler_errors, asembler_errors, run_errors, run_times, output_errors, output_success):
        self.files = files
        self.compiler_errors = compiler_errors
        self.asembler_errors = asembler_errors
        self.run_errors = run_errors
        self.run_times = run_times
        self.output_errors = output_errors
        self.output_success = output_success
        self.averarge_run_time = self.get_average_run_time()
    def get_average_run_time(self):
        num = 0
        whole_time = timedelta(0, 0, 0, 0, 0, 0, 0)
        for _, v in self.run_times.items():
            num += 1
            whole_time += parse_time_string(v)
        if num == 0 :
            return whole_time
        return whole_time / num
def check_compiler(compiler_out):
    wrong_files = []
    files = []
    with open(compiler_out, "r", encoding="utf-8") as f:
        for line in f.readlines():
            line = line[2:]
            if line[0] == '*':
                pattern = r'\*\s+(\S+).(sy|c)\s'
                match = re.search(pattern, line)
                file = match.group(1).split('/')[-1]
                files.append(file)
            elif line[0] == 'x':
                pattern = r'x\s+(\S+).compiler_out\s'
                file = match.group(1).split('/')[-1]
                wrong_files.append(file)
    return files, wrong_files

def check_asembler(asembler_out):
    wrong_files = []
    with open(asembler_out, "r", encoding="utf-8") as f:
        for line in f.readlines():
            line = line[2:]
            if line[0] != 'x':
                continue
            pattern = r'x\s+(\S+).asm_out\s'
            match = re.search(pattern, line)
            wrong_files.append(match.group(1).split('/')[-1])
    return wrong_files

def check_run(run_out):
    wrong_files = []
    success_files = {}
    with open(run_out, "r", encoding="utf-8") as f:
        for line in f.readlines():
            line = line[2:]
            if line[0] == '*' :
                # 正则表达式模式，分别匹配文件名和时间
                pattern = r'\*\s+(\S+).exe\s+---\s+time:\s+(\d+H-\d+M-\d+S-\d+us)'
                match = re.search(pattern, line)
                success_files[match.group(1).split('/')[-1]] = match.group(2)
            elif line[0] == 'x':
                pattern = r'x\s+(\S+).run_out\s'
                match = re.search(pattern, line)
                wrong_files.append(match.group(1).split('/')[-1])
    return wrong_files, success_files


def check_output(output_out):
    success_files = []
    failed_files = []
    with open(output_out, "r", encoding="utf-8") as f:
        for line in f.readlines():
            pattern = r'([\*x])\s+(\S+).out\s'    
            match = re.search(pattern, line)
            if match is None:
                continue
            if match.group(1) == '*' :
                success_files.append(match.group(2).split('/')[-1])
            elif match.group(1) == 'x':
                failed_files.append(match.group(2).split('/')[-1])
    return success_files, failed_files

def get_score_non_string(test_time, cmp_time):
    if test_time == timedelta(0, 0, 0, 0, 0, 0, 0):
        return 100
    return cmp_time / test_time * 100
def get_score_string(test_time, cmp_time):
    test_time = parse_time_string(test_time)    
    cmp_time = parse_time_string(cmp_time)
    return get_score_non_string(test_time, cmp_time)

def print_file(file_name, test_result, gcc_o0_result, gcc_o1_result, gcc_o2_result, gcc_o3_result):
    assert file_name in test_result.files, "file not in test"
    row = [file_name]
    if file_name in test_result.compiler_errors:
        row.append("compiler error")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        return row
    if file_name in test_result.asembler_errors:
        row.append("asembler error")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        return row
    if file_name in test_result.run_errors:
        row.append("run error")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        return row
    if file_name in test_result.output_errors:
        row.append("output error")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        return row
    if file_name not in test_result.run_times:
        row.append("not calculate time")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        row.append("/")
        return row
    if file_name not in test_result.output_success:
        row.append("no correct output")
    else:
        row.append("success")
    row.append(test_result.run_times[file_name])

    if file_name in gcc_o0_result.run_times:
        gcc_o0_score = get_score_string(test_result.run_times[file_name], gcc_o0_result.run_times[file_name])
        row.append("{}({:.4f})".format(gcc_o0_result.run_times[file_name], gcc_o0_score))
    else:
        row.append("no time")
    gcc_o1_score = get_score_string(test_result.run_times[file_name], gcc_o1_result.run_times[file_name])
    gcc_o2_score = get_score_string(test_result.run_times[file_name], gcc_o2_result.run_times[file_name])
    gcc_o3_score = get_score_string(test_result.run_times[file_name], gcc_o3_result.run_times[file_name])
    row.append("{}({:.4f})".format(gcc_o1_result.run_times[file_name], gcc_o1_score))
    row.append("{}({:.4f})".format(gcc_o2_result.run_times[file_name], gcc_o2_score))
    row.append("{}({:.4f})".format(gcc_o3_result.run_times[file_name], gcc_o3_score))
    return row

def better_run_time_num(test_result, cmp_result):
    better_num = 0
    worse_num = 0
    for k, v in test_result.run_times.items():
        if k not in cmp_result.run_times: # 用于比较的没有跑出来
            better_num += 1
            continue
        test_time = parse_time_string(v)
        cmp_time = parse_time_string(cmp_result.run_times[k])
        if test_time < cmp_time:
            better_num += 1
        if test_time > cmp_time:
            worse_num += 1
    return better_num, worse_num

def get_score(test_result, cmp_result):
    score = 0
    num = 0
    for k, v in test_result.run_times.items():
        if k not in cmp_result.run_times: # 用于比较的没有跑出来
            continue
        if k in cmp_result.output_errors or k in test_result.output_errors:
            continue
        test_time = parse_time_string(v)
        cmp_time = parse_time_string(cmp_result.run_times[k])
        if test_time == timedelta(0, 0, 0, 0, 0, 0, 0):
            continue
        score += cmp_time / test_time
        num += 1
    if num == 0:
        return 100
    return score / num * 100
def write(result_file, result):
    with open(result_file, "w", encoding="utf-8") as f:
        f.write(result)
# 输入： gcc 结果路径, 测试结果路径，汇总结果文件(.md)路径，描述信息 
# 输出汇总结果文件到指定路径
import sys
gcc_o0_run_time = sys.argv[1] + "/gcc_o0_time"
gcc_o1_run_time = sys.argv[1] + "/gcc_o1_time"
gcc_o2_run_time = sys.argv[1] + "/gcc_o2_time"
gcc_o3_run_time = sys.argv[1] + "/gcc_o3_time"
result_file = sys.argv[3]

result = "# {}\n".format(sys.argv[4].replace("\\n", "\n"))
result += "judge at {}\n".format(datetime.now())

import os
test_compiler = sys.argv[2] + "/test_compiler"
if os.path.exists(test_compiler):
    files, compiler_errors = check_compiler(test_compiler)
    result += "testcases num: {}\n".format(len(files))
    result += "compiler error num: {}\n".format(len(compiler_errors))
else:
    result += "no comiler output\n"
    write(result_file, result)
    exit(0)

test_asembler = sys.argv[2] + "/test_asembler"
if os.path.exists(test_asembler):
    asembler_errors = check_asembler(test_asembler)
    result += "asembler error num: {}\n".format(len(asembler_errors))
else:
    result += "no asembler output\n"
    write(result_file, result)
    exit(0)

test_exe = sys.argv[2] + "/test_exe"
if os.path.exists(test_exe):
    run_errors, run_times = check_run(test_exe)
    result += "run error num: {}\n".format(len(run_errors))
    result += "have run time num: {}\n".format(len(run_times))
else:
    result += "no run output\n"
    write(result_file, result)
    exit(0)

test_output = sys.argv[2] + "/test_output"
if os.path.exists(test_output):
    success_out, output_errors = check_output(test_output)
    result += "output error num: {}\n".format(len(output_errors))
else:
    result += "no output\n"
    write(result_file, result)
    exit(0)

test_result = judge_object(files, compiler_errors, asembler_errors, run_errors, run_times, output_errors, success_out)

_, gcc_o0_time = check_run(gcc_o0_run_time)
gcc_o0_result = judge_object(files, [], [], _, gcc_o0_time, [], [])
_, gcc_o1_time = check_run(gcc_o1_run_time)
gcc_o1_result = judge_object(files, [], [], _, gcc_o1_time, [], [])
_, gcc_o2_time = check_run(gcc_o2_run_time)
gcc_o2_result = judge_object(files, [], [], _, gcc_o2_time, [], [])
_, gcc_o3_time = check_run(gcc_o3_run_time)
gcc_o3_result = judge_object(files, [], [], _, gcc_o3_time, [], [])

better_num, worse_num = better_run_time_num(test_result, gcc_o0_result)
score = get_score(test_result, gcc_o0_result)
result += "better than {} -O0 num: {}, worse than {} -O0 num: {}, performance score:{:.4f}\n".format(sys.argv[1], better_num, sys.argv[1], worse_num, score)
better_num, worse_num = better_run_time_num(test_result, gcc_o1_result)
score = get_score(test_result, gcc_o1_result)
result += "better than {} -O1 num: {}, worse than {} -O1 num: {}, performance score:{:.4f}\n".format(sys.argv[1], better_num, sys.argv[1], worse_num, score)
better_num, worse_num = better_run_time_num(test_result, gcc_o2_result)
score = get_score(test_result, gcc_o2_result)
result += "better than {} -O2 num: {}, worse than {} -O2 num: {}, performance score:{:.4f}\n".format(sys.argv[1], better_num, sys.argv[1], worse_num, score)
better_num, worse_num = better_run_time_num(test_result, gcc_o3_result)
score = get_score(test_result, gcc_o3_result)
result += "better than {} -O3 num: {}, worse than {} -O3 num: {}, performance score:{:.4f}\n".format(sys.argv[1], better_num, sys.argv[1], worse_num, score)

rows = []
visited = {}
for file in compiler_errors:
    visited[file] = True
    rows.append(print_file(file, test_result, gcc_o0_result, gcc_o1_result, gcc_o2_result, gcc_o3_result))
for file in asembler_errors:
    visited[file] = True
    rows.append(print_file(file, test_result, gcc_o0_result, gcc_o1_result, gcc_o2_result, gcc_o3_result))
for file in run_errors:
    visited[file] = True
    rows.append(print_file(file, test_result, gcc_o0_result, gcc_o1_result, gcc_o2_result, gcc_o3_result))
for file in output_errors:
    visited[file] = True
    rows.append(print_file(file, test_result, gcc_o0_result, gcc_o1_result, gcc_o2_result, gcc_o3_result))
for file in files:
    if file in visited:
        continue
    rows.append(print_file(file, test_result, gcc_o0_result, gcc_o1_result, gcc_o2_result, gcc_o3_result))
row = ["average", "/", time_string(test_result.averarge_run_time)]
row.append("{}({:.4f})".format(time_string(gcc_o0_result.averarge_run_time), get_score_non_string(test_result.averarge_run_time, gcc_o0_result.averarge_run_time)))
row.append("{}({:.4f})".format(time_string(gcc_o1_result.averarge_run_time), get_score_non_string(test_result.averarge_run_time, gcc_o1_result.averarge_run_time)))
row.append("{}({:.4f})".format(time_string(gcc_o2_result.averarge_run_time), get_score_non_string(test_result.averarge_run_time, gcc_o2_result.averarge_run_time)))
row.append("{}({:.4f})".format(time_string(gcc_o3_result.averarge_run_time), get_score_non_string(test_result.averarge_run_time, gcc_o3_result.averarge_run_time)))
rows.append(row)

headers = ["file", "status", "hdu-compiler", "gcc -O0", "gcc -O1", "gcc -O2", "gcc -O3"]
table = "| " + " | ".join(headers) + " |\n"
table += "| " + " | ".join(["---"] * len(headers)) + " |\n"
for row in rows:
    table += "| " + " | ".join(row) + " |\n"
result += table
write(result_file, result)
