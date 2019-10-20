---
title: golang slice 性能分析
date: 2018-01-18 21:56:24
tags: [golang, slice, 性能]
avatar: hatlonely13.png
---

golang 在 gc 这块的做得比较弱，频繁地申请和释放内存会消耗很多的资源。另外 slice 使用数组实现，有一个容量和长度的问题，当 slice 的容量用完再继续添加元素时需要扩容，而这个扩容会把申请新的空间，把老的内容复制到新的空间，这是一个非常耗时的操作。有两种方式可以减少这个问题带来的性能开销：

1. 在 slice 初始化的时候设置 capacity（但更多的时候我们可能并不知道 capacity 的大小）
2. 复用 slice

下面就针对这两个优化设计了如下的benchmark，代码在: <https://github.com/hatlonely/hellogolang/blob/master/internal/buildin/slice_test.go>

```
BenchmarkAppendWithoutCapacity-8                     100      21442390 ns/op
BenchmarkAppendWithCapLessLen10th-8                  100      18579700 ns/op
BenchmarkAppendWithCapLessLen3th-8                   100      13867060 ns/op
BenchmarkAppendWithCapEqualLen-8                     200       6287940 ns/op
BenchmarkAppendWithCapGreaterLen10th-8               100      18692880 ns/op
BenchmarkAppendWithoutCapacityReuse-8                300       5014320 ns/op
BenchmarkAppendWithCapEqualLenReuse-8                300       4821420 ns/op
BenchmarkAppendWithCapGreaterLen10thReuse-8          300       4903230 ns/op
```

### 主要结论

1. 在已知 capacity 的情况下，直接设置 capacity 减少内存的重新分配，有效提高性能
2. capacity < length，capacity 越接近 length，性能越好
3. capacity > length，如果太大，反而会造成性能下降，这里当 capacity > 10 * length时，与不设置 capacity 的性能差不太多
4. 多次使用复用同一块内存能有效提高性能

### 测试代码

```go
func BenchmarkAppendWithoutCapacity(b *testing.B) {
    for i := 0; i < b.N; i++ {
        var arr []int
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithCapLessLen10th(b *testing.B) {
    for i := 0; i < b.N; i++ {
        arr := make([]int, 0, N/10)
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithCapLessLen3th(b *testing.B) {
    for i := 0; i < b.N; i++ {
        arr := make([]int, 0, N/3)
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithCapEqualLen(b *testing.B) {
    for i := 0; i < b.N; i++ {
        arr := make([]int, 0, N)
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithCapGreaterLen10th(b *testing.B) {
    for i := 0; i < b.N; i++ {
        arr := make([]int, 0, N*10)
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithoutCapacityReuse(b *testing.B) {
    var arr []int
    for i := 0; i < b.N; i++ {
        arr = arr[:0]
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithCapEqualLenReuse(b *testing.B) {
    arr := make([]int, N)
    for i := 0; i < b.N; i++ {
        arr = arr[:0]
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}

func BenchmarkAppendWithCapGreaterLen10thReuse(b *testing.B) {
    arr := make([]int, N*10)
    for i := 0; i < b.N; i++ {
        arr = arr[:0]
        for i := 0; i < N; i++ {
            arr = append(arr, i)
        }
    }
}
```
