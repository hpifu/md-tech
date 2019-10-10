---
title: golang 性能优化之 bitset 代替 hashset
date: 2018-04-12 15:04:28
tags: [golang, bitset, hashset, set, 性能优化, 数据结构]
---

hashset 是一种非常高效的数据结构，插入和查询的复杂度都是 O(1)，基本上能满足大部分场景的性能需求，但在一些特殊的场景下，频次非常高的调用依然会成为性能瓶颈（用 pprof 分析），比如广告里面的定向逻辑，在一次请求中过滤逻辑可能会执行上千次，而其中有些过滤刚好都是一些枚举值，比如性别定向，年龄定向等等，对于这种可以用枚举表示的值可以用 bitset 优化，能有20多倍的性能提升

bitset 的本质也是一种 hashset，只不过哈希桶用一个 uint64 来表示了，uint64 中的每一位用来代表一个元素是否存在，如果为1表示存在，为0表示不存在，而插入和查询操作就变成了位运算

### bitset 实现

bitset 的实现比较容易，下面这个是一个只支持枚举值不超过64的版本，当然也可以拓展到任意长度，使用一个 uint64 数组作为 hash 桶即可

``` golang
type BitSet struct {
	bit uint64
}

func (bs *BitSet) Add(i uint64) {
	bs.bit |= 1 << i
}

func (bs *BitSet) Del(i uint64) {
	bs.bit &= ^(1 << i)
}

func (bs BitSet) Has(i uint64) bool {
	return bs.bit&(1<<i) != 0
}
```

### 性能测试

``` golang
func BenchmarkSetContains(b *testing.B) {
	bitset := NewBitSet()
	hashset := map[uint64]struct{}{}
	for _, i := range []uint64{1, 2, 4, 10} {
		bitset.Add(i)
		hashset[i] = struct{}{}
	}

	b.Run("bitset", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			for i := uint64(0); i < uint64(10); i++ {
				_ = bitset.Has(i)
			}
		}
	})

	b.Run("hashset", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			for i := uint64(0); i < uint64(10); i++ {
				_, _ = hashset[i]
			}
		}
	})
}
```

```
BenchmarkSetContains/bitset-8         	500000000	         3.81 ns/op	       0 B/op	       0 allocs/op
BenchmarkSetContains/hashset-8        	20000000	        89.4 ns/op	       0 B/op	       0 allocs/op
```

可以看到 bitset 相比 hashset 有20多倍的性能提升

### 参考链接

- 代码地址：<https://github.com/hatlonely/easygolang/blob/master/datastruct/bitset.go>
