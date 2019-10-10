---
title: golang 几种字符串的连接方式
date: 2018-01-24 13:45:55
tags: [golang, string]
---

最近在做性能优化，有个函数里面的耗时特别长，看里面的操作大多是一些字符串拼接的操作，而字符串拼接在 golang 里面其实有很多种实现。

### 实现方法

#### 1. 直接使用运算符

```golang
func BenchmarkAddStringWithOperator(b *testing.B) {
    hello := "hello"
    world := "world"
    for i := 0; i < b.N; i++ {
        _ = hello + "," + world
    }
}

func BenchmarkAddMoreStringWithOperator(b *testing.B) {
	hello := "hello"
	world := "world"
	for i := 0; i < b.N; i++ {
		var str string
		for i := 0; i < 100; i++ {
			str += hello + "," + world
		}
	}
}
```

golang 里面的字符串都是不可变的，每次运算都会产生一个新的字符串，所以会产生很多临时的无用的字符串，不仅没有用，还会给 gc 带来额外的负担，所以性能比较差

#### 2. fmt.Sprintf()

```golang
func BenchmarkAddStringWithSprintf(b *testing.B) {
    hello := "hello"
    world := "world"
    for i := 0; i < b.N; i++ {
        _ = fmt.Sprintf("%s,%s", hello, world)
    }
}
```

内部使用 `[]byte` 实现，不像直接运算符这种会产生很多临时的字符串，但是内部的逻辑比较复杂，有很多额外的判断，还用到了 `interface`，所以性能也不是很好

#### 3. strings.Join()

```golang
func BenchmarkAddStringWithJoin(b *testing.B) {
    hello := "hello"
    world := "world"
    for i := 0; i < b.N; i++ {
        _ = strings.Join([]string{hello, world}, ",")
    }
}
```

join会先根据字符串数组的内容，计算出一个拼接之后的长度，然后申请对应大小的内存，一个一个字符串填入，在已有一个数组的情况下，这种效率会很高，但是本来没有，去构造这个数据的代价也不小

#### 4. buffer.WriteString()

```golang
func BenchmarkAddStringWithBuffer(b *testing.B) {
    hello := "hello"
    world := "world"
    for i := 0; i < b.N; i++ {
        var buffer bytes.Buffer
        buffer.WriteString(hello)
        buffer.WriteString(",")
        buffer.WriteString(world)
        _ = buffer.String()
    }
}

func BenchmarkAddMoreStringWithBuffer(b *testing.B) {
	hello := "hello"
	world := "world"
	for i := 0; i < b.N; i++ {
		var buffer bytes.Buffer
		for i := 0; i < 100; i++ {
			buffer.WriteString(hello)
			buffer.WriteString(",")
			buffer.WriteString(world)
		}
		_ = buffer.String()
	}
}
```

这个比较理想，可以当成可变字符使用，对内存的增长也有优化，如果能预估字符串的长度，还可以用 `buffer.Grow()` 接口来设置 capacity

### 测试结果

```
BenchmarkAddStringWithOperator-8                50000000                28.4 ns/op             0 B/op          0 allocs/op
BenchmarkAddStringWithSprintf-8                 10000000               234 ns/op              48 B/op          3 allocs/op
BenchmarkAddStringWithJoin-8                    30000000                56.2 ns/op            16 B/op          1 allocs/op
BenchmarkAddStringWithBuffer-8                  20000000                86.0 ns/op           112 B/op          1 allocs/op
BenchmarkAddMoreStringWithOperator-8              100000             14295 ns/op           58896 B/op        100 allocs/op
BenchmarkAddMoreStringWithBuffer-8                300000              4551 ns/op            5728 B/op          7 allocs/op
```

这个是在我的自己 Mac 上面跑的结果，go 版本 `go version go1.8 darwin/amd64`，这个结果仅供参考，还是要以实际生产环境的值为准，代码在：<https://github.com/hatlonely/hellogolang/blob/master/internal/buildin/string_test.go>

### 主要结论

1. 在已有字符串数组的场合，使用 `strings.Join()` 能有比较好的性能
2. 在一些性能要求较高的场合，尽量使用 `buffer.WriteString()` 以获得更好的性能
3. 较少字符串连接的场景下性能最好，而且代码更简短清晰，可读性更好
4. 如果需要拼接的不仅仅是字符串，还有数字之类的其他需求的话，可以考虑 `fmt.Sprintf`

### 参考链接

go语言字符串拼接性能分析: <http://herman.asia/efficient-string-concatenation-in-go>
