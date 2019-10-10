---
title: shell 编程
date: 2018-11-04 01:58:18
tags: [shell, 编程]
---

## 变量

变量定义：`name=Tom`
变量使用：`echo $name`
自定义环境变量：`export name`
常见环境变量：`$HOME $PATH`
查看环境变量：`env`
声明只读变量：`readonly name`
特殊变量：
```
$0  shell执行程序名
$n  位置参数，n = 1...9
$*  所有位置参数组成的字符串
$#  位置参数的个数
$$  程序执行后的pid，常用来生产临时文件
$!  最后一个后台程序执行后的pid
$?  命令执行后的返回值，0表示成功，其他失败
```
位置参数只能有9个，可以用shift命令将位置参数后移一格
set命令可以改变位置参数，执行set "p1 p2 p3"后，$0=p1 $1=p2 $2=p3

## 数值运算

### 整数运算

```bash
# 4种方式实现自增
i=0
((i++))
let i++
expr $i + 1
echo $i 1 | awk '{printf($1+$2);}'
```

### 浮点运算

(())，let和expr都无法进行浮点运算，bc和awk可以
```bash
echo "10 3" | awk '{printf("%0.3f", $1/$2)}'
```

### 随机数

环境变量RANDOM产生0到32767的随机数，而awk的rand函数可以产生0到1之间的随机数
```bash
echo $RANDOM
echo "" | awk '{srand(); printf("%f", rand());}'
```

### 序列数

可以用循环实现，建议用seq工具
```bash
seq 5           # 1 2 3 4 5
seq 1 2 5       # 1 3 5
seq -s: 1 2 5   # 1:3:5
seq -w 1 3 10   # 01 04 07 10
{1..5}          # 1 2 3 4 5
```

## 函数

可以有return语句，但是只能返回整数
```bash
# 定义hello函数
hello () {
    echo "hello $1"
}
hello hatlonely     # 调用hello，hatlonely为参数
```

## 控制语句

```bash
#!/bin/bash

# 遍历目录
for file in `ls`; do
    if [ -f $file ]; then
        echo "$file is a file"
    else
        echo "$file is not a file"
    fi
done

# for循环
for ((i = 0; i < 10; i++)) do
    echo $i
done

# while循环
i=0
while [ $i -lt 10 ]; do
    echo $i
    ((i++))
done

# 处理命令行参数
usage()
{
    echo "usage: $0 [-a] [-e <admin>] [-f <serverfile>] [-h] [-d <domain>] [-s <whois_server>] [-q] [-x <warndays>]"
}

while getopts ae:f:hd:s:qx: option; do
    case "${option}" in
        a) ALARM="TRUE";;
        e) ADMIN=${OPTARG};;
        d) DOMAIN=${OPTARG};;
        f) SERVERFILE=$OPTARG;;
        s) WHOIS_SERVER=$OPTARG;;
        q) QUIET="TRUE";;
        x) WARNDAYS=$OPTARG;;
        \?) usage; exit 1;;
    esac
done

echo "ALARM=$ALARM"
echo "ADMIN=$ADMIN"

# 读取输入
while read line; do
    echo $line
done
```

## 字符串

### 判断字符串的值

```
${var}  变量var的值, 与$var相同

${var-DEFAULT}  如果var没有被声明, 那么就以$DEFAULT作为其值 *
${var:-DEFAULT} 如果var没有被声明, 或者其值为空, 那么就以$DEFAULT作为其值 *

${var=DEFAULT}  如果var没有被声明, 那么就以$DEFAULT作为其值 *
${var:=DEFAULT} 如果var没有被声明, 或者其值为空, 那么就以$DEFAULT作为其值 *

${var+OTHER}    如果var声明了, 那么其值就是$OTHER, 否则就为null字符串
${var:+OTHER}   如果var被设置了, 那么其值就是$OTHER, 否则就为null字符串

${var?ERR_MSG}  如果var没被声明, 那么就打印$ERR_MSG *
${var:?ERR_MSG} 如果var没被设置, 那么就打印$ERR_MSG *

${!varprefix*}  匹配之前所有以varprefix开头进行声明的变量
${!varprefix@}  匹配之前所有以varprefix开头进行声明的变量
```

