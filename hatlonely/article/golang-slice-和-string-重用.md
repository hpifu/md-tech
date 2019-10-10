---
title: golang slice 和 string 重用
date: 2018-03-17 15:34:40
tags: [golang, slice, string, 重用, 性能优化]
thumbnail: /img/thumbnail/cloud.jpg
---

相比于 c/c++，golang 的一个很大的改进就是引入了 gc 机制，不再需要用户自己管理内存，大大减少了程序由于内存泄露而引入的 bug，但是同时 gc 也带来了额外的性能开销，有时甚至会因为使用不当，导致 gc 成为性能瓶颈，所以 golang 程序设计的时候，应特别注意对象的重用，以减少 gc 的压力。而 slice 和 string 是 golang 的基本类型，了解这些基本类型的内部机制，有助于我们更好地重用这些对象

### slice 和 string 内部结构

slice 和 string 的内部结构可以在 `$GOROOT/src/reflect/value.go` 里面找到

``` golang
type StringHeader struct {
	Data uintptr
	Len  int
}

type SliceHeader struct {
	Data uintptr
	Len  int
	Cap  int
}
```

可以看到一个 string 包含一个数据指针和一个长度，长度是不可变的

slice 包含一个数据指针、一个长度和一个容量，当容量不够时会重新申请新的内存，Data 指针将指向新的地址，原来的地址空间将被释放

从这些结构就可以看出，string 和 slice 的赋值，包括当做参数传递，和自定义的结构体一样，都仅仅是 Data 指针的浅拷贝

### slice 重用

#### append 操作

``` golang
si1 := []int{1, 2, 3, 4, 5, 6, 7, 8, 9}
si2 := si1
si2 = append(si2, 0)
Convey("重新分配内存", func() {
    header1 := (*reflect.SliceHeader)(unsafe.Pointer(&si1))
    header2 := (*reflect.SliceHeader)(unsafe.Pointer(&si2))
    fmt.Println(header1.Data)
    fmt.Println(header2.Data)
    So(header1.Data, ShouldNotEqual, header2.Data)
})
```

si1 和 si2 开始都指向同一个数组，当对 si2 执行 append 操作时，由于原来的 Cap 值不够了，需要重新申请新的空间，因此 Data 值发生了变化，在 `$GOROOT/src/reflect/value.go` 这个文件里面还有关于新的 cap 值的策略，在 `grow` 这个函数里面，当 cap 小于 1024 的时候，是成倍的增长，超过的时候，每次增长 25%，而这种内存增长不仅仅数据拷贝（从旧的地址拷贝到新的地址）需要消耗额外的性能，旧地址内存的释放对 gc 也会造成额外的负担，所以如果能够知道数据的长度的情况下，尽量使用 `make([]int, len, cap)` 预分配内存，不知道长度的情况下，可以考虑下面的内存重用的方法

#### 内存重用

``` golang
si1 := []int{1, 2, 3, 4, 5, 6, 7, 8, 9}
si2 := si1[:7]
Convey("不重新分配内存", func() {
    header1 := (*reflect.SliceHeader)(unsafe.Pointer(&si1))
    header2 := (*reflect.SliceHeader)(unsafe.Pointer(&si2))
    fmt.Println(header1.Data)
    fmt.Println(header2.Data)
    So(header1.Data, ShouldEqual, header2.Data)
})

Convey("往切片里面 append 一个值", func() {
    si2 = append(si2, 10)
    Convey("改变了原 slice 的值", func() {
        header1 := (*reflect.SliceHeader)(unsafe.Pointer(&si1))
        header2 := (*reflect.SliceHeader)(unsafe.Pointer(&si2))
        fmt.Println(header1.Data)
        fmt.Println(header2.Data)
        So(header1.Data, ShouldEqual, header2.Data)
        So(si1[7], ShouldEqual, 10)
    })
})
```

si2 是 si1 的一个切片，从第一段代码可以看到切片并不重新分配内存，si2 和 si1 的 Data 指针指向同一片地址，而第二段代码可以看出，当我们往 si2 里面 append 一个新的值的时候，我们发现仍然没有内存分配，而且这个操作使得 si1 的值也发生了改变，因为两者本就是指向同一片 Data 区域，利用这个特性，我们只需要让 `si1 = si1[:0]` 就可以不断地清空 si1 的内容，实现内存的复用了

**PS**: 你可以使用 `copy(si2, si1)` 实现深拷贝

### string

``` golang
Convey("字符串常量", func() {
    str1 := "hello world"
    str2 := "hello world"
    Convey("地址相同", func() {
        header1 := (*reflect.StringHeader)(unsafe.Pointer(&str1))
        header2 := (*reflect.StringHeader)(unsafe.Pointer(&str2))
        fmt.Println(header1.Data)
        fmt.Println(header2.Data)
        So(header1.Data, ShouldEqual, header2.Data)
    })
})
```

这个例子比较简单，字符串常量使用的是同一片地址区域

``` golang
Convey("相同字符串的不同子串", func() {
    str1 := "hello world"[:6]
    str2 := "hello world"[:5]
    Convey("地址相同", func() {
        header1 := (*reflect.StringHeader)(unsafe.Pointer(&str1))
        header2 := (*reflect.StringHeader)(unsafe.Pointer(&str2))
        fmt.Println(header1.Data, str1)
        fmt.Println(header2.Data, str2)
        So(str1, ShouldNotEqual, str2)
        So(header1.Data, ShouldEqual, header2.Data)
    })
})
```

相同字符串的不同子串，不会额外申请新的内存，但是要注意的是这里的相同字符串，指的是 `str1.Data == str2.Data && str1.Len == str2.Len`，而不是 `str1 == str2`，下面这个例子可以说明 `str1 == str2` 但是其 Data 并不相同

``` golang
Convey("不同字符串的相同子串", func() {
    str1 := "hello world"[:5]
    str2 := "hello golang"[:5]
    Convey("地址不同", func() {
        header1 := (*reflect.StringHeader)(unsafe.Pointer(&str1))
        header2 := (*reflect.StringHeader)(unsafe.Pointer(&str2))
        fmt.Println(header1.Data, str1)
        fmt.Println(header2.Data, str2)
        So(str1, ShouldEqual, str2)
        So(header1.Data, ShouldNotEqual, header2.Data)
    })
})
```

实际上对于字符串，你只需要记住一点，字符串是不可变的，任何字符串的操作都不会申请额外的内存（对于仅内部数据指针而言），我曾自作聪明地设计了一个 cache 去存储字符串，以减少重复字符串所占用的空间，事实上，除非这个字符串本身就是由 `[]byte` 创建而来，否则，这个字符串本身就是另一个字符串的子串（比如通过 `strings.Split` 获得的字符串），本来就不会申请额外的空间，这么做简直就是多此一举

### 参考链接

- Go Slices: usage and internals：<https://blog.golang.org/go-slices-usage-and-internals>
- 测试代码链接：<https://github.com/hatlonely/hellogolang/blob/master/internal/buildin/reuse_test.go>