#ifndef _JAGUAR_BLOCKINGQUEUE_H_
#define _JAGUAR_BLOCKINGQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class BlockingQueue {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    BlockingQueue() {}
    
    void push(T value) {
        std::lock_guard<std::mutex> lck(mut);
        data_queue.push(std::move(value));
        data_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lck(mut);
        data_cond.wait(lck, [this] {
            return !data_queue.empty();
        });
        value = std::move(data_queue.front());
        data_queue.pop();
    }
};

#endif