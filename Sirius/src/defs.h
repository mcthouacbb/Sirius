#pragma once

#include <cstdint>
#include <iostream>
#include <bitset>
#include <bit>
#include <cassert>
#include <cstdint>

enum class PieceType
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE
};

enum class Color
{
    WHITE,
    BLACK
};

constexpr Color operator~(const Color& c)
{
    return static_cast<Color>(static_cast<int>(c) ^ 1);
}

enum class Piece : uint8_t {
    NONE = static_cast<int>(PieceType::NONE)
};

inline Piece makePiece(PieceType type, Color color)
{
    return Piece((static_cast<int>(color) << 3) | static_cast<int>(type));
}

inline PieceType getPieceType(Piece piece)
{
    return static_cast<PieceType>(static_cast<int>(piece) & 0b111);
}

inline Color getPieceColor(Piece piece)
{
    return static_cast<Color>(static_cast<int>(piece) >> 3);
}


enum class MoveType
{
    NONE = 0 << 12,
    ENPASSANT = 1 << 12,
    PROMOTION = 2 << 12,
    CASTLE = 3 << 12
};

enum class Promotion
{
    KNIGHT = 0 << 14,
    BISHOP = 1 << 14,
    ROOK = 2 << 14,
    QUEEN = 3 << 14
};

inline PieceType promoPiece(Promotion promo)
{
    static const PieceType promoPieces[4] = {
        PieceType::KNIGHT,
        PieceType::BISHOP,
        PieceType::ROOK,
        PieceType::QUEEN
    };

    return promoPieces[static_cast<int>(promo) >> 14];
}

struct Move
{
public:
    Move() = default;
    Move(int src, int dst, MoveType type);
    Move(int src, int dst, MoveType type, Promotion promotion);

    bool operator==(const Move& other) const = default;
    bool operator!=(const Move& other) const = default;

    int srcPos() const;
    int dstPos() const;
    int fromTo() const;
    MoveType type() const;
    Promotion promotion() const;
private:
    static constexpr int TYPE_MASK = 3 << 12;
    static constexpr int PROMOTION_MASK = 3 << 14;
    uint16_t m_Data;
};

inline Move::Move(int src, int dst, MoveType type)
    : m_Data(0)
{
    assert(src >= 0 && src < 64 && "Src pos is out of range");
    assert(dst >= 0 && dst < 64 && "Dst pos is out of range");
    m_Data = static_cast<uint16_t>(src | (dst << 6) | static_cast<int>(type));
}

inline Move::Move(int src, int dst, MoveType type, Promotion promotion)
    : m_Data(0)
{
    assert(src >= 0 && src < 64 && "Src pos is out of range");
    assert(dst >= 0 && dst < 64 && "Dst pos is out of range");
    m_Data = static_cast<uint16_t>(src | (dst << 6) | static_cast<int>(type) | static_cast<int>(promotion));
}


inline int Move::srcPos() const
{
    return m_Data & 63;
}

inline int Move::dstPos() const
{
    return (m_Data >> 6) & 63;
}

inline int Move::fromTo() const
{
    return m_Data & 4095;
}

inline MoveType Move::type() const
{
    return static_cast<MoveType>(m_Data & TYPE_MASK);
}

inline Promotion Move::promotion() const
{
    return static_cast<Promotion>(m_Data & PROMOTION_MASK);
}

constexpr int MAX_PLY = 128;

constexpr int SCORE_MAX = 32767;
constexpr int SCORE_MATE = 32700;
constexpr int SCORE_MATE_IN_MAX = SCORE_MATE - MAX_PLY;
constexpr int SCORE_WIN = 31000;
constexpr int SCORE_DRAW = 0;
constexpr int SCORE_NONE = -32701;

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
    friend constexpr PackedScore operator*(int a, const PackedScore& b);
    friend constexpr PackedScore operator*(const PackedScore& a, int b);
private:
    constexpr PackedScore(int value)
        : m_Value(value)
    {
    }

    int32_t m_Value;
};

constexpr PackedScore::PackedScore(int mg, int eg)
    : m_Value((static_cast<int32_t>(static_cast<uint32_t>(eg) << 16) + mg))
{

}

constexpr PackedScore& PackedScore::operator+=(const PackedScore& other)
{
    m_Value += other.m_Value;
    return *this;
}

constexpr PackedScore& PackedScore::operator-=(const PackedScore& other)
{
    m_Value -= other.m_Value;
    return *this;
}

constexpr int PackedScore::mg() const
{
    return static_cast<int16_t>(m_Value);
}

constexpr int PackedScore::eg() const
{
    return static_cast<int16_t>(static_cast<uint32_t>(m_Value + 0x8000) >> 16);
}

constexpr PackedScore operator+(const PackedScore& a, const PackedScore& b)
{
    return PackedScore(a.m_Value + b.m_Value);
}

constexpr PackedScore operator-(const PackedScore& a, const PackedScore& b)
{
    return PackedScore(a.m_Value - b.m_Value);
}

constexpr PackedScore operator-(const PackedScore& p)
{
    return PackedScore(-p.m_Value);
}

constexpr PackedScore operator*(int a, const PackedScore& b)
{
    return PackedScore(a * b.m_Value);
}

constexpr PackedScore operator*(const PackedScore& a, int b)
{
    return PackedScore(a.m_Value * b);
}

inline int fileOf(int square)
{
    return square % 8;
}

inline int rankOf(int square)
{
    return square / 8;
}

template<Color c>
inline int relativeRankOf(int square)
{
    if constexpr (c == Color::BLACK)
        square ^= 56;
    return rankOf(square);
}

inline int chebyshev(int sq1, int sq2)
{
    return std::max(std::abs(rankOf(sq1) - rankOf(sq2)), std::abs(fileOf(sq1) - fileOf(sq2)));
}
