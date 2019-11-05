#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

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

void producer(BlockingQueue<int>& q, int i) {
    while (true) {
        std::random_device rd;
        auto               p = rd() % 10;
        q.push(p);
        std::cout << "produce " << i << " [" << p << "]" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1500 + rd() % 5000));
    }
}

void consumer(BlockingQueue<int>& q, int i) {
    while (true) {
        std::random_device rd;
        auto               p = q.pop();
        std::cout << "consume " << i << " [" << p << "]" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1500 + rd() % 5000));
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

int main(int argc, const char* argv[]) {
    producerConsumer();

    return 0;
}
