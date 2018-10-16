#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>
#include <forward_list>
#include <atomic>

template <typename T, class Hash = std::hash<T>>
class StripedHashSet {
private:
    Hash hash_;
    std::atomic<size_t> table_size_;
    size_t concurrency_level_;
    size_t growth_factor_;
    double load_factor_;
    std::vector<std::forward_list<T>> buckets_;
    std::vector<std::mutex> mutex_;

    size_t GetBucketIndex(const size_t hash_value) const {
        return hash_value % buckets_.size();
    }
    size_t GetStripeIndex(const size_t hash_value) const {
        return hash_value % concurrency_level_;
    }
    bool ElementIsFound(const size_t element_index, const T &element) {
        return std::count(buckets_[element_index].begin(), buckets_[element_index].end(), element) != 0;
    }

    void Rehash() {
        std::vector<std::unique_lock<std::mutex>> locks_;
        locks_.reserve(mutex_.size());
        for (size_t mutex_index = 0; mutex_index < mutex_.size(); ++mutex_index) {
            locks_.emplace_back(std::unique_lock<std::mutex>(mutex_[mutex_index]));
        }
        if (table_size_ <= buckets_.size() * load_factor_) {
            return;
        }
        std::vector<std::forward_list<T>> new_buckets_(buckets_.size() * growth_factor_);
        for (const auto& buckets_it : buckets_) {
            for (const auto& elements_it : buckets_it) {
                new_buckets_[hash_(elements_it) % new_buckets_.size()].push_front(elements_it);
            }
        }
        buckets_ = std::move(new_buckets_);
        return;
    }

public:
    explicit StripedHashSet(const size_t concurrency_level,
                            const size_t growth_factor = 3,
                            const double load_factor = 0.75) {
        concurrency_level_ = concurrency_level;
        table_size_ = 0;
        growth_factor_ = growth_factor;
        load_factor_ = load_factor;
        buckets_ = std::vector<std::forward_list<T>>(concurrency_level);
        mutex_ = std::vector<std::mutex>(concurrency_level);
    }

    bool Insert(const T& element) {
        const size_t element_hash = hash_(element);
        const size_t stripe_index = GetStripeIndex(element_hash);
        std::unique_lock<std::mutex> lock(mutex_[stripe_index]);
        const size_t element_index = GetBucketIndex(element_hash);
        if (ElementIsFound(element_index, element)) {
            return false;
        }
        buckets_[element_index].push_front(element);
        table_size_++;
        if (table_size_ > buckets_.size() * load_factor_) {
            lock.unlock();
            Rehash();
        }
        return true;
    }

    bool Remove(const T& element) {
        const size_t element_hash = hash_(element);
        const size_t stripe_index = GetStripeIndex(element_hash);
        std::unique_lock<std::mutex> lock(mutex_[stripe_index]);
        const size_t element_index = GetBucketIndex(element_hash);
        if (!ElementIsFound(element_index, element)) {
            return false;
        }
        buckets_[element_index].remove(element);
        table_size_--;
        return true;
    }

    bool Contains(const T& element) {
        const size_t element_hash = hash_(element);
        const size_t stripe_index = GetStripeIndex(element_hash);
        std::unique_lock<std::mutex> lock(mutex_[stripe_index]);
        const size_t element_index = GetBucketIndex(element_hash);
        return ElementIsFound(element_index, element);
    }

    size_t Size() {
        return table_size_.load();
    }
};


template <typename T> using ConcurrentSet = StripedHashSet<T>;
