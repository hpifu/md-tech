---
title: java 多线程
date: 2018-03-17 20:48:31
tags: [java, 多线程, 并发, runnable, thread, callable]
---

java 的多线程有好几种，可以继承 Thread，也可以实现 Runnable 接口，还可以实现 Callable 接口

### Thread

``` golang
class MyThread extends Thread {
    private String name;

    public MyThread(String name) {
        this.name = name;
    }

    @Override
    public void run() {
        for (int i = 0; i < 5; i++) {
            try {
                Thread.sleep(100L);
            } catch (Exception e) {
                e.printStackTrace();
            }
            System.out.printf("%s is running %d\n", name, i);
        }
    }
}

{
    Thread t1 = new MyThread("t1");
    Thread t2 = new MyThread("t2");

    t1.start();
    t2.start();
    try {
        t1.join();
        t2.join();
    } catch (Exception e) {
        e.printStackTrace();
    }
}
```

继承 Thread，自己实现 `run` 方法，就可以定一个线程类，调用 `start` 就可以在一个新的线程里面调用 `run` 方法，如果需要等待线程结束，可以调用 `join` 方法

### Runnable

``` golang
class MyRunnable implements Runnable {
    private String name;

    public MyRunnable(String name) {
        this.name = name;
    }

    @Override
    public void run() {
        for (int i = 0; i < 5; i++) {
            try {
                Thread.sleep(100L);
            } catch (Exception e) {
                e.printStackTrace();
            }
            System.out.printf("%s is running %d\n", name, i);
        }
    }
}

{
    Thread r1 = new Thread(new MyRunnable("r1"));
    Thread r2 = new Thread(new MyRunnable("r2"));

    r1.start();
    r2.start();
    try {
        r1.join();
        r2.join();
    } catch (Exception e) {
        e.printStackTrace();
    }
}
```

和 Thread 差不多，只不过不直接继承 Thread，而是实现 Runnable 接口（Runable 只有一个 `run` 方法），使用上面用这个 Runnable 去构造一个 Thread，这种方式相对直接继承 Thread 的方式要更加灵活，因为 java 是单继承，如果继承了 Thread 就不能再继承别的类

事实上，建议永远不要直接继承 Thread 类，因为从语义上来讲，Thread 也应该也只是方法运行的方式，你的类应该是可以在这种方式下运行，而不是一种 Thread 对象，从这个角度讲，Runnable 提供了更好的语义，用一个 Thread 对象去运行一个 Runable

### Callable

``` golang
class MyCallable implements Callable<Integer> {
    private Random random;

    public MyCallable() {
        this.random = new Random();
    }

    @Override
    public Integer call() throws Exception {
        Thread.sleep(100L);
        return this.random.nextInt();
    }
}

{
    FutureTask<Integer> future1 = new FutureTask<>(new MyCallable());
    FutureTask<Integer> future2 = new FutureTask<>(new MyCallable());
    new Thread(future1).start();
    new Thread(future2).start();

    try {
        System.out.println(future1.get(50, TimeUnit.MILLISECONDS));
    } catch (TimeoutException e) {
        System.out.println("future1 timeout");
    } catch (Exception e) {
        e.printStackTrace();
    }

    try {
        System.out.println(future2.get());
    } catch (Exception e) {
        e.printStackTrace();
    }
}
```

Callable 接口也只有一个方法 `call`，和 Runnable 不同的是 Callable 允许有返回值，而这个返回值可以通过 `FutureTask.get` 获取，还可以设置任务运行的超时时间，超时后会抛出一个异常

### ThreadPool

``` golang
class MyCallable implements Callable<Integer> {
    private Random random;

    public MyCallable() {
        this.random = new Random();
    }

    @Override
    public Integer call() throws Exception {
        Thread.sleep(100L);
        return this.random.nextInt();
    }
}

{
    ExecutorService es = Executors.newFixedThreadPool(5);
    Future<Integer> future1 = es.submit(new MyCallable());
    Future<Integer> future2 = es.submit(new MyCallable());

    try {
        System.out.println(future1.get(50, TimeUnit.MILLISECONDS));
    } catch (TimeoutException e) {
        System.out.println("future1 timeout");
    } catch (Exception e) {
        e.printStackTrace();
    }

    try {
        System.out.println(future2.get());
    } catch (Exception e) {
        e.printStackTrace();
    }

    es.shutdown();
}
```

java 里面线程的创建和销毁成本比较大，所以一般会需要放到线程池里面跑，java 的基础设施就是好，这些在标准库里面都有实现，使用上面也很简单，直接 new 出一个线程池就好了，然后就可以往里面 submit Callable 对象，线程池也有很多种，上面用到的 `newFixedThreadPool` 是固定线程数的线程池，下面用到的 `newCachedThreadPool` 在线程不够用的时候会创建新线程，同时也会不断复用之前创建的线程

```
{
    ExecutorService es = Executors.newCachedThreadPool();
    CompletionService<Integer> cs = new ExecutorCompletionService<>(es);
    cs.submit(new MyCallable());
    cs.submit(new MyCallable());
    cs.submit(new MyCallable());
    cs.submit(new MyCallable());

    try {
        System.out.println(cs.take().get());
        System.out.println(cs.take().get());
        System.out.println(cs.take().get());
        System.out.println(cs.take().get());
    } catch (Exception e) {
        e.printStackTrace();
    }

    es.shutdown();
}
```

典型的生成者消费者模型里面，我们需要把生产的结果放到一个队列里面，而消费者从这个队列里面不断地去消费，`ExecutorCompletionService` 就相当于这个队列，MyCallable 的结果会写入到缓存里面，使用 `cs.take().get()` 从里面取出结果

### 总结

线程的创建，销毁，切换在 java 里面都是耗性能的操作，如果有需求要大量地创建线程，尽量使用线程池去复用线程

### 参考链接

- 测试代码链接：<https://github.com/hatlonely/hellojava/blob/master/src/test/java/javase/ThreadTest.java>
- “implements Runnable” vs. “extends Thread” ：<https://stackoverflow.com/questions/541487/implements-runnable-vs-extends-thread>
