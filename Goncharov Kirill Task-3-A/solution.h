#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>

template <class T, class Container = std::deque<T>>
class BlockingQueue {
private:
    std::mutex mutex_;
    size_t capacity_;
    Container queue_;
    bool state_;
    std::condition_variable cv_is_empty_;
    std::condition_variable cv_is_full_;
public:
    explicit BlockingQueue(const size_t& capacity);
    void Put(T&& element);
    bool Get(T& result);
    void Shutdown();
    bool empty() const;
    bool full() const;
};

template <class T, class Container>
BlockingQueue<T, Container>::BlockingQueue(const size_t &capacity) {
    state_ = true;
    capacity_ = capacity;
    if (capacity == 0) {
        throw std::exception();
    }
}

template <class T, class Container>
bool BlockingQueue<T, Container>::empty() const {
    return queue_.empty();
}

template <class T, class Container>
bool BlockingQueue<T, Container>::full() const {
    return (queue_.size() == capacity_);
}

template <class T, class Container>
void BlockingQueue<T, Container>::Put(T&& element) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!state_) {
        throw std::exception();
    }
    cv_is_full_.wait(lock, [this] { return !state_ || !full(); });
    if (!state_) {
        throw std::exception();
    }
    queue_.push_back(std::move(element));
    cv_is_empty_.notify_one();
}

template <class T, class Container>
bool BlockingQueue<T, Container>::Get(T &result) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_is_empty_.wait(lock, [this] { return !state_ || !empty(); });
    if (empty() && !state_) {
        return false;
    }
    result = std::move(queue_.front());
    queue_.pop_front();
    cv_is_full_.notify_one();
    return true;
}

template <class T, class Container>
void BlockingQueue<T, Container>::Shutdown() {
    std::unique_lock<std::mutex> lock(mutex_);
    state_ = false;
    cv_is_empty_.notify_all();
    cv_is_full_.notify_all();
}



