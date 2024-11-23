#pragma once

#include <cstdint>
#include <iostream>
#include <bit>
#include <bitset>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include "defs.h"

class Bitboard
{
public:
    constexpr Bitboard() = default;
    explicit constexpr Bitboard(uint64_t v);
    static constexpr Bitboard fromSquare(Square sq);

    constexpr Bitboard operator<<(int other) const;
    constexpr Bitboard operator>>(int other) const;
    constexpr Bitboard operator&(const Bitboard& other) const;
    constexpr Bitboard operator|(const Bitboard& other) const;
    constexpr Bitboard operator^(const Bitboard& other) const;
    constexpr Bitboard operator~() const;
    constexpr Bitboard& operator<<=(int other);
    constexpr Bitboard& operator>>=(int other);
    constexpr Bitboard& operator&=(const Bitboard& other);
    constexpr Bitboard& operator|=(const Bitboard& other);
    constexpr Bitboard& operator^=(const Bitboard& other);

    constexpr bool operator==(const Bitboard& other) const = default;
    constexpr bool operator!=(const Bitboard& other) const = default;

    constexpr Bitboard north() const;
    constexpr Bitboard south() const;
    constexpr Bitboard west() const;
    constexpr Bitboard east() const;
    constexpr Bitboard northEast() const;
    constexpr Bitboard northWest() const;
    constexpr Bitboard southEast() const;
    constexpr Bitboard southWest() const;

    constexpr uint64_t value() const;

    Square lsb() const;
    Bitboard lsbBB() const;
    Square msb() const;
    uint32_t popcount() const;
    Square poplsb();
    constexpr bool has(Square sq) const;
    constexpr bool any() const;
    constexpr bool empty() const;
    constexpr bool multiple() const;
    constexpr bool one() const;

    friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);

    template<Color c, int r>
    static constexpr Bitboard nthRank();

    static constexpr Bitboard fileBB(int file);
private:
    uint64_t m_Value;
};

constexpr Bitboard::Bitboard(uint64_t v)
    : m_Value(v)
{

}

constexpr Bitboard FILE_A_BB = Bitboard(0x0101010101010101ull);
constexpr Bitboard FILE_B_BB = Bitboard(0x0202020202020202ull);
constexpr Bitboard FILE_C_BB = Bitboard(0x0404040404040404ull);
constexpr Bitboard FILE_D_BB = Bitboard(0x0808080808080808ull);
constexpr Bitboard FILE_E_BB = Bitboard(0x1010101010101010ull);
constexpr Bitboard FILE_F_BB = Bitboard(0x2020202020202020ull);
constexpr Bitboard FILE_G_BB = Bitboard(0x4040404040404040ull);
constexpr Bitboard FILE_H_BB = Bitboard(0x8080808080808080ull);

constexpr Bitboard RANK_1_BB = Bitboard(0x00000000000000FFull);
constexpr Bitboard RANK_2_BB = Bitboard(0x000000000000FF00ull);
constexpr Bitboard RANK_3_BB = Bitboard(0x0000000000FF0000ull);
constexpr Bitboard RANK_4_BB = Bitboard(0x00000000FF000000ull);
constexpr Bitboard RANK_5_BB = Bitboard(0x000000FF00000000ull);
constexpr Bitboard RANK_6_BB = Bitboard(0x0000FF0000000000ull);
constexpr Bitboard RANK_7_BB = Bitboard(0x00FF000000000000ull);
constexpr Bitboard RANK_8_BB = Bitboard(0xFF00000000000000ull);

constexpr Bitboard LIGHT_SQUARES_BB = Bitboard(0x55AA55AA55AA55AAull);
constexpr Bitboard DARK_SQUARES_BB = Bitboard(0xAA55AA55AA55AA55ull);

constexpr Bitboard Bitboard::fromSquare(Square sq)
{
    return Bitboard(1ull << sq.value());
}

constexpr Bitboard Bitboard::operator<<(int other) const
{
    return Bitboard(m_Value << other);
}

constexpr Bitboard Bitboard::operator>>(int other) const
{
    return Bitboard(m_Value >> other);
}

constexpr Bitboard Bitboard::operator&(const Bitboard& other) const
{
    return Bitboard(m_Value & other.m_Value);
}

