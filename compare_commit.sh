#########################################################################
# File Name: compare_commit.sh
# Author: Deng Zengyong
# mail: 2595650269@qq.com
# Created Time: 2023年08月31日 星期四 14时40分04秒
#########################################################################
#!/bin/bash
current_branch=$(git symbolic-ref --short HEAD)
commit1=$(git rev-parse $1)
commit2=$(git rev-parse $2)
git fetch
git checkout origin/raspi_log
python3 compare.py "raspi_log/$commit1" "raspi_log/$commit2" $3
git checkout $current_branch
