#pragma once

#include "../defs.h"

template<typename T, size_t N>
struct EnumArray : public std::array<T, N>
{
    using std::array<T, N>::operator[];

    T& operator[](T p)
    {
        return (*this)[static_cast<int>(p)];
    }

    const T& operator[](T p) const
    {
        return (*this)[static_cast<int>(p)];
    }
};

using ColorArray = EnumArray<Color, 2>;
using PieceTypeArray = EnumArray<PieceType, 6>;
