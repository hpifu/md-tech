---
title: golang 字节对齐
date: 2018-03-17 00:20:52
tags: [golang, 字节对齐, 结构体, 性能优化, 内存优化]
---

最近在做一些性能优化的工作，其中有个结构体占用的空间比较大，而且在内存中的数量又特别多，就在想有没有优化的空间，想起了 c 语言里面的字节对齐，通过简单地调整一下字段的顺序，就能省出不少内存，这个思路在 golang 里面同样适用

### 基本数据大小

在这之前先来看下 golang 里面基本的类型所占数据的大小

``` golang
So(unsafe.Sizeof(true), ShouldEqual, 1)
So(unsafe.Sizeof(int8(0)), ShouldEqual, 1)
So(unsafe.Sizeof(int16(0)), ShouldEqual, 2)
So(unsafe.Sizeof(int32(0)), ShouldEqual, 4)
So(unsafe.Sizeof(int64(0)), ShouldEqual, 8)
So(unsafe.Sizeof(int(0)), ShouldEqual, 8)
So(unsafe.Sizeof(float32(0)), ShouldEqual, 4)
So(unsafe.Sizeof(float64(0)), ShouldEqual, 8)
So(unsafe.Sizeof(""), ShouldEqual, 16)
So(unsafe.Sizeof("hello world"), ShouldEqual, 16)
So(unsafe.Sizeof([]int{}), ShouldEqual, 24)
So(unsafe.Sizeof([]int{1, 2, 3}), ShouldEqual, 24)
So(unsafe.Sizeof([3]int{1, 2, 3}), ShouldEqual, 24)
So(unsafe.Sizeof(map[string]string{}), ShouldEqual, 8)
So(unsafe.Sizeof(map[string]string{"1": "one", "2": "two"}), ShouldEqual, 8)
So(unsafe.Sizeof(struct{}{}), ShouldEqual, 0)
```

- bool 类型虽然只有一位，但也需要占用1个字节，因为计算机是以字节为单位
- 64为的机器，一个 int 占8个字节
- string 类型占16个字节，内部包含一个指向数据的指针（8个字节）和一个 int 的长度（8个字节）
- slice 类型占24个字节，内部包含一个指向数据的指针（8个字节）和一个 int 的长度（8个字节）和一个 int 的容量（8个字节）
- map 类型占8个字节，是一个指向 map 结构的指针
- 可以用 struct{} 表示空类型，这个类型不占用任何空间，用这个作为 map 的 value，可以将 map 当做 set 来用

### 字节对齐

结构体中的各个字段在内存中并不是紧凑排列的，而是按照字节对齐的，比如 int 占8个字节，那么就只能写在地址为8的倍数的地址处，至于为什么要字节对齐，主要是为了效率考虑，而更深层的原理看了一下网上的说法，感觉不是很靠谱，就不瞎说了，感兴趣可以自己研究下

``` golang
// |x---|
So(unsafe.Sizeof(struct {
    i8 int8
}{}), ShouldEqual, 1)
```

简单封装一个 int8 的结构体，和 int8 一样，仅占1个字节，没有额外空间

``` golang
// |x---|xxxx|xx--|
So(unsafe.Sizeof(struct {
    i8  int8
    i32 int32
    i16 int16
}{}), ShouldEqual, 12)

// |x-xx|xxxx|
So(unsafe.Sizeof(struct {
    i8  int8
    i16 int16
    i32 int32
}{}), ShouldEqual, 8)
```

这两个结构体里面的内容完全一样，调整了一下字段顺序，节省了 33% 的空间

``` golang
// |x---|xxxx|xx--|----|xxxx|xxxx|
So(unsafe.Sizeof(struct {
    i8  int8
    i32 int32
    i16 int16
    i64 int64
}{}), ShouldEqual, 24)

// |x-xx|xxxx|xxxx|xxxx|
So(unsafe.Sizeof(struct {
    i8  int8
    i16 int16
    i32 int32
    i64 int64
}{}), ShouldEqual, 16)
```

这里需要注意的是 int64 只能出现在8的倍数的地址处，因此第一个结构体中，有连续的4个字节是空的

``` golang
type I8 int8
type I16 int16
type I32 int32

So(unsafe.Sizeof(struct {
    i8  I8
    i16 I16
    i32 I32
}{}), ShouldEqual, 8)
```

给类型重命名之后，类型的大小并没有发生改变

### 参考链接

- 测试代码链接：<https://github.com/hatlonely/hellogolang/blob/master/internal/buildin/struct_test.go>
