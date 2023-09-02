#########################################################################
# File Name: github_ci.sh
# Author: Deng Zengyong
# mail: 2595650269@qq.com
# Created Time: 2023年09月02日 星期六 17时07分10秒
#########################################################################
#!/bin/bash
if_have_tested() {
	sha_id=$1
	current_sha_id=$(git rev-parse HEAD)
	git checkout raspi_log
	if [ -d "./raspi_log/$sha_id" ]; then
		git checkout $current_sha_id
		return 0
	fi
	git checkout $current_sha_id
	return 1
}

current_sha_id=$(git rev-parse HEAD)
sha_id=$current_sha_id
while ! if_have_tested "$sha_id"; do
	commit_description=$(git log -n 1 --format="%h %an %ad %s" $sha_id) 
	echo "sha_id: $sha_id"
	echo "description: $commit_description"
	make clean
	make VERSION=release release_C_SETS="-O2 -fPIE"
	mkdir "$sha_id"
	make test VERSION=release -j8 > "$sha_id/test_compiler"
	mkdir ./tmp
	find test -type f -name "*.s" -exec  rsync -R {} ./tmp  \;   
	rsync -avz -e ssh ./tmp 25956@raspberrypi:/home/25956/Compiler
	rm -rf ./tmp
	command_raspi="cd Compiler/tmp
		nohup find test -type f -name "*.s" -exec  rsync -R {} ../  \; 
		cd ..  
		nohup rm -rf ./tmp
		nohup rm "./test_asembler" "./test_exe" "./test_output"
		nohup make test_asm -j4 > "./test_asembler"
		nohup make test_exe > "./test_exe"
		nohup make test_output > "./test_output"
		nohup make clean
		nohup echo 'raspi finish' "
	ssh 25956@raspberrypi "$command_raspi"
	rsync -avz 25956@raspberrypi:/home/25956/Compiler/test_asembler "$sha_id/"
	rsync -avz 25956@raspberrypi:/home/25956/Compiler/test_exe "$sha_id/"   
	rsync -avz 25956@raspberrypi:/home/25956/Compiler/test_output "$sha_id/"
	git checkout raspi_log
	python3 show_result.py "raspi_gcc" "$sha_id" "$sha_id/result.md" "remote raspi test commit\n$commit_description"
	mv "$sha_id" "./raspi_log/"
	rm -rf "$sha_id"
	git checkout $sha_id
	git checkout HEAD^
	sha_id=$(git rev-parse HEAD)	
done

# 情空已经不存在的 commit
git checkout raspi_log
git add "./raspi_log"
git commit -m "raspi_log: $current_sha_id" 
git push 
if [ $# -eq 1 ]; then
	# 将结果保存到指定文件
	cp "raspi_log/$current_sha_id/result.md" $1 
fi
