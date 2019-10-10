---
title: golang 设计模式之选项模式
date: 2018-03-10 22:25:25
tags: [golang, 设计模式, 选项模式]
thumbnail: /img/thumbnail/fish.jpg
---

有时候一个函数会有很多参数，为了方便函数的使用，我们会给希望给一些参数设定默认值，调用时只需要传与默认值不同的参数即可，类似于 python 里面的默认参数和字典参数，虽然 golang 里面既没有默认参数也没有字典参数，但是我们有选项模式

### 可变长参数列表

在这之前，首先需要介绍一下可变长参数列表，顾名思义，就是参数的个数不固定，可以是一个也可以是多个，最典型的用法就是标准库里面的 `fmt.Printf`，语法比较简单，如下面例子实现任意多个参数的加法

``` go
func add(nums ...int) int {
    sum := 0
    for _, num := range nums {
        sum += num
    }
    return sum
}

So(add(1, 2), ShouldEqual, 3)
So(add(1, 2, 3), ShouldEqual, 6)
```

在类型前面加 `...` 来表示这个类型的变长参数列表，使用上把参数当成 `slice` 来用即可

### 选项模式

假设我们要实现这样一个函数，这个函数接受5个参数，三个 `string`（其中第一个参数是必填参数），两个 `int`，这里功能只是简单输出这个参数，于是我们可以简单用如下代码实现

``` go
func MyFunc1(requiredStr string, str1 string, str2 string, int1 int, int2 int) {
    fmt.Println(requiredStr, str1, str2, int1, int2)
}

// 调用方法
MyFunc1("requiredStr", "defaultStr1", "defaultStr2", 1, 2)
```

这种实现比较简单，但是同时传入参数较多，对调用方来说，使用的成本就会比较高，而且每个参数的具体含义这里并不清晰，很容易出错

那选项模式怎么实现这个需求呢？先来看下最终的效果

``` go
MyFunc2("requiredStr")
MyFunc2("requiredStr", WithOptionStr1("mystr1"))
MyFunc2("requiredStr", WithOptionStr2AndInt2("mystr2", 22), WithOptionInt1(11))
```

如上面代码所示，你可以根据自己的需求选择你需要传入的参数，大大简化了函数调用的复杂度，并且每个参数都有了清晰明确的含义

那怎么实现上面的功能呢

#### 定义可选项和默认值

首先定义可选项和默认值，这里有4个可选项，第一个参数为必填项

``` go
type MyFuncOptions struct {
    optionStr1 string
    optionStr2 string
    optionInt1 int
    optionInt2 int
}

var defaultMyFuncOptions = MyFuncOptions{
    optionStr1: "defaultStr1",
    optionStr2: "defaultStr2",
    optionInt1: 1,
    optionInt2: 2,
}
```

#### 实现 With 方法

这些 With 方法看起来有些古怪，接受一个选项参数，返回一个选项方法，而选项方法以选项作为参数负责修改选项的值，如果没看明白没关系，可以先看函数功能如何实现

``` go
type MyFuncOption func(options *MyFuncOptions)

func WithOptionStr1(str1 string) MyFuncOption {
    return func(options *MyFuncOptions) {
        options.optionStr1 = str1
    }
}

func WithOptionInt1(int1 int) MyFuncOption {
    return func(options *MyFuncOptions) {
        options.optionInt1 = int1
    }
}

func WithOptionStr2AndInt2(str2 string, int2 int) MyFuncOption {
    return func(options *MyFuncOptions) {
        options.optionStr2 = str2
        options.optionInt2 = int2
    }
}
```

这里我们让 optionStr2 和 optionInt2 合并一起设置，实际应用场景中可以用这种方式将相关的参数放到一起设置

#### 实现函数功能

``` go
func MyFunc2(requiredStr string, opts ...MyFuncOption) {
    options := defaultMyFuncOptions
    for _, o := range opts {
        o(&options)
    }

    fmt.Println(requiredStr, options.optionStr1, options.optionStr2, options.optionInt1, options.optionInt2)
}
```

使用 With 方法返回的选项方法作为参数列表，用这些方法去设置选项

### 选项模式的应用

从这里可以看到，为了实现选项的功能，我们增加了很多的代码，实现成本相对还是较高的，所以实践中需要根据自己的业务场景去权衡是否需要使用。个人总结满足下面条件可以考虑使用选项模式

1. 参数确实比较复杂，影响调用方使用
2. 参数确实有比较清晰明确的默认值
3. 为参数的后续拓展考虑

在 golang 的很多开源项目里面也用到了选项模式，比如 grpc 中的 rpc 方法就是采用选项模式设计的，除了必填的 rpc 参数外，还可以一些选项参数，grpc_retry 就是通过这个机制实现的，可以实现自动重试功能

### 参考链接

- Go 函数式选项模式：<https://studygolang.com/articles/12329>
- Functional Options Pattern in Go：<https://halls-of-valhalla.org/beta/articles/functional-options-pattern-in-go,54/>
