---
title: golang pprof 性能分析工具
date: 2018-01-29 21:29:49
tags: [golang, pprof, 性能]
---

性能优化是个永恒的话题，而很多时候我们在作性能优化的时候，往往基于代码上面的直觉，把所有能想到的优化都优化了一遍，不错过任何小的优化点，结果整个代码的逻辑变得极其复杂，而性能上面并没有太大的提升。事实上，性能问题往往集中在某些小点，有时候很小的改动就能有巨大的提升，所以问题的关键是是怎么去找出这些优化点，幸运的是 golang 在设计的时候就考虑了这个问题，原生提供了性能分析的工具，可以很方便地帮我们找到性能瓶颈

### pprof 简介

golang 的性能分析库在 `runtime/pprof` 里，主要提供下面几个接口

```golang
// 堆栈分析
func WriteHeapProfile(w io.Writer) error
// cpu分析
func StartCPUProfile(w io.Writer) error
func StopCPUProfile()
```

使用上面比较简单，只需要将文件指针传给对应的函数即可，性能数据将写入到文件中，然后可以使用 golang 自带的 pprof 工具生成 svg，pdf 的可视化图，然后就可以很直观地从这些图里面看到主要的性能消耗了

### 举个例子

#### 首先需要一个程序

首先需要在你的程序里面注入 pprof 代码，下面是一段示例代码，完整代码在：<https://github.com/hatlonely/hellogolang/blob/master/cmd/pprof/pprof_runtime.go>，这里使用的 `PPCmd` 方法，是为了方便使用，做的一个简单封装，代码在：<https://github.com/hatlonely/easygolang/blob/master/pprof/pprof.go>

```golang
func main() {
    go doSomething1()
    go doSomething2()
    go doSomething3()

    if err := pprof.PPCmd("cpu 10s"); err != nil {
        panic(err)
    }

    if err := pprof.PPCmd("mem"); err != nil {
        panic(err)
    }
}
```

编译，运行上面代码会生成两个 pprof 文件，`cpu.pprof.yyyymmddhhmmss` 和 `mem.pprof.yyyymmddhhmmss`，编译运行的方法如下：

```
cd $GOPATH/src
git clone git@github.com:hatlonely/hellogolang.git
cd hellogolang
glide install
go build cmd/pprof_runtime.go
./pprof_runtime
```

#### pprof 文件分析

pprof 文件是二进制的，不是给人读的，需要翻译一下，而 golang 原生就给我们提供了分析工具，直接执行下面命令即可，会生成一张很直观的 svg 图片，直接用 chrome 就可以打开，当然也可以生成别的格式（pdf，png 都可以），可以用 `go tool pprof -h` 命令查看支持的输出类型

```
go tool pprof -svg ./pprof_runtime cpu.pprof.201801301415 > cpu.svg
```

注意这个工具依赖于 graphviz 工具，Mac 上可用 `brew install graphviz`，centos `yum install graphviz` 即可

![性能分析图](/img/pprof_runtime_cpu.png)

### http 接口

`net/http/pprof` 里面对 `runtime/pprof` 作了一些封装，对外提供了 http 接口，可以直接通过浏览器访问，但是只是一些字符串的结果，没有作可视化，体验并不是很好，用 `go tool` 访问体验能好一点

```
go tool pprof http://localhost:3000/debug/pprof/profile
go tool pprof http://localhost:3000/debug/pprof/heap
```

个人感觉这个接口比较鸡肋，首先最大的问题是展示上面并不直观，要是能直接在网页上面可视化地展示可能还真的挺方便的；还有就是需要额外的提供一个 http 的端口，而这个接口还依赖 `net/http`这就意味着如果你的应用使用的是其他第三方的 http 库，可能还需要解决兼容性的问题；实际上，我再使用这个接口的时候，在服务器压力较大的场景下，会出现访问超时，而这种压力较大情况下的性能可能才是真正的性能瓶颈。

建议在根据的需求，自己封装 `runtime/pprof` 的接口，当然是用场景比较简单也可以用我上面的封装，然后在服务里面自己提供一个专门的性能分析接口（可能是 gprc，thrift，或者其他的第三方 http 框架）

### 火焰图

除了上面生成的 svg 图，还可以生成火焰图，这是 uber 提供的一个工具，在显示上面可能更直观一些

安装命令如下：

```
go get github.com/uber/go-torch
git clone git@github.com:brendangregg/FlameGraph.git
export PATH=$PATH:/path/to/FlameGraph
```

使用方法如下：

```
go-torch --binaryname=./pprof_runtime --binaryinput=cpu.pprof.201801301415
```

![性能分析图](/img/pprof_runtime_cpu_torch.svg)

### 参考链接

- Package pprof: <https://golang.org/pkg/runtime/pprof/>
- Profiling Go Programs: <https://blog.golang.org/profiling-go-programs>
- Go torch: <https://github.com/uber/go-torch>
