---
title: golang json 性能分析
date: 2018-01-28 13:42:19
tags: [golang, json, 性能]
---

Json 作为一种重要的数据格式，具有良好的可读性以及自描述性，广泛地应用在各种数据传输场景中。Go 语言里面原生支持了这种数据格式的序列化以及反序列化，内部使用反射机制实现，性能有点差，在高度依赖 json 解析的应用里，往往会成为性能瓶颈，好在已有很多第三方库帮我们解决了这个问题，但是这么多库，对于像我这种有选择困难症的人来说，到底要怎么选择呢，下面就给大家来一一分析一下

### ffjson

```
go get -u github.com/pquerna/ffjson
```

原生的库性能比较差的主要原因是使用了很多反射的机制，为了解决这个问题，ffjson 通过预编译生成代码，类型的判断在预编译阶段已经确定，避免了在运行时的反射

但也因此在编译前需要多一个步骤，需要先生成 ffjson 代码，生成代码只需要执行 `ffjson <file.go>` 就可以了，其中 `file.go` 是一个包含 json 结构体定义的 go 文件。注意这里 ffjson 是这个库提供的一个代码生成工具，直接执行上面的 `go get` 会把这个工具安装在 `$GOPATH/bin` 目录下，把 `$GOPATH/bin` 加到 `$PATH` 环境变量里面，可以全局访问

另外，如果有些结构，不想让 ffjson 生成代码，可以通过增加注释的方式

```golang
// ffjson: skip
type Foo struct {
   Bar string
}

// ffjson: nodecoder
type Foo struct {
   Bar string
}
```

### easyjson

```
go get -u github.com/mailru/easyjson/...
```

easyjson 的思想和 ffjson 是一致的，都是增加一个预编译的过程，预先生成对应结构的序列化反序列化代码，除此之外，easyjson 还放弃了一些原生库里面支持的一些不必要的特性，比如：key 类型声明，key 大小写不敏感等等，以达到更高的性能

生成代码执行 `easyjson -all <file.go>` 即可，如果不指定 `-all` 参数，只会对带有 `//easyjson:json` 的结构生成代码

``` golang
//easyjson:json
type A struct {
    Bar string
}
```

### jsoniter

```
go get -u github.com/json-iterator/go
```

这是一个很神奇的库，滴滴开发的，不像 easyjson 和 ffjson 都使用了预编译，而且 100% 兼容原生库，但是性能超级好，也不知道怎么实现的，如果有人知道的话，可以告诉我一下吗？

> 2018-1-28日更新，来自官方（@taowen）的回复：
> 没啥神奇的。就是预先缓存了对应struct的decoder实例而已。然后unsafe.Pointer省掉了一些interface{}的开销。还有一些文本解析上的优化

使用上面，你只要把所有的 

```
import "encoding/json"
```

替换成

```
import "github.com/json-iterator/go"

var json = jsoniter.ConfigCompatibleWithStandardLibrary
```

就可以了，其它都不需要动

### codec-json

```
go get -u github.com/ugorji/go/codec
```

这个库里面其实包含很多内容，json 只是其中的一个功能，比较老，使用起来比较麻烦，性能也不是很好

### jsonparser

```
go get -u github.com/buger/jsonparser
```

严格来说，这个库不属于 json 序列化的库，只是提供了一些 json 解析的接口，使用的时候需要自己去设置结构里面的值，事实上，每次调用都需要重新解析 json 对象，性能并不是很好

就像名字暗示的那样，这个库只是一个解析库，并没有序列化的接口

### 性能测试

对上面这些 json 库，作了一些性能测试，测试代码在：<https://github.com/hatlonely/hellogolang/blob/master/internal/json/json_benchmark_test.go>，下面是在我的 Macbook 上测试的结果（实际结果和库的版本以及机器环境有关，建议自己再测试一遍）：

```
BenchmarkMarshalStdJson-4                    1000000          1097 ns/op
BenchmarkMarshalJsonIterator-4               2000000           781 ns/op
BenchmarkMarshalFfjson-4                     2000000           941 ns/op
BenchmarkMarshalEasyjson-4                   3000000           513 ns/op
BenchmarkMarshalCodecJson-4                  1000000          1074 ns/op
BenchmarkMarshalCodecJsonWithBufio-4         1000000          2161 ns/op
BenchmarkUnMarshalStdJson-4                   500000          2512 ns/op
BenchmarkUnMarshalJsonIterator-4             2000000           591 ns/op
BenchmarkUnMarshalFfjson-4                   1000000          1127 ns/op
BenchmarkUnMarshalEasyjson-4                 2000000           608 ns/op
BenchmarkUnMarshalCodecJson-4                  20000        122694 ns/op
BenchmarkUnMarshalCodecJsonWithBufio-4        500000          3417 ns/op
BenchmarkUnMarshalJsonparser-4               2000000           877 ns/op
```

![golang_json_performance](/img/golang_json_performance.png)

从上面的结果可以看出来：

1. easyjson 无论是序列化还是反序列化都是最优的，序列化提升了1倍，反序列化提升了3倍
2. jsoniter 性能也很好，接近于easyjson，关键是没有预编译过程，100%兼容原生库
3. ffjson 的序列化提升并不明显，反序列化提升了1倍
4. codecjson 和原生库相比，差不太多，甚至更差
5. jsonparser 不太适合这样的场景，性能提升并不明显，而且没有反序列化

**所以综合考虑，建议大家使用 jsoniter，如果追求极致的性能，考虑 easyjson**

### 参考链接

- ffjson: <https://github.com/pquerna/ffjson>
- easyjson: <https://github.com/mailru/easyjson>
- jsoniter: <https://github.com/json-iterator/go>
- jsonparser: <https://github.com/buger/jsonparser>
- codecjson: <http://ugorji.net/blog/go-codec-primer>
