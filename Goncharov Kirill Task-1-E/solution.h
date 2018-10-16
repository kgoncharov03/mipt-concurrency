#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

class PetersonMutex {
private:
    std::array< std::atomic<bool>, 2 > want_;
    std::atomic<int> victim_;
public:
    PetersonMutex() {
        want_[1].store(false);
        want_[0].store(false);
        victim_.store(0);
    }
    void lock(int thread_num) {
        want_[thread_num].store(true);
        victim_.store(thread_num);
        while(want_[1 - thread_num].load() && victim_ == thread_num) {
            std::this_thread::yield();
        }
    }
    void unlock(int thread_num) {
        want_[thread_num].store(false);
    }
};

std::pair<int, int> FindPowerOfTwo(int num) {
    int power = 0;
    int temp = num;
    while(temp >>= 1)
        ++power;
    if(1 << power == num)
        return std::make_pair(power, num);
    else
        return std::make_pair(power + 1, 1 << (power + 1));
}

int GetParentId(int num) {
    return (num - 1) / 2;
}

int GetEdge(int num) {
    return num % 2;
}

class TreeMutex {
private:
    int size_;
    int levels_;
    std::vector<PetersonMutex> mutex_tree_;
public:
    TreeMutex(std::size_t n_threads) {
        size_ = FindPowerOfTwo(n_threads).second - 1;
        levels_ = FindPowerOfTwo(n_threads).first;
        mutex_tree_ = std::vector<PetersonMutex>(size_);
    }
    void lock(std::size_t current_thread) {
        int tree_index = size_ + current_thread;
        while (tree_index > 0) {
            mutex_tree_[GetParentId(tree_index)].lock(GetEdge(tree_index));
            tree_index = GetParentId(tree_index);
        }
    }
    void unlock(std::size_t current_thread) {
        int tree_index = size_ + current_thread;
        int way = 0;
        for(int i = 0; i < levels_; ++i) {
            if(GetEdge(tree_index) == 1)
                way |= 1 << i;
            tree_index = GetParentId(tree_index);
        }
        for(int i = levels_ - 1; i >= 0; --i) {
            if ((way >> i) & 1) {
                mutex_tree_[tree_index].unlock(1);
                tree_index = 2 * tree_index + 1;
            } else {
                mutex_tree_[tree_index].unlock(0);
                tree_index = 2 * tree_index + 2;
            }
        }
    }
};

