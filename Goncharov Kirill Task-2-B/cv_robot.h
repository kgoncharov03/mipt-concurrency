#pragma once

#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>



class Robot {
public:
    void StepLeft() {
        std::unique_lock<std::mutex> lock(mutex_);
        step_cv_.wait(lock, [this] { return foot == kLeft; });
        std::cout << "left" << std::endl;
        ChangeFoot();
        step_cv_.notify_one();
    }

    void StepRight() {
        std::unique_lock<std::mutex> lock(mutex_);
        step_cv_.wait(lock, [this] { return foot == kRight; });
        std::cout << "right" << std::endl;
        ChangeFoot();
        step_cv_.notify_one();
    }

    void ChangeFoot() {
        if (foot == kLeft) {
           foot = kRight;
            return;
        }
        foot = kLeft;
        return;
    }

private:
    enum Direction { kLeft, kRight };
    Direction foot = kLeft;
    std::condition_variable step_cv_;
    std::mutex mutex_;
};
