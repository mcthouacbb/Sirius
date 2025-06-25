#pragma once

#include <array>
#include <cstddef>

// stormphrax yoink
namespace internal
{

template<typename T, size_t N, size_t... Ns>
struct MultiArrayImpl
{
    using Type = std::array<typename MultiArrayImpl<T, Ns...>::Type, N>;
};

template<typename T, size_t N>
struct MultiArrayImpl<T, N>
{
    using Type = std::array<T, N>;
};

}

template<typename T, size_t... Ns>
using MultiArray = typename internal::MultiArrayImpl<T, Ns...>::Type;
