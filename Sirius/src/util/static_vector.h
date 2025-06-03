#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>

template<typename T, size_t Capacity>
class StaticVector
{
public:
    StaticVector() = default;

    void push_back(const T& elem)
    {
        assert(m_Size < Capacity);
        m_Data[m_Size++] = elem;
    }

    void push_back(T&& elem)
    {
        assert(m_Size < Capacity);
        m_Data[m_Size++] = std::move(elem);
    }

    void clear()
    {
        m_Size = 0;
    }

    void fill(const T& value)
    {
        std::fill(begin(), end(), value);
    }

    size_t size() const
    {
        return m_Size;
    }

    T* data()
    {
        return m_Data.data();
    }

    const T* data() const
    {
        return m_Data.data();
    }

    auto begin()
    {
        return m_Data.begin();
    }

    auto end()
    {
        return begin() + m_Size;
    }

    auto begin() const
    {
        return cbegin();
    }

    auto end() const
    {
        return cend();
    }

    auto cbegin() const
    {
        return m_Data.cbegin();
    }

    auto cend() const
    {
        return cbegin() + m_Size;
    }

    T& operator[](size_t pos)
    {
        assert(pos < m_Size);
        return m_Data[pos];
    }

    const T& operator[](size_t pos) const
    {
        assert(pos < m_Size);
        return m_Data[pos];
    }

    void resize(size_t size)
    {
        m_Size = size;
    }

private:
    std::array<T, Capacity> m_Data;
    size_t m_Size = 0;
};
