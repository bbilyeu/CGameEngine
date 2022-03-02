//#ifndef SAFEQUEUE_H
//#define SAFEQUEUE_H
#pragma once

#include <queue>
#include <mutex>

/**
 * @file SafeQueue.h
 * @brief SafeQueue is a lazy approach to mutex locking a std::queue object for thread safety
 */
template <typename T>
class SafeQueue
{
    public:
        SafeQueue() : m_mutex(), m_queue() {}; // default ctor
        SafeQueue(const SafeQueue& p) { copy(*this, p); } // copy ctor
        SafeQueue(SafeQueue&& p) noexcept { swap(*this, p); } // move ctor
        SafeQueue& operator=(const SafeQueue& p) { copy(*this, p); return *this; } // copy assignment
        SafeQueue& operator=(SafeQueue&& p) noexcept { swap(*this, p); return *this; } // move assignment

        friend void copy(SafeQueue& dst, const SafeQueue& src) { if(&dst != &src) { dst.m_queue = src.m_queue; } }
        friend void swap(SafeQueue& dst, SafeQueue& src) { if(&dst != &src) { std::swap(dst.m_queue, src.m_queue); } }

        T& front() // never called without empty() or size() > 0 check
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_queue.front();
        }

        bool empty() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_queue.empty();
        }

        unsigned int size() const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_queue.size();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_queue.empty()) { return; } // quick exit
            else { std::queue<T>().swap(m_queue); }
        }

        void pop()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(!m_queue.empty()) { m_queue.pop(); }
        }

        void push(const T& item)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(item);
        }

        /// \TODO: TEST THIS!
        template <class... Args>
        void emplace(Args&&... args)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.emplace(std::forward<Args>(args)...);
        }

    protected:
        mutable std::mutex m_mutex;
        std::queue<T> m_queue;
};

//#endif // SAFEQUEUE_H
