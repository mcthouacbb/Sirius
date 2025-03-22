#pragma once

#include "../defs.h"
#include <array>

template<typename T, typename E, size_t N>
struct EnumArray : public std::array<T, N>
{
    using std::array<T, N>::operator[];

    T& operator[](E p)
    {
        return (*this)[static_cast<int>(p)];
    }

    const T& operator[](E p) const
    {
        return (*this)[static_cast<int>(p)];
    }
};

template<typename T>
using ColorArray = EnumArray<T, Color, 2>;

template<typename T>
using PieceTypeArray = EnumArray<T, PieceType, 6>;
