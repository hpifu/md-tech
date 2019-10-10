---
title: c++ map 查找性能测试
date: 2018-08-01 18:07:11
tags: [c++, map, 性能]
thumbnail: /img/thumbnail/city_chongqing.jpg
---

最近在为推荐服务作性能调优，这个服务的主要逻辑是用离线计算的模型数据给请求中的每个广告打分，再返回这些广告的排序结果，这里面打分的过程其实就用请求中的数据拼成各种key，去查一个大的 map，这种计算非常多，成为了主要的性能瓶颈，代码比较老，使用的是 boost::unordered_map，为了解决这个问题，找了一些第三方库和标准库对比了一下

下面是在一台 aws `r4.xlarge` 机器上的测试结果(注意编译的时候一定要加 -O2)：

```
std::map<int, int>                                 => 51866903
std::unordered_map<int, int>                       => 3838175
std::unordered_map<int, int, nohashint>            => 3508570
std::unordered_map<int, int>(N)                    => 3804471
boost::unordered_map<int, int>                     => 3291384
boost::unordered_map<int, int, nohashint>          => 3293934
boost::unordered_map<int, int>(N)                  => 3265856
google::dense_hash_map<int, int>                   => 785969
google::dense_hash_map<int, int, nohashint>        => 784455
google::dense_hash_map<int, int>(N)                => 899262
tsl::hopscotch_map<int, int>                       => 654668
tsl::hopscotch_map<int, int, nohashint>            => 680964
tsl::hopscotch_map<int, int>(N)                    => 663607
tsl::robin_map<int, int>                           => 406176
tsl::robin_map<int, int, nohashint>                => 411358
tsl::robin_map<int, int>(N)                        => 409993
```

可以看到 tsl::robin_map 的性能基本上能达到 std::unordered_map 的 10 倍，这个性能和操作系统以及库版本也有一定关系，实际生产环境中建议把[代码](https://github.com/hatlonely/hellocpp/blob/master/src/unordered_map/test_unordered_map.cpp)拉下来在自己的环境下测试一下

我们线上用 tsl::robin_map 替换了原来的 boost::unordered_map，整体性能提升了 5 倍，这里面当然也还包含了一些其他的优化，这个优化算是比较大的优化点了

### 参考链接

- Benchmark of major hash maps implementations:
  <https://tessil.github.io/2016/08/29/benchmark-hopscotch-map.html>
- 测试代码: <https://github.com/hatlonely/hellocpp/blob/master/src/unordered_map/test_unordered_map.cpp>
