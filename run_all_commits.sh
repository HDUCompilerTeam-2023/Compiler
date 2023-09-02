#########################################################################
# File Name: run_all_commits.sh
# Author: Deng Zengyong
# mail: 2595650269@qq.com
# Created Time: 2023年09月02日 星期六 20时16分37秒
#########################################################################
#!/bin/bash
current=$(git rev-parse HEAD)
file_path="./commit_list_all.txt"
git log --all --pretty=format:%H > $file_path
cp "./github_ci.sh" "./ci.sh"
chmod +x "./ci.sh"
while IFS= read -r line; do
    echo "sha: $line"
    git checkout $line
    ./ci.sh
    # 在这里执行你希望的操作
done < "$file_path"
rm "./ci.sh"
git checkout $current

