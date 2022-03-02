//#ifndef SAFEVECTOR_H_INCLUDED
//#define SAFEVECTOR_H_INCLUDED
#pragma once

#include <vector>
#include <mutex>

/**
 * @file SafeVector.h
 * @brief SafeVector is a lazy approach to mutex locking a std::vector object for thread safety
 */
template<typename T>
class SafeVector
{
    using iterator = T*;
    using const_iterator = T const*;

    public:
        SafeVector() : m_mutex(), m_vec() {} // default ctor
        SafeVector(const SafeVector& p) { copy(*this, p); } // copy ctor
        SafeVector(SafeVector&& p) noexcept { swap(*this, p); } // move ctor
        SafeVector& operator=(const SafeVector& p) { copy(*this, p); return *this; } // copy assignment
        SafeVector& operator=(SafeVector&& p) noexcept { swap(*this, p); return *this; } // move assignment
        SafeVector& operator=(const std::vector<T>& p) { m_vec = p; return *this; } // copy assignment from std::vector
        SafeVector& operator=(std::vector<T>&& p) noexcept { m_vec = p; return *this; } // move assignment from std::vector

        friend void copy(SafeVector& dst, const SafeVector& src) { if(&dst != &src) { dst.m_vec = src.m_vec; } }
        friend void swap(SafeVector& dst, SafeVector& src) { if(&dst != &src) { std::swap(dst.m_vec, src.m_vec); } }

        T& front() // never called without empty() or size() > 0 check
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_vec.front();
        }

        T& back() // never called without empty() or size() > 0 check
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_vec.back();
        }

        T& operator[](size_t n)
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_vec[n];
        }

        const T& operator[](size_t n) const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_vec[n];
        }

        iterator begin()
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return &*m_vec.begin();
        }

        const_iterator begin() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return &*m_vec.begin();
        }

        iterator end()
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return &*m_vec.end();
        }

        const_iterator end() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return &*m_vec.end();
        }

        bool contains(const T& item)
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            for(unsigned int i = 0; i < m_vec.size(); i++)
            {
                if(m_vec[i] == item) { return true; }
            }

            // else
            return false;
        }

        bool empty() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_vec.empty();
        }

        unsigned int size() const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_vec.size();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_vec.empty()) { return; } // quick exit
            else { std::vector<T>().swap(m_vec); }
        }

        void erase(size_t pos)
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            m_vec[pos] = m_vec.back();
            m_vec.pop_back();
        }

        void pop_back()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(!m_vec.empty()) { m_vec.pop_back(); }
        }

        void push_back(const T& item)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_vec.push_back(item);
        }

        void setVector(const std::vector<T>& vec)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_vec = vec;
        }

        void setVector(const SafeVector<T>& svec)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_vec = svec.m_vec;
        }

        /// \TODO: TEST THIS!
        template <class... Args>
        void emplace_back(Args&&... args)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_vec.emplace_back(std::forward<Args>(args)...);
        }

    protected:
        mutable std::mutex m_mutex;
        std::vector<T> m_vec;
};

//#endif // SAFEVECTOR_H_INCLUDED
