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
            return 0
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

def print_file(file_name, test_result1, test_result2):
    row = [file_name]
    have_run_time1 = False
    have_run_time2 = False
    if file_name in test_result1.compiler_errors:
        row.append("compiler error")
    elif file_name in test_result1.asembler_errors:
        row.append("asembler error")
    elif file_name in test_result1.run_errors:
        row.append("run error")
    elif file_name in test_result1.output_errors:
        row.append("output error")
    elif file_name not in test_result1.files:
        row.append("not run")
    elif file_name not in test_result1.run_times:
        row.append("not caluate time")
    else:
        have_run_time1 = True
        row.append(test_result1.run_times[file])
    if file_name in test_result2.compiler_errors:
        row.append("compiler error")
    elif file_name in test_result2.asembler_errors:
        row.append("asembler error")
    elif file_name in test_result2.run_errors:
        row.append("run error")
    elif file_name in test_result2.output_errors:
        row.append("output error")
    elif file_name not in test_result2.files:
        row.append("not run")
    elif file_name not in test_result2.run_times:
        row.append("not caluate time")
    else:
        have_run_time2 = True
        row.append(test_result2.run_times[file])
    if have_run_time1 and have_run_time2:
        row.append("{:.4f}".format(get_score_string(test_result1.run_times[file], test_result2.run_times[file])))
    else:
        row.append("/")
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
        if test_time > worse_num:
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

def get_judge_object(dir):
    files, compiler_errors = check_compiler(dir + "/test_compiler")
    asembler_errors = check_asembler(dir + "/test_asembler")
    run_errors, success_exe = check_run(dir + "/test_exe")
    success_out, failed_out = check_output(dir + "/test_output")
    return judge_object(files, compiler_errors, asembler_errors, run_errors, success_exe, failed_out, success_out)
import sys

# 输入： 两个结果文件夹路径以及比较结果文件夹路径
# 输出： 复制两个结果文件夹下的md文件 以及比较结果(compare_result.md)到结果文件夹
import os 
os.system("rm -rf {}".format(sys.argv[3]))
os.system("mkdir {}".format(sys.argv[3]))
os.system("cp {} {}".format(sys.argv[1] + "/result.md", sys.argv[3] + "/{}_result.md".format(sys.argv[1].split('/')[-1])))
os.system("cp {} {}".format(sys.argv[2] + "/result.md", sys.argv[3] + "/{}_result.md".format(sys.argv[2].split('/')[-1])))

test_result1 = get_judge_object(sys.argv[1])
test_result2 = get_judge_object(sys.argv[2])

rows = []
visited = {}
for file in test_result1.compiler_errors:
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result1.asembler_errors:
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result1.run_errors:
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result1.output_errors:
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result2.compiler_errors:
    if file in visited:
        continue
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result2.asembler_errors:
    if file in visited:
        continue
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result2.run_errors:
    if file in visited:
        continue
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result1.output_errors:
    if file in visited:
        continue
    visited[file] = True
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result1.files:
    if file in visited:
        continue
    rows.append(print_file(file, test_result1, test_result2))
for file in test_result2.files:
    if file in visited:
        continue
    rows.append(print_file(file, test_result1, test_result2))
row = ["average", time_string(test_result1.averarge_run_time), time_string(test_result2.averarge_run_time), "{:.4f}".format(get_score_non_string(test_result1.averarge_run_time, test_result2.averarge_run_time))]
rows.append(row)

headers = ["file", sys.argv[1], sys.argv[2], "better rate"]
table = "| " + " | ".join(headers) + " |\n"
table += "| " + " | ".join(["---"] * len(headers)) + " |\n"
for row in rows:
    table += "| " + " | ".join(row) + " |\n"

with open(sys.argv[3] + "/compare_result.md", "w", encoding="utf-8") as f:
    f.write("# compare {} with {}\n".format(sys.argv[1], sys.argv[2]))
    f.write("compiler1 result in {}\n".format(sys.argv[3] + "/{}_result.md".format(sys.argv[1].split('/')[-1])))
    f.write("compiler2 result in {}\n".format(sys.argv[3] + "/{}_result.md".format(sys.argv[2].split('/')[-1])))
    f.write("testcases num: compiler1: {}, compiler2: {}\n".format(len(test_result1.files), len(test_result2.files)))
    f.write("compiler error num: compiler1: {}, compiler2: {}\n".format(len(test_result1.compiler_errors), len(test_result2.compiler_errors)))
    f.write("asembler error num: compiler1: {}, compiler2: {}\n".format(len(test_result1.asembler_errors), len(test_result2.asembler_errors)))
    f.write("run error num: compiler1: {}, compiler2: {}\n".format(len(test_result1.run_errors), len(test_result2.run_errors)))
    f.write("output error num: compiler1: {}, compiler2: {}\n".format(len(test_result1.output_errors), len(test_result1.output_errors)))
    f.write("have run time num: compiler1: {}, compiler2: {}\n".format(len(test_result1.run_times), len(test_result2.run_times)))
    better_num, worse_num = better_run_time_num(test_result1, test_result2)
    score = get_score(test_result1, test_result2)
    f.write("compiler1 better than compiler2  num: {}\n".format(better_num))
    f.write("compiler1 worse than compiler2  num: {}\n".format(worse_num))
    f.write("compiler1/compiler2 performance score: {:.4f}\n".format(score))
    f.write(table)