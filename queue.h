#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadQueue {
public:
    ThreadQueue() {
    }
    
    void Push(const T& t) {
        {
            std::lock_guard<std::mutex> guard(m_);
            q_.push(t);
        }
        cond_var_.notify_one();
    }

    T Pop() {
        {
            std::unique_lock<std::mutex> lock(m_);
            if (q_.empty()) {
                cond_var_.wait(lock, [this] { return !q_.empty(); });
            }
            T res = q_.front();
            q_.pop();
            return res;
        }
    }

private:
    std::queue<T> q_;
    std::mutex m_;
    std::condition_variable cond_var_;
};

