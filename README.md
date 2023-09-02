# Simple Compiler of Sys-Y

## Build

使用 GNU make 构建，通过`make help`查看可支持的命令

通过`VERSION`参数传递编译版本，可选值为`debug`与`release`，默认为`debug`

## Commit

     <type>[<scope>]: <description>
     <blank line>
    [<body>
     <blank line>]
    [<footer>]

- description 使用祈使句，一般现在时，首字母小写
- body 使用祈使句，一般现在时，首字母大写，包含动机，并且与之前的行为对比
- footer
  - `BREAKING CHANGE:` 说明破坏性更改（与 MAJOR 版本号相关）
- type
  - `fix` - 修复 BUG （与 PATCH 版本号相关）
  - `feat` - 新增功能（与 MINOR 版本号相关）
  - `build` - 构建系统的变更
  - `perf` - 提高代码性能
  - `refactor` - 不增加新功能也不修复 BUG 的更改
  - `style` - 代码风格修改，不影响代码含义
  - `test` - 添加或修改测试
  - `docs` - 修改文档
- scope
  - `lexer` - 修改词法分析部分
  - `parser` - 修改语法分析部分
  - `output` - 修改日志等输出部分
  - `changelog` - 更新 changelog
  - `TODO` - 更新 TODO
  - 待更新

## Directories

    /
    ├── doc/       --- 文档
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
- [x] 2. 增加浮点数
- [x] 3. ~~修改 if-else 部分的规则，以去除相关的优先级定义~~
- [x] 4. 生成语法分析树
- [x] 5. 构建符号表
- [x] 6. 语义分析
- [x] 7. 通过参数传入文件
- [x] ~~8. 头文件支持~~
- [ ] ~~9. 参数支持~~
- [ ] 10. 更多语法支持
- [x] 11. 错误恢复
- [ ] ~~12. 预处理~~
- [ ] 13. 更详细的错误恢复
- [x] 14. 前端部分输出携带位置信息
- [x] 15. 中级中间代码生成
- [ ] ~~16. 高级中间代码优化~~
- [x] 17. 数组定义时各维长度非负整数，且各维长度都要显示给出
- [x] 18. 数组作为函数形参第一维省去 剩余给出，实参数组长度都要严格一致
- [x] 19. 逻辑运算的结果用 int 表示， float 做逻辑运算做类型转换
- [x] ~~20. 拆分赋值 一元和二元 的运算（看情况~~
- [ ] ~~21. 编译器能确定的数组引用 需要检查越界（0到定义时长度）~~
- [x] 22. 初始化列表 (尽量完成)
- [x] 23. 修改文法，贴近大赛要求——`const`部分、函数形参部分与表达式等
- [x] 24. 常量变量的折叠
- [x] 25. 声明运行时库里的函数
- [x] 26. simplifyCFG 对 ssa 的支持
- [ ] ~~26. 确定参数传递方式~~
- [ ] ~~27. 通过模拟器确定翻译的正确性~~
- [x] 28. 简单的寄存器分配
- [x] 29. 简单的代码生成
- [x] 30. 修改 mir
  - [x] 局部变量使用 alloca 指令，全局变量在 operand 中使用 vmem 代表地址
  - [x] 参数传递采用vreg传递， 函数开头开辟空间去 store
  - [ ] ~~func 传参采用数组而不是 list~~
  - [x] 数组不采用一次偏移，向 llvm 看齐（类型系统）
- [x] 31. 修改 hir
  - [x] 数组
  - other
- [ ] ~~32. 输出成 llvm 格式，采用 llvm 做检验~~
- [x] 33. 合并 symbol 与 mir 的 func
- [x] 34. 输入输出格式
- [x] 35. `arm_lir_trans`常数加常数不放入寄存器
- [x] 36. 变量初始化
- [x] 37. load store 指令的 sp 支持
- [x] 38. 删掉 `add_at`
- [x] 39. Side Effects
- [ ] 40. MemorySSA
- [ ] 41. 冗余存储消除
- [ ] 42. 存储代码移动
- [ ] 43. LoopUnroll
- [ ] 44. LoopUnswitch
- [ ] 45. LoopStrengthReduction
- [ ] 46. Reassociate
- [ ] 47. PeepHole
- [ ] 48. InstrSchedule
- [ ] 49. IPADCE
- [ ] 50. Parallel and SIMD
- [ ] 51. IP OPT

## 关于测试
### 本地评测
* 在 `./qemu_arm_gcc` 下已经输出了交叉编译器`gcc -O0`、`gcc -O1`、`gcc -O2`、`gcc -O3`的所有运行时间
* 通过运行 `./test_whole.sh test_dir result_dir` 对 `test_dir` 下的所有 `.sy` 文件运行编译器、交叉编译的链接器、可执行文件，将编译器、连接器、可执行文件以及输出的测试结果输出到 `result_dir` ,同时与 `./qemu_arm_gcc` 的运行时间进行比较产生结果报告 `result.md`
* 如果有两个编译器的输出结果(包括 `test_compiler`、`test_asembler`、`test_exe`、`test_output`、 `result.md`) 分别在 `dir1`、`dir2` 下，想比较两个编译器的结果差异可以使用 `python compare.py dir1 dir2 result_dir`, 在 `result_dir` 会输出两个编译器分别的报告以及比较报告 `compare_result.md`
  
### 远端（树莓派）评测
* 有一个专门的分支 `raspi_log` 存放了所有提交的运行结果情况，每次 `push` 后会在树莓派上运行该分支所有未测试的提交，并将当前最新分支的结果报告 `test-results.md` 返回到 `github action` 上。 (未来可以将结果发邮件或者以网页形式展示)
* 如果想比较两个提交在远端运行结果的差异可以运行 `./compare_commit.sh commit1 commit2 result_dir` , 如比较当前和上一个差异：`./compare_commit.sh HEAD HEAD^ result_dir`, 运行的结果与本地评测运行 `compare.py` 的结果一致