constexpr Bitboard Bitboard::operator|(const Bitboard& other) const
{
    return Bitboard(m_Value | other.m_Value);
}

constexpr Bitboard Bitboard::operator^(const Bitboard& other) const
{
    return Bitboard(m_Value ^ other.m_Value);
}

constexpr Bitboard Bitboard::operator~() const
{
    return Bitboard(~m_Value);
}

constexpr Bitboard& Bitboard::operator<<=(int other)
{
    m_Value <<= other;
    return *this;
}

constexpr Bitboard& Bitboard::operator>>=(int other)
{
    m_Value >>= other;
    return *this;
}

constexpr Bitboard& Bitboard::operator&=(const Bitboard& other)
{
    m_Value &= other.m_Value;
    return *this;
}

constexpr Bitboard& Bitboard::operator|=(const Bitboard& other)
{
    m_Value |= other.m_Value;
    return *this;
}

constexpr Bitboard& Bitboard::operator^=(const Bitboard& other)
{
    m_Value ^= other.m_Value;
    return *this;
}


inline uint8_t reverse(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

inline std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard)
{
    uint64_t v = bitboard.m_Value;
    for (int i = 0; i < 8; i++)
    {
        uint8_t val = reverse(v >> 56);
        os << std::bitset<8>(val) << std::endl;
        v <<= 8;
    }
    os << std::endl;
    return os;
}

constexpr Bitboard Bitboard::north() const
{
    return Bitboard(m_Value << 8);
}

constexpr Bitboard Bitboard::south() const
{
    return Bitboard(m_Value >> 8);
}

constexpr Bitboard Bitboard::west() const
{
    return Bitboard(m_Value >> 1) & ~FILE_H_BB;
}

constexpr Bitboard Bitboard::east() const
{
    return Bitboard(m_Value << 1) & ~FILE_A_BB;
}

constexpr Bitboard Bitboard::northEast() const
{
    return north().east();
}
constexpr Bitboard Bitboard::northWest() const
{
    return north().west();
}
constexpr Bitboard Bitboard::southEast() const
{
    return south().east();
}
constexpr Bitboard Bitboard::southWest() const
{
    return south().west();
}

constexpr uint64_t Bitboard::value() const
{
    return m_Value;
}

inline Square Bitboard::lsb() const
{
    assert(m_Value > 0);

#if defined(__GNUC__) || defined(__clang__)
    return Square(__builtin_ctzll(m_Value));
#elif defined(_MSC_VER)
    unsigned long idx;
    _BitScanForward64(&idx, m_Value);
    return Square(idx);
#else
    return Square(std::countl_zero(m_Value));
#endif
}

inline Bitboard Bitboard::lsbBB() const
{
    assert(m_Value > 0);

    return Bitboard(m_Value & (0 - m_Value));
}

inline Square Bitboard::msb() const
{
    assert(m_Value > 0);

#if defined(__GNUC__) || defined(__clang__)
    return Square(63 - __builtin_clzll(m_Value));
#elif defined(_MSC_VER)
    unsigned long idx;
    _BitScanReverse64(&idx, m_Value);
    return Square(idx);
#else
    return Square(63 - std::countr_zero(m_Value));
#endif
}

inline uint32_t Bitboard::popcount() const
{
    return std::popcount(m_Value);
}

inline Square Bitboard::poplsb()
{
    assert(m_Value > 0);

    Square b = lsb();
    m_Value &= m_Value - 1;
    return b;
}

constexpr bool Bitboard::has(Square sq) const
{
    return ((m_Value >> sq.value()) & 1) > 0;
}

constexpr bool Bitboard::any() const
{
    return m_Value != 0;
}

constexpr bool Bitboard::empty() const
{
    return m_Value == 0;
}
constexpr bool Bitboard::multiple() const
{
    return (m_Value & (m_Value - 1)) != 0;
}

constexpr bool Bitboard::one() const
{
    return !empty() && !multiple();
}

template<Color c, int r>
constexpr Bitboard Bitboard::nthRank()
{
    if constexpr (c == Color::WHITE)
        return RANK_1_BB << (8 * r);
    else
        return RANK_8_BB >> (8 * r);
}

constexpr Bitboard Bitboard::fileBB(int file)
{
    return FILE_A_BB << file;
}
