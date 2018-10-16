#pragma once

#include <atomic>
#include <thread>

template <typename T>
class LockFreeStack {
    struct Node {
        T element_;
        std::atomic<Node*> next_;
        Node(T element) : element_(element), next_(nullptr) {}
    };

    void Delete(std::atomic<Node*> top) {
        Node* current_top = top_.load();
        while (current_top) {
            Node *temp = current_top->next_;
            delete current_top;
            current_top = temp;
        }
    }

    void AddToDelete(std::atomic<Node*> current_top) {
        Node* new_delete = current_top;
        current_top = top_to_delete_.load();
        new_delete->next_.store(current_top);
        while (true) {
            if (top_to_delete_.compare_exchange_strong(current_top, new_delete)) {
                break;
            }
            new_delete->next_.store(current_top);
        }
    }

public:
    explicit LockFreeStack() {}

    ~LockFreeStack() {
        Delete(top_);
        Delete(top_to_delete_);
    }

    void Push(T element) {
        Node* new_top = new Node(element);
        Node* current_top = top_.load();
        new_top->next_ = current_top;
        while (true) {
            if (top_.compare_exchange_strong(current_top, new_top)) {
                return;
            }
            new_top->next_ = current_top;
        }
    }

    bool Pop(T& element) {
        Node* current_top = top_.load();
        while (true) {
            if (!current_top) {
                return false;
            }
            if (top_.compare_exchange_strong(current_top, current_top->next_)) {
                element = current_top->element_;
                AddToDelete(current_top);
                return true;
            }
        }
    }

private:
    std::atomic<Node*> top_{nullptr};
    std::atomic<Node*> top_to_delete_{nullptr};
};


template <typename T>
using ConcurrentStack = LockFreeStack<T>;

