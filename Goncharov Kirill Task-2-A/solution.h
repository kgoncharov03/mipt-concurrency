#include <condition_variable>
#include <cstddef>
#include <mutex>

template <typename ConditionVariable = std::condition_variable>
class CyclicBarrier {
private:
    int num_threads_;
    int threads_finished_;
    int threads_waiting_;
    ConditionVariable in_cv_;
    ConditionVariable out_cv_;
    std::mutex mutex_;
public:
    CyclicBarrier(size_t num_threads) {
        num_threads_ = num_threads;
        threads_finished_ = 0;
        threads_waiting_ = 0;
    }
    void Pass() {
        std::unique_lock<std::mutex> lock(mutex_);
        in_cv_.wait(lock, [this] {return threads_finished_ == 0; });
        in_cv_.notify_all();
        if (threads_waiting_ + 1 == num_threads_) {
            threads_finished_ = num_threads_;
        }
        ++threads_waiting_;
        out_cv_.wait(lock, [this] {return threads_waiting_ == num_threads_; });
        out_cv_.notify_all();
        if (threads_finished_ - 1 == 0) {
            threads_waiting_ = 0;
        }
        --threads_finished_;
    }
};

