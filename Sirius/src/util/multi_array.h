#pragma once

#include <array>
#include <cstddef>

// stormphrax yoink
namespace internal
{

template<typename T, usize N, usize... Ns>
struct MultiArrayImpl
{
    using Type = std::array<typename MultiArrayImpl<T, Ns...>::Type, N>;
};

template<typename T, usize N>
struct MultiArrayImpl<T, N>
{
    using Type = std::array<T, N>;
};

}

template<typename T, usize... Ns>
using MultiArray = typename internal::MultiArrayImpl<T, Ns...>::Type;
