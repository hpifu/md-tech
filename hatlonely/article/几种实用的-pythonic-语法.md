---
title: 几种实用的 pythonic 语法
date: 2018-02-04 20:02:22
tags: [python, pythonic, 语法糖]
thumbnail: /img/thumbnail/my_fair_lady.jpg
---

python 是一门简单而优雅的语言，可能是过于简单了，不用花太多时间学习就能使用，其实 python 里面还有一些很好的特性，能大大简化你代码的逻辑，提高代码的可读性

关于 pythonic，你可以在终端打开 python，然后输入 `import this`，看看输出什么，这就是 Tim Peters 的 [《The Zen of Python》](https://www.python.org/dev/peps/pep-0020/)，这首充满诗意的诗篇里概况了 python 的设计哲学，而这些思想，其实在所有语言也基本上是通用的

> Beautiful is better than ugly.
> Explicit is better than implicit.
> Simple is better than complex.
> Complex is better than complicated.
> Flat is better than nested.
> Sparse is better than dense.
> Readability counts.
> Special cases aren't special enough to break the rules.
> Although practicality beats purity.
> Errors should never pass silently.
> Unless explicitly silenced.
> In the face of ambiguity, refuse the temptation to guess.
> There should be one-- and preferably only one --obvious way to do it.
> Although that way may not be obvious at first unless you're Dutch.
> Now is better than never.
> Although never is often better than \*right\* now.
> If the implementation is hard to explain, it's a bad idea.
> If the implementation is easy to explain, it may be a good idea.
> Namespaces are one honking great idea -- let's do more of those!

### 使用生成器 yield

生成器是 python 里面一个非常有用的语法特性，却也是最容易被忽视的一个，可能是因为大部分能用生成器的地方也能用列表吧。

生成器可以简单理解成一个函数，每次执行到 yield 语句就返回一个值，通过不停地调用这个函数，就能获取到所有的值，这些值就能构成了一个等效的列表，但是与列表不同的是，这些值是不断计算得出，而列表是在一开始就计算好了，这就是 lazy evaluation 的思想。这个特性在数据量特别大的场景非常有用，比如大数据处理，一次无法加载所有的文件，使用生成器就能做到一行一行处理，而不用担心内存溢出

``` python
def fibonacci():
    num0 = 0
    num1 = 1
    for i in range(10):
        num2 = num0 + num1
        yield num2
        num0 = num1
        num1 = num2

for i in fibonacci():
    print(i)
```

### 用 else 子句简化循环和异常

`if / else` 大家都用过，但是在 python 里面，`else` 还可以用在循环和异常里面

``` python
# pythonic 写法
for cc in ['UK', 'ID', 'JP', 'US']:
    if cc == 'CN':
        break
else:
    print('no CN')

# 一般写法
no_cn = True
for cc in ['UK', 'ID', 'JP', 'US']:
    if cc == 'CN':
        no_cn = False
        break
if no_cn:
    print('no CN')
```

`else` 放在循环里面的含义是，如果循环全部遍历完成，没有执行 `break`，则执行 `else` 子句

``` python
# pythonic 写法
try:
    db.execute('UPDATE table SET xx=xx WHERE yy=yy')
except DBError:
    db.rollback()
else:
    db.commit()

# 一般写法
has_error = False
try:
    db.execute('UPDATE table SET xx=xx WHERE yy=yy')
except DBError:
    db.rollback()
    has_error = True
if not has_error:
    db.commit()
```

`else` 放到异常里面可以表示，如果没有异常发生需要执行的操作

### 用 with 子句自动管理资源

我们都知道，打开的文件需要在用完之后关闭，要不就会造成资源泄露，但是实际编程的时候经常会忘记关闭，特别是在一些逻辑复杂的场景中，更是如此，python 有一个优雅地解决方案，那就是 `with` 子句

``` python
# pythonic 写法
with open('pythonic.py') as fp:
    for line in fp:
        print(line[:-1])

# 一般写法
fp = open('pythonic.py')
for line in fp:
    print(line[:-1])
fp.close()
```

使用 `with as` 语句后，无需手动调用 `fp.close()`, 在作用域结束后，文件会被自动 close 掉，完整的执行过如下:

1. 调用 `open('pythonic.py')`，返回的一个对象 `obj`,
2. 调用 `obj.__enter__()` 方法，返回的值赋给 `fp`
3. 执行 `with` 中的代码块
4. 执行 `obj.__exit__()`
5. 如果这个过程发生异常，将异常传给 `obj.__exit__()`，如果 `obj.__exit__()` 返回 `False`, 异常将被继续抛出，如果返回 `True`，异常被挂起，程序继续运行

### 列表推导与生成器表达式

> 列表推导
> `[expr for iter_var in iterable if cond_expr]`
>
> 生成器表达式
> `(expr for iter_var in iterable if cond_expr)`

列表推导和生成器表达式提供了一种非常简洁高效的方式来创建列表或者迭代器

``` python
# pythonic 写法
squares = [x * x for x in range(10)]

# 一般写法
squares = []
for x in range(10):
    squares.append(x * x)
```

### 用 items 遍历 map

python 里面 map 的遍历有很多种方式，在需要同事使用 key 和 value 的场合，建议使用 `items()` 函数

``` python
m = {'one': 1, 'two': 2, 'three': 3}
for k, v in m.items():
    print(k, v)

for k, v in sorted(m.items()):
    print(k, v)
```

### 参考链接

- Google python 语言规范: <http://zh-google-styleguide.readthedocs.io/en/latest/google-python-styleguide/python_language_rules/>
- 《编写高质量代码：改善 Python 程序的91个建议》
