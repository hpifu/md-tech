---
title: c++ 实现 blocking queue
date: 2019-10-05 21:11:21
tags: [c++, blocking queue, 多线程, 生产者消费者]
---

阻塞队列就是多线程线程安全的队列，在多线程场景下经常用到，c++ 标准库里面没有提供阻塞队列，boost 中有提供，生成环境下可以使用

## blocking queue 实现

主要设计思路：

1. 使用 `std::vector` 实现一个环形队列，使用两个指针 start 和 end 来标识起止位置，push 的时候在 end 处插入，pop 的时候直接 start 指针往后移即可
2. 使用 `std::condition_variable` 实现同步，push 的时候需要满足 `_not_full` 条件，push 完成发送 `_not_empty` 通知，pop 的时候需要满足 `_not_empty` 条件，pop 完成发送 `_not_full` 通知

``` c++
template <typename T>
class BlockingQueue {
    std::mutex              _mutex;
    std::condition_variable _not_full;
    std::condition_variable _not_empty;
    int                     _start;
    int                     _end;
    int                     _capacity;
    std::vector<T>          _vt;

   public:
    BlockingQueue(const BlockingQueue<T>& other) = delete;
    BlockingQueue<T>& operator=(const BlockingQueue<T>& other) = delete;
    BlockingQueue(int capacity) : _capacity(capacity), _vt(capacity + 1), _start(0), _end(0) {}

    bool isempty() {
        return _end == _start;
    }

    bool isfull() {
        return (_start + _capacity - _end) % (_capacity + 1) == 0;
    }

    void push(const T& e) {
        std::unique_lock<std::mutex> lock(_mutex);
        while (isfull()) {
            _not_full.wait(lock);
        }

        _vt[_end++] = e;
        _end %= (_capacity + 1);
        _not_empty.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (isempty()) {
            _not_empty.wait(lock);
        }

        auto res = _vt[_start++];
        _start %= (_capacity + 1);
        _not_full.notify_one();
        return res;
    }
};
```

## 生成者消费者

生成者线程不断往队列面插入一个随机数，消费者线程从队列里面取

``` c++
void producer(BlockingQueue<int>& q, int i) {
    while (true) {
        std::random_device rd;
        auto               p = rd() % 10;
        q.push(p);
        std::cout << "produce " << i << " [" << p << "]" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500 + rd() % 5000));
    }
}

void consumer(BlockingQueue<int>& q, int i) {
    while (true) {
        std::random_device rd;
        auto               p = q.pop();
        std::cout << "consume " << i << " [" << p << "]" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500 + rd() % 5000));
    }
}

void producerConsumer() {
    BlockingQueue<int>       q(4);
    std::vector<std::thread> ps(10);
    std::vector<std::thread> cs(20);
    for (int i = 0; i < ps.size(); i++) {
        ps[i] = std::thread(producer, std::ref(q), i);
    }
    for (int i = 0; i < cs.size(); i++) {
        cs[i] = std::thread(consumer, std::ref(q), i);
    }
    for (int i = 0; i < ps.size(); i++) {
        ps[i].join();
    }
    for (int i = 0; i < ps.size(); i++) {
        cs[i].join();
    }
}
```

## 链接

- 完整代码: <https://github.com/hpifu/md-tech/blob/master/hatlonely/code/blocking-queue/blocking-queue.cpp>
