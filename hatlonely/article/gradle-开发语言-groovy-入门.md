---
title: gradle 开发语言 groovy 入门
date: 2018-03-21 16:38:45
tags: [groovy, gradle, closure]
---

gradle 是 java 的构建工具的一种，同类的工具还有 maven，ant。我比较喜欢 gradle 主要是其语法简洁，而且功能强大。除此之外，还支持很多其他语言的构建

gradle 使用的是 groovy 开发，以前没有接触过这个语言，以后可能也不太会用，了解这个语言只是想看懂 `build.gradle` 这个脚本，以及后续能在此基础上作简单的拓展开发

### 简介

groovy 是基于 jvm 的语言，支持很多动态和静态两种模式，既可以用作脚本开发，也可以用来构建大型的服务，建立在强大的 java 生态之上，同时支持很多现代的语法特性，可以说是非常强大，使用上面也非常方便

### 基本数据类型

#### 字符串

``` groovy
assert "hello world".contains("hello")
assert "hello world".startsWith("hello")
assert "hello world".endsWith("world")
assert "0123456789".indexOf("678") == 6
assert "0123456789".charAt(6) == "6"
assert "0123456789".length() == 10
assert "0123456789".substring(5) == "56789"
assert "0123456789".substring(5, 8) == "567"

assert "0123456789"[6] == "6"
assert "0123456789"[6..8] == "678"
assert "0123456789"[2, 3, 7] == "237"
assert "0123456789"[1, 3..<6, 8] == "13458"
assert "0123456789" == "0123456789"
assert "0123456789".is("0123456789")
assert !""
```

- 字符串可以用双引号也可以用单引号，
- 可以直接调用 java 中的字符串方法
- 的函数调用的括号可以省略，如上面的 `assert` 函数
- 相比较 java，groovy 增加了很多运算符，如上面用到的 `==`, `[]`, `..`, `is` 等等
- 空字符串会被当做 false 处理，类似的，`0`，`null`，`[]` 也会被当做 false 处理

#### 容器

``` groovy
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].contains(3)
assert 3 in [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].indexOf(6) == 6
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].get(6) == 6
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9][6] == 6
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9][6, 8] == [6, 8]
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9][6..8] == [6, 7, 8]
assert [[2, 3],[4, 5, 6]].combinations() == [[2, 4], [3, 4], [2, 5], [3, 5], [2, 6], [3, 6]]
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].each { ++it } == [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
assert [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].collect { ++it } == [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

assert ["a": 1, "b": 2, "c": 3]["b"] == 2
assert "a" in ["a": 1, "b": 2, "c": 3]

["a": 1, "b": 2, "c": 3].each {
    println(it.key + " => " + it.value)
}

def items = [4, 5]
def list = [1, 2, 3, *items, 6]
assert list == [1, 2, 3, 4, 5, 6]

def m1 = [c: 3, d: 4]
def map = [a: 1, b: 2, *:m1]
assert map == [a: 1, b: 2, c: 3, d: 4]
```

- 相比 java，groovy 支持字面语法，定义 list 和 map 方便了很多
- 可以使用 def 定义变量，而无需显式的写明类型
- 可以用 `each` 实现循环遍历，实际上 each 的参数是一个闭包，闭包里面的 `it` 是个关键字，其实代表的是第一个参数
- 使用 `collect` 方法实现列表的转化

### 闭包

这个东西很有意思，简单理解就是一段代码，这段代码可以传入参数，也可以有上下文，这个东西也是一个对象，可以用来赋值，也可传递

``` groovy
def add = { a,b -> a+b }
assert add(1, 2) == 3
assert add("a", "b") == "ab"
```

`{ a,b -> a+b }` 这就是一个闭包，`a,b` 是传入的参数，如果不需要参数也可以省略，当只有一个参数的时候也可以省略，因为有个默认参数 `it`，`->` 后面是要执行的代码，如果没有显式地指定 return 语句，默认会以最后一个语句返回，这里返回值就是 `a+b`，因此这个闭包可以被用来当做 add 函数使用

``` groovy
def ncopies = {int n, String str -> str*n}
def twice = ncopies.curry(2)    // curry 参数绑定
assert twice("bla") == ncopies(2, "bla")
```

可以用 `curry` 绑定参数值

``` groovy
fib = { long n -> n < 2 ? n : fib(n-1)+fib(n-2) }.memoize()
assert fib(15) == 610
```

这还能自动帮你缓存递归的结果……感觉有点过分了……

``` groovy
def plus2  = { it + 2 }
def times3 = { it * 3 }
def times3plus2 = plus2 << times3
assert times3plus2(3) == 11
assert times3plus2(4) == plus2(times3(4))
```

可以把多个闭包连起来组成新的闭包

``` groovy
class Thing1 {
    String name
}
class Thing2 {
    String name
}

def t1 = new Thing1(name: 'Norman')
def t2 = new Thing2(name: 'Teapot')
def upperCasedName = { delegate.name.toUpperCase() }
upperCasedName.delegate = t1
assert upperCasedName() == 'NORMAN'
upperCasedName.delegate = t2
assert upperCasedName() == 'TEAPOT'
```

**代理**，我的理解是闭包运行的上下文，每个闭包都会有一个代理，默认是闭包本身（owner），这个代理可以设置成别的对象，然后就可以再闭包里面调用这个对象的方法了，`delegate` 也可以省略，`def upperCasedName = { name.toUpperCase() }` 也是可以的

仔细想想，就这个功能其实把 t1 当做参数传给 upperCaseName() 也是可以的，没必要整这么个复杂的概念，但是有意思的事情才刚刚开始，往下看你就知道了

### gradle 的 dependencies 原理

``` groovy
dependencies {
    testCompile group: 'junit', name: 'junit', version: '4.11'
    compile group: 'com.fasterxml.jackson.core', name: 'jackson-core', version: '2.9.4'
    compile group: 'com.fasterxml.jackson.core', name: 'jackson-databind', version: '2.9.4'
    compile group: 'com.fasterxml.jackson.core', name: 'jackson-annotations', version: '2.9.4'
}
```

在 `build.gradle` 里面有这么一段依赖管理的代码，那这段代码是怎么工作的呢，dependencies 看起来像是一个函数，而且参数是一个闭包，闭包里面有条语句，调用了几个函数，这些函数接受一个 Map 的参数，那这些函数在哪里呢，可以是全局的，也可以像下面这样，通过代理的方式传进去

``` groovy
class Dependency {
    void testCompile(Map map) {
        println("download dependency: " + map)
    }

    void compile(Map map) {
        println("download dependency: " + map)
    }
}

def dependencies(Closure cl) {
    def dependency = new Dependency()
    def code = cl.rehydrate(dependency, this, this)
    code()
}
```

`rehydrate` 的功能是 `clone` 一个闭包，第一个参数就是代理，将代理设置成一个 `Dependency` 对象后，就用调用代理对象的方法了

### 参考链接

- 官方文档：<http://www.groovy-lang.org/documentation.html#gettingstarted>
