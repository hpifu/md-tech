---
title: golang 单元测试
date: 2018-01-31 21:36:48
tags: [golang, 单元测试]
thumbnail: /img/thumbnail/zootopia1.jpg
---

单元测试是质量保证十分重要的一环，好的单元测试不仅能及时地发现问题，更能够方便地调试，提高生产效率，所以很多人认为写单元测试是需要额外的时间，会降低生产效率，是对单元测试最大的偏见和误解

go 语言原生支持了单元测试，使用上非常简单，测试代码只需要放到以 `_test.go` 结尾的文件中即可。golang的测试分为单元测试和性能测试，单元测试的测试用例以 `Test` 开头，性能测试以 `Benchmark` 开头

### 举个例子

实现排列组合函数对应的单元测试和性能测试

#### 实现排列组合函数

```
// combination.go

package hmath

func combination(m, n int) int {
    if n > m-n {
        n = m - n
    }

    c := 1
    for i := 0; i < n; i++ {
        c *= m - i
        c /= i + 1
    }

    return c
}
```

#### 实现单元测试和性能测试

```
// combination_test.go

package hmath

import (
    "math/rand"
    "testing"
)

// 单元测试
// 测试全局函数，以TestFunction命名
// 测试类成员函数，以TestClass_Function命名
func TestCombination(t *testing.T) {
    // 这里定义一个临时的结构体来存储测试case的参数以及期望的返回值
    for _, unit := range []struct {
        m        int
        n        int
        expected int
    }{
        {1, 0, 1},
        {4, 1, 4},
        {4, 2, 6},
        {4, 3, 4},
        {4, 4, 1},
        {10, 1, 10},
        {10, 3, 120},
        {10, 7, 120},
    } {
        // 调用排列组合函数，与期望的结果比对，如果不一致输出错误
        if actually := combination(unit.m, unit.n); actually != unit.expected {
            t.Errorf("combination: [%v], actually: [%v]", unit, actually)
        }
    }
}

// 性能测试
func BenchmarkCombination(b *testing.B) {
    // b.N会根据函数的运行时间取一个合适的值
    for i := 0; i < b.N; i++ {
        combination(i+1, rand.Intn(i+1))
    }
}

// 并发性能测试
func BenchmarkCombinationParallel(b *testing.B) {
    // 测试一个对象或者函数在多线程的场景下面是否安全
    b.RunParallel(func(pb *testing.PB) {
        for pb.Next() {
            m := rand.Intn(100) + 1
            n := rand.Intn(m)
            combination(m, n)
        }
    })
}
```

#### 运行测试

```
go test combination_test.go combination.go           # 单元测试
go test --cover combination_test.go combination.go   # 单元测试覆盖率
go test -bench=. combination_test.go combination.go  # 性能测试 cpu
go test -bench=. -benchmem combination_test.go combination.go   # 性能测试 cpu + 内存
```

#### setup 和 teardown

setup 和 teardown 是在每个 case 执行前后都需要执行的操作，golang 没有直接的实现，可以通过下面这个方法实现全局的 setup 和 teardown，具体每个 case 的 setup 和 teardown 需要自己实现

```
func TestMain(m *testing.M) {
    // setup code...
    os.Exit(m.Run())
    // teardown code...
}
```

### goconvey

这个第三方工具会自动帮我们跑测试，并且以非常友好的可视化界面帮我们展示测试的结果，包括测试失败的原因，测试覆盖率等等，内部还提供了很多友好的断言，能提高测试代码的可读性

#### 使用方法

```
go get github.com/smartystreets/goconvey
```

然后用终端在测试代码的目录下运行 `goconvey` 命令即可

#### 测试例子

```
package package_name

import (
    "testing"
    . "github.com/smartystreets/goconvey/convey"
)

func TestIntegerStuff(t *testing.T) {
    Convey("Given some integer with a starting value", t, func() {
        x := 1

        Convey("When the integer is incremented", func() {
            x++

            Convey("The value should be greater by one", func() {
                So(x, ShouldEqual, 2)
            })
        })
    })
}
```

### 参考链接

- go testing: <http://docs.studygolang.com/pkg/testing/>
- goconvey: <https://github.com/smartystreets/goconvey>
- goconvey 文档: <https://github.com/smartystreets/goconvey/wiki/Documentation>
- goconvey 标准断言: <https://github.com/smartystreets/goconvey/wiki/Assertions>
