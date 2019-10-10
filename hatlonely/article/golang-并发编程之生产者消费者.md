---
title: golang 并发编程之生产者消费者
date: 2018-03-11 16:53:07
tags: [golang, 并发编程, 生产者消费者, 多线程, sync.WaitGroup]
---

golang 最吸引人的地方可能就是并发了，无论代码的编写上，还是性能上面，golang 都有绝对的优势

学习一个语言的并发特性，我喜欢实现一个生产者消费者模型，这个模型非常经典，适用于很多的并发场景，下面我通过这个模型，来简单介绍一下 golang 的并发编程

### go 并发语法

#### 协程 `go`

**协程**是 golang 并发的最小单元，类似于其他语言的线程，只不过线程的实现借助了操作系统的实现，每次线程的调度都是一次系统调用，需要从用户态切换到内核态，这是一项非常耗时的操作，因此一般的程序里面线程太多会导致大量的性能耗费在线程切换上。而在 golang 内部实现了这种调度，协程在这种调度下面的切换非常的轻量级，成百上千的协程跑在一个 golang 程序里面是很正常的事情

golang 为并发而生，启动一个协程的语法非常简单，使用 `go` 关键字即可

``` go
go func () {
    // do something
}()
```

#### 同步信号 `sync.WaitGroup`

多个协程之间可以通过 `sync.WaitGroup` 同步，这个类似于 Linux 里面的信号量

``` go
var wg sync.WaitGroup  // 声明一个信号量
wg.Add(1)   // 信号量加一
wg.Done()   // 信号量减一
wg.Wait()   // 信号量为正时阻塞，直到信号量为0时被唤醒
```

#### 通道 `chan`

通道可以理解为一个消息队列，生产者往队列里面放，消费者从队列里面取。通道可以使用 `close` 关闭

``` go
ic := make(chan int, 10)  // 申明一个通道
ic <- 10        // 往通道里面放
i := <- ic      // 从通道里面取

close(ic)       // 关闭通道
```

### 生产者消费者实现

#### 定义产品类

这个产品类根据具体的业务需求定义

``` go
type Product struct {
    name  int
    value int
}
```

#### 生产者

如果 `stop` 标志不为 `false`，不断地往通道里面放 `product`，完成之后信号量完成

``` go
func producer(wg *sync.WaitGroup, products chan<- Product, name int, stop *bool) {
    for !*stop {
        product := Product{name: name, value: rand.Int()}
        products <- product
        fmt.Printf("producer %v produce a product: %#v\n", name, product)
        time.Sleep(time.Duration(200+rand.Intn(1000)) * time.Millisecond)
    }
    wg.Done()
}
```

#### 消费者

不断地从通道里面取 product，然后作对应的处理，直到通道被关闭，并且 products 里面为空， for 循环才会终止，而这正是我们期望的

``` go
func consumer(wg *sync.WaitGroup, products <-chan Product, name int) {
    for product := range products {
        fmt.Printf("consumer %v consume a product: %#v\n", name, product)
        time.Sleep(time.Duration(200+rand.Intn(1000)) * time.Millisecond)
    }
    wg.Done()
}
```

#### 主线程

``` go
var wgp sync.WaitGroup
var wgc sync.WaitGroup
stop := false
products := make(chan Product, 10)

// 创建 5 个生产者和 5 个消费者
for i := 0; i < 5; i++ {
    go producer(&wgp, products, i, &stop)
    go consumer(&wgc, products, i)
    wgp.Add(1)
    wgc.Add(1)
}

time.Sleep(time.Duration(1) * time.Second)
stop = true     // 设置生产者终止信号
wgp.Wait()      // 等待生产者退出
close(products) // 关闭通道
wgc.Wait()      // 等待消费者退出
```


