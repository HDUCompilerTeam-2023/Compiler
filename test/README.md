# 测试用例

`2021初赛所有用例` ：2021初赛期间所有的功能用例与性能用例，h_开头的为隐藏用例。

`performance_test2021-public` ：前5组为2021年编译大赛预赛性能用例，后5组为2020年决赛阶段用例，同时也是2021决赛阶段的**部分**测试用例。

`performance_test2021-private` ：10组决赛的隐藏用例，赛后全部公开。

`function_test2021` ：2021年编译大赛初赛的**公开**用例。每个测试用例由同名文件的`.sy` , `.out`和`.in`文件组成。

`function_test2020` : 2020年编译大赛的功能测试用例，与2021部分重复，可以用来本地测试用。每个测试用例由同名文件的`.sy` , `.out`和`.in`文件组成。



# 运行时库

`libsysy.a`: arm架构的SysY运行时静态库

`libsysy.so`: arm架构的SysY运行时动态库

`sylib.h`: SysY运行时库头文件

`sylib.c`: SysY运行时库源文件

# 测评程序代码

`source`:大赛评测程序部分代码,`Compiler.java`负责编译从gitlab上拉取的项目。方便大家看到评测机如何编译大家的代码。

# 测评镜像

测评镜像的Dockerfile被存放在`docker`文件夹下,其中ARM-Dockerfile是树莓派上的汇编链接镜像,x86-Dockerfile是x86上的编译测评镜像