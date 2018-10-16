#pragma once

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>
#include <vector>

class Semaphore {
public:
    Semaphore() : capacity_(1), state_(1) {}

    explicit Semaphore(size_t num_threads, size_t state) : capacity_(num_threads), state_(state) {}

    void Acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (state_ == capacity_) {
            cv_.wait(lock, [this] { return state_ < capacity_; });
        }
        state_++;
    }

    void Release() {
        std::unique_lock<std::mutex> lock(mutex_);
        state_--;
        cv_.notify_one();
    }

private:
    std::condition_variable cv_;
    size_t capacity_;
    std::mutex mutex_;
    size_t state_;
};

class Robot {
public:
    Robot(const std::size_t num_foots) : num_foots_(num_foots), semaphores_(num_foots) {
        semaphores_[0].Release();
    }

    void Step(const std::size_t foot) {
        semaphores_[foot].Acquire();
        std::cout << "foot " << foot << std::endl;
        semaphores_[(foot + 1) % num_foots_].Release();
    }

private:
    size_t num_foots_;
    std::vector<Semaphore> semaphores_;
};

