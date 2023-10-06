#pragma once

#include <cstdint>
#include "../defs.h"

constexpr int SCORE_MAX = 32767;
constexpr int SCORE_MATE = 32700;
constexpr int SCORE_MATE_IN_MAX = SCORE_MATE - MAX_PLY;
constexpr int SCORE_WIN = 31000;
constexpr int SCORE_DRAW = 0;

inline bool isMateScore(int score)
{
    return std::abs(score) >= SCORE_MATE_IN_MAX;
}

struct PackedScore
{
public:
    constexpr PackedScore() = default;
    constexpr PackedScore(int mg, int eg);
    constexpr PackedScore& operator+=(const PackedScore& other);
    constexpr PackedScore& operator-=(const PackedScore& other);

    constexpr int mg() const;
    constexpr int eg() const;

    friend constexpr PackedScore operator+(const PackedScore& a, const PackedScore& b);
    friend constexpr PackedScore operator-(const PackedScore& a, const PackedScore& b);
    friend constexpr PackedScore operator-(const PackedScore& p);
private:
    constexpr PackedScore(int value)
        : m_Value(value)
    {}

    int32_t m_Value;
};

inline constexpr PackedScore::PackedScore(int mg, int eg)
    : m_Value((static_cast<int32_t>(static_cast<uint32_t>(eg) << 16) + mg))
{
    
}

inline constexpr PackedScore& PackedScore::operator+=(const PackedScore& other)
{
    m_Value += other.m_Value;
    return *this;
}

inline constexpr PackedScore& PackedScore::operator-=(const PackedScore& other)
{
    m_Value -= other.m_Value;
    return *this;
}

inline constexpr int PackedScore::mg() const
{
    return static_cast<int16_t>(m_Value);
}

inline constexpr int PackedScore::eg() const
{
    return static_cast<int16_t>(static_cast<uint32_t>(m_Value + 0x8000) >> 16);
}

inline constexpr PackedScore operator+(const PackedScore& a, const PackedScore& b)
{
    return PackedScore(a.m_Value + b.m_Value);
}

inline constexpr PackedScore operator-(const PackedScore& a, const PackedScore& b)
{
    return PackedScore(a.m_Value - b.m_Value);
}

inline constexpr PackedScore operator-(const PackedScore& p)
{
    return PackedScore(-p.m_Value);
}