### 字符串操作

```
${#string}                  $string的长度

${string:position}          在$string中, 从位置$position开始提取子串
${string:position:length}   在$string中, 从位置$position开始提取长度为$length的子串

${string#substring}     从变量$string的开头, 删除最短匹配$substring的子串
${string##substring}    从变量$string的开头, 删除最长匹配$substring的子串
${string%substring}     从变量$string的结尾, 删除最短匹配$substring的子串
${string%%substring}    从变量$string的结尾, 删除最长匹配$substring的子串

${string/substring/replacement}     使用$replacement, 来代替第一个匹配的$substring
${string//substring/replacement}    使用$replacement, 代替所有匹配的$substring
${string/#substring/replacement}    如果$string的前缀匹配$substring,
                                    那么就用$replacement来代替匹配到的$substring
${string/%substring/replacement}    如果$string的后缀匹配$substring,
                                    那么就用$replacement来代替匹配到的$substring
```

## 条件测试

使用[]测试时，前后都要空格

### 文件测试

```
-a file exists.
-b file exists and is a block special file.
-c file exists and is a character special file.
-d file exists and is a directory.
-e file exists (just the same as -a).
-f file exists and is a regular file.
-g file exists and has its setgid(2) bit set.
-G file exists and has the same group ID as this process.
-k file exists and has its sticky bit set.
-L file exists and is a symbolic link.
-n string length is not zero.
-o Named option is set on.
-O file exists and is owned by the user ID of this process.
-p file exists and is a first in, first out (FIFO) special file or named pipe.
-r file exists and is readable by the current process.
-s file exists and has a size greater than zero.
-S file exists and is a socket.
-t file descriptor number fildes is open and associated with a terminal device.
-u file exists and has its setuid(2) bit set.
-w file exists and is writable by the current process.
-x file exists and is executable by the current process.
-z string length is zero.
```

### 整数比较

```
-eq     ==
-ne     !=
-ge     >=
-gt     >
-le     <=
-lt     <
```

### 字符串测试

```
string      测试字符串不为空
-n string   测试字符串不为空
-z string   测试字符串为空
str1=str2   字符串相同
str1!=str2  字符串不同
```


### 逻辑运算

```
!expression     not
expr1 -a expr2  and
expr1 -o expr2  or
```

## 路径处理

```bash
$ temp=/home/hatlonely/hatlonely/librayrs.txt
$ echo `basename ${temp}`
$ echo `dirname ${temp}`
$ echo ${temp##*/}
$ echo ${temp%/*}
```

## 颜色

格式: `echo "/033[字背景颜色;字体颜色m字符串/033[控制码"`
如果单纯显示字体颜色可以固定控制码位`0m`。
例如: `echo "/033[字背景颜色;字体颜色m字符串/033[0m"`

### 字背景颜色范围:40 - 49

```
40:黑
41:深红
42:绿
43:黄色
44:蓝色
45:紫色
46:深绿
47:白色
```

### 字颜色:30 - 39

```
30:黑
31:红
32:绿
33:黄
34:蓝色
35:紫色
36:深绿
37:白色
```

### ANSI控制码

```
\33[0m 关闭所有属性
\33[01m 设置高亮度
\33[04m 下划线
\33[05m 闪烁
\33[07m 反显
\33[08m 消隐
\33[30m -- \33[37m 设置前景色
\33[40m -- \33[47m 设置背景色
\33[nA 光标上移n行
\33[nB 光标下移n行
\33[nC 光标右移n行
\33[nD 光标左移n行
\33[y;xH设置光标位置
\33[2J 清屏
\33[K 清除从光标到行尾的内容
\33[s 保存光标位置
\33[u 恢复光标位置
\33[?25l 隐藏光标
\33[?25h 显示光标
```

### 例子

```
echo -e "\033[31m \033[05m 请确认您的操作,输入 [Y/N] \033[0m"
```
