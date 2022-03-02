//#ifndef SAFEUNORDEREDMAP_H
//#define SAFEUNORDEREDMAP_H
#pragma once

#include <unordered_map>
#include <mutex>

/**
 * @file SafeUnorderedMap.h
 * @brief SafeUnorderedMap is a lazy approach to mutex locking a std::unordered_map object for thread safety
 */
template <class Key,
    class T,
    class Hash = std::hash<Key>,
    class Pred = std::equal_to<Key>,
    class Allocator = std::allocator<std::pair<const Key, T> >,
    class SPair = std::pair<Key, T> >

class SafeUnorderedMap
{
    public:
        SafeUnorderedMap() : m_mutex(), m_uomap() {}; // default ctor
        SafeUnorderedMap(const SafeUnorderedMap& m) { copy(*this, m); } // copy ctor
        SafeUnorderedMap(SafeUnorderedMap&& m) noexcept { swap(*this, m); } // move ctor
        SafeUnorderedMap& operator=(const SafeUnorderedMap& m) { copy(*this, m); return *this; } // copy assignment
        SafeUnorderedMap& operator=(SafeUnorderedMap&& m) noexcept { swap(*this, m); return *this; } // move assignment

        friend void copy(SafeUnorderedMap& dst, const SafeUnorderedMap& src) { if(&dst != &src) { dst.m_uomap = src.m_uomap; } }
        friend void swap(SafeUnorderedMap& dst, SafeUnorderedMap& src) { if(&dst != &src) { std::swap(dst.m_uomap, src.m_uomap); } }

        typedef typename std::unordered_map<Key, T>::iterator iterator;
        typedef typename std::unordered_map<Key, T>::const_iterator const_iterator;

        T& operator[](const Key& keyVal)
        {
            // return existing
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap[keyVal];
        }

        T& operator[](Key&& keyVal)
        {
            // create new
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap[std::move(keyVal)];
        }

        T& at(const Key& keyVal)
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.at(keyVal);
        }

        const T& at(const Key& keyVal) const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.at(keyVal);
        }

        iterator begin()
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.begin();
        }

        const_iterator begin() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.begin();
        }

        iterator end()
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.end();
        }

        const_iterator end() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.end();
        }

        iterator find(const Key& keyVal)
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.find(keyVal);
        }

        const_iterator find(const Key& keyVal) const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.find(keyVal);
        }

        bool empty() const
        {
            std::unique_lock<std::mutex> ulock(m_mutex);
            return m_uomap.empty();
        }

        unsigned int size() const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_uomap.size();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_uomap.empty()) { return; } // quick exit
            else { std::unordered_map<Key, T>().swap(m_uomap); }
        }

        void erase(const Key& keyVal)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.erase(keyVal);
        }

        void erase(iterator& it)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.erase(it);
        }

        void erase(const_iterator& it)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.erase(it);
        }

        void insert(Key& k, T& t)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.insert(std::make_pair(k, t));
        }

        void insert(const Key& k, const T& t)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.insert(std::make_pair(k, t));
        }

        void insert(SPair&& sp)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.insert(sp);
        }

        void insert(const SPair&& sp)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_uomap.insert(sp);
        }

    private:
        mutable std::mutex m_mutex;
        std::unordered_map<Key, T> m_uomap;
};

//#endif // SAFEUNORDEREDMAP_H
