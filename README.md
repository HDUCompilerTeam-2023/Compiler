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
- [ ] 6. 语义分析
- [x] 7. 通过参数传入文件
- [x] ~~8. 头文件支持~~
- [ ] 9. 参数支持
- [ ] 10. 更多语法支持
- [x] 11. 错误恢复
- [ ] ~~12. 预处理~~
- [ ] 13. 更详细的错误恢复
- [ ] 14. 前端部分输出携带位置信息
- [ ] 15. 中级中间代码生成
- [ ] 16. 高级中间代码优化
- [x] 17. 数组定义时各维长度非负整数，且各维长度都要显示给出
- [ ] 18. 数组作为函数形参第一维省去 剩余给出，实参数组长度都要严格一致
- [x] 19. 逻辑运算的结果用 int 表示， float 做逻辑运算做类型转换
- [x] ~~20. 拆分赋值 一元和二元 的运算（看情况~~
- [ ] 21. 编译器能确定的数组引用 需要检查越界（0到定义时长度）
- [ ] 22. 初始化列表 (尽量完成)
- [x] 23. 修改文法，贴近大赛要求——`const`部分、函数形参部分与表达式等
- [ ] 24. 常量变量的折叠
- [ ] 25. 声明运行时库里的函数
