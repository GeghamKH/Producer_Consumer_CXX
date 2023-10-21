#pragma once

#include <mutex>
#include <condition_variable>

class semaphore {
    std::mutex m_mutex;
    std::condition_variable m_condition;
    unsigned long m_count = 0; // Initialized as locked.

public:
    void release() {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_count;
        m_condition.notify_one();
    }

    void acquire() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_count) // Handle spurious wake-ups.
            m_condition.wait(lock);
        --m_count;
    }
};