# Simple Compiler of Sys-Y

## Build

使用 GNU make 构建，通过`make help`查看可支持的命令

## Directories

    /
    ├── doc/       --- 文档 （ 目前暂无 ）
    ├── build/     --- 构建目标 （ 由 make clean 删除 ）
    ├── tmp-src/   --- flex 与 bison 生成的 C 文件 （ 由 make clean 删除 ）
    ├── grammar/   --- flex 与 bison 的文法定义
    ├── include/   --- C 头文件
    ├── script/    --- Makefile 子文件
    ├── src/       --- C 源文件
    ├── test/      --- 测试用例
    ├── .gitignore
    ├── Makefile
    └── README.md

## Change log

[CHANGELOG](doc/CHANGELOG.md)

## TODO

- [x] 1. 确认语法分析过程中使用到的`Exp`、`DefExp`、`ConstExp`正确无误（目前直接使用`Exp`、`AssignExp`）
- [ ] 2. 增加浮点数
- [ ] 3. 修改 if-else 部分的规则，以去除相关的优先级定义
- [ ] 4. 生成语法分析树
