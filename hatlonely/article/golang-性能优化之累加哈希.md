---
title: golang 性能优化之累加哈希
date: 2018-04-12 17:01:28
tags: [golang, 性能优化, hash, 数据结构]
---

很多时候性能问题总是发生在一些不起眼的地方。最近做一个性能问题分析的时候发现，一个函数里面使用由于字符串拼接产生的临时字符串导致内存上涨了40%（120G 内存的机器），而这些临时字符串给 GC 也带来了非常大的负担，成为主要的性能瓶颈，而这些字符串作为 map 的 key，又必须要拼接，所以想到了直接使用 hash 后的值作为 map 的 key，而这个 hash 值使用累加 hash 计算得出。

所谓累加 hash，就是对字符串的 hash 可以分为任意多段，对每一段连续 hash，结果累加，对于任意一种分段方式，最后能得到一致的 hash 结果，比如：`H.hash("hello world")`, `H.hash("hello").hash(" ").hash("world")`, `H.hash("hello wo").hash("rld)"` 这些结果最后都应该是一致的，利用这个特性，就能做到对多个字符串哈希而不用拼接

### BKDR hash 实现

``` golang
type StringHasherBKDR uint64

// NewStringHasherBKDR 创建一个新的 Hasher
func NewStringHasherBKDR() StringHasherBKDR {
	return StringHasherBKDR(0)
}

// AddStr 增加一个字符串
func (bkdr StringHasherBKDR) AddStr(str string) StringHasherBKDR {
	val := uint64(bkdr)
	for i := 0; i < len(str); i++ {
		val = val*131 + uint64(str[i])
	}
	return StringHasherBKDR(val)
}

// AddInt 添加一个 int 值
func (bkdr StringHasherBKDR) AddInt(i uint64) StringHasherBKDR {
	val := uint64(bkdr)
	val = val*131 + i
	return StringHasherBKDR(val)
}

// Val 转成 uint64 的值
func (bkdr StringHasherBKDR) Val() uint64 {
	return uint64(bkdr)
}
```

使用上也很简单

```
hasher := NewStringHasherBKDR()
So(hasher.AddStr("hello world").Val(), ShouldEqual, hasher.AddStr("hello").AddStr(" ").AddStr("world").Val())
```

### 参考链接

- 代码地址: <https://github.com/hatlonely/easygolang/blob/master/hashopt/>

