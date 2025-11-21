#pragma once

#include <bit>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;

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
    return static_cast<Color>(static_cast<i32>(c) ^ 1);
}

enum class Piece : u8
{
    NONE = static_cast<i32>(PieceType::NONE)
};

inline Piece makePiece(PieceType type, Color color)
{
    return Piece((static_cast<i32>(color) << 3) | static_cast<i32>(type));
}

inline PieceType getPieceType(Piece piece)
{
    return static_cast<PieceType>(static_cast<i32>(piece) & 0b111);
}

inline Color getPieceColor(Piece piece)
{
    return static_cast<Color>(static_cast<i32>(piece) >> 3);
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
        PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN};

    return promoPieces[static_cast<i32>(promo) >> 14];
}

constexpr i32 RANK_1 = 0;
constexpr i32 RANK_2 = 1;
constexpr i32 RANK_3 = 2;
constexpr i32 RANK_4 = 3;
constexpr i32 RANK_5 = 4;
constexpr i32 RANK_6 = 5;
constexpr i32 RANK_7 = 6;
constexpr i32 RANK_8 = 7;

constexpr i32 FILE_A = 0;
constexpr i32 FILE_B = 1;
constexpr i32 FILE_C = 2;
constexpr i32 FILE_D = 3;
constexpr i32 FILE_E = 4;
constexpr i32 FILE_F = 5;
constexpr i32 FILE_G = 6;
constexpr i32 FILE_H = 7;

struct Square
{
public:
    constexpr Square() = default;
    explicit constexpr Square(i32 sq);
    constexpr Square(i32 rank, i32 file);

    constexpr Square& operator+=(i32 other);
    constexpr Square& operator-=(i32 other);

    constexpr bool operator==(const Square& other) const = default;
    constexpr bool operator!=(const Square& other) const = default;
    constexpr bool operator>(const Square& other) const;
    constexpr bool operator>=(const Square& other) const;
    constexpr bool operator<(const Square& other) const;
    constexpr bool operator<=(const Square& other) const;

    constexpr Square& operator++();
    constexpr Square operator++(int);
    constexpr Square& operator--();
    constexpr Square operator--(int);

    constexpr Square operator+(i32 other) const;
    constexpr Square operator-(i32 other) const;
    constexpr i32 operator-(Square other) const;

    constexpr i32 value() const;
    constexpr i32 rank() const;
    constexpr i32 file() const;
    template<Color c>
    constexpr i32 relativeRank() const;
    constexpr i32 relativeRank(Color c) const;

    constexpr bool darkSquare() const;
    constexpr bool lightSquare() const;

    static constexpr i32 chebyshev(Square a, Square b);
    static constexpr i32 manhattan(Square a, Square b);
    static constexpr Square average(Square a, Square b);

private:
    u8 m_Value;
};

constexpr Square::Square(i32 sq)
    : m_Value(static_cast<u8>(sq))
{
    assert(sq < 64);
}

constexpr Square::Square(i32 rank, i32 file)
    : m_Value(static_cast<u8>(rank * 8 + file))
{
}

constexpr Square& Square::operator+=(i32 other)
{
    *this = *this + other;
    return *this;
}

constexpr Square& Square::operator-=(i32 other)
{
    *this = *this - other;
    return *this;
}

constexpr bool Square::operator>(const Square& other) const
{
    return value() > other.value();
}

constexpr bool Square::operator>=(const Square& other) const
{
    return value() >= other.value();
}

constexpr bool Square::operator<(const Square& other) const
{
    return value() < other.value();
}

constexpr bool Square::operator<=(const Square& other) const
{
    return value() <= other.value();
}

constexpr Square& Square::operator++()
{
    m_Value++;
    return *this;
}

constexpr Square Square::operator++(int)
{
    Square tmp = *this;
    operator++();
    return tmp;
}

constexpr Square& Square::operator--()
{
    m_Value--;
    return *this;
}

constexpr Square Square::operator--(int)
{
    Square tmp = *this;
    operator--();
    return tmp;
}

constexpr Square Square::operator+(i32 other) const
{
    return Square(value() + other);
}

constexpr Square Square::operator-(i32 other) const
{
    return Square(value() - other);
}

constexpr i32 Square::operator-(Square other) const
{
    return value() - other.value();
}

constexpr i32 Square::value() const
{
    return static_cast<i32>(m_Value);
}

constexpr i32 Square::rank() const
{
    return value() / 8;
}

constexpr i32 Square::file() const
{
    return value() % 8;
}

template<Color c>
constexpr i32 Square::relativeRank() const
{
    if constexpr (c == Color::BLACK)
        return rank() ^ 7;
    return rank();
}

constexpr i32 Square::relativeRank(Color c) const
{
    if (c == Color::BLACK)
        return rank() ^ 7;
    return rank();
}

constexpr bool Square::darkSquare() const
{
    return (rank() + file()) % 2 == 0;
}

constexpr bool Square::lightSquare() const
{
    return !darkSquare();
}

constexpr i32 Square::chebyshev(Square a, Square b)
{
    // hack cus std::abs isn't constexpr
    i32 rankDiff = a.rank() - b.rank();
    rankDiff = rankDiff < 0 ? -rankDiff : rankDiff;
    i32 fileDiff = a.file() - b.file();
    fileDiff = fileDiff < 0 ? -fileDiff : fileDiff;
    return std::max(rankDiff, fileDiff);
}

constexpr i32 Square::manhattan(Square a, Square b)
{
    // hack cus std::abs isn't constexpr
    i32 rankDiff = a.rank() - b.rank();
    rankDiff = rankDiff < 0 ? -rankDiff : rankDiff;
    i32 fileDiff = a.file() - b.file();
    fileDiff = fileDiff < 0 ? -fileDiff : fileDiff;
    return rankDiff + fileDiff;
}

constexpr Square Square::average(Square a, Square b)
{
    return Square((a.value() + b.value()) / 2);
}

struct Move
{
public:
    Move() = default;
    Move(Square from, Square to, MoveType type);
    Move(Square from, Square to, MoveType type, Promotion promotion);

    bool operator==(const Move& other) const = default;
    bool operator!=(const Move& other) const = default;

    static constexpr Move nullmove();

    Square fromSq() const;
    Square toSq() const;
    i32 fromTo() const;
    MoveType type() const;
    Promotion promotion() const;

private:
    static constexpr i32 TYPE_MASK = 3 << 12;
    static constexpr i32 PROMOTION_MASK = 3 << 14;
    u16 m_Data;
};

inline Move::Move(Square from, Square to, MoveType type)
    : m_Data(0)
{
    m_Data = static_cast<u16>(from.value() | (to.value() << 6) | static_cast<i32>(type));
}

inline Move::Move(Square from, Square to, MoveType type, Promotion promotion)
    : m_Data(0)
{
    m_Data = static_cast<u16>(
        from.value() | (to.value() << 6) | static_cast<i32>(type) | static_cast<i32>(promotion));
}

constexpr Move Move::nullmove()
{
    return Move();
}

inline Square Move::fromSq() const
{
    return Square(m_Data & 63);
}

inline Square Move::toSq() const
{
    return Square((m_Data >> 6) & 63);
}

inline i32 Move::fromTo() const
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

constexpr i32 MAX_PLY = 128;

constexpr i32 SCORE_MAX = 32767;
constexpr i32 SCORE_MATE = 32700;
constexpr i32 SCORE_MATE_IN_MAX = SCORE_MATE - MAX_PLY;
constexpr i32 SCORE_WIN = 31000;
constexpr i32 SCORE_DRAW = 0;
constexpr i32 SCORE_NONE = -32701;

inline bool isMateScore(i32 score)
{
    return std::abs(score) >= SCORE_MATE_IN_MAX;
}

struct ScorePair
{
public:
    constexpr ScorePair() = default;
    constexpr ScorePair(i32 mg, i32 eg);
    constexpr ScorePair& operator+=(const ScorePair& other);
    constexpr ScorePair& operator-=(const ScorePair& other);

    constexpr i32 mg() const;
    constexpr i32 eg() const;

    friend constexpr ScorePair operator+(const ScorePair& a, const ScorePair& b);
    friend constexpr ScorePair operator-(const ScorePair& a, const ScorePair& b);
    friend constexpr ScorePair operator-(const ScorePair& p);
    friend constexpr ScorePair operator*(i32 a, const ScorePair& b);
    friend constexpr ScorePair operator*(const ScorePair& a, i32 b);

private:
    constexpr ScorePair(i32 value)
        : m_Value(value)
    {
    }

    i32 m_Value;
};

constexpr ScorePair::ScorePair(i32 mg, i32 eg)
    : m_Value((static_cast<i32>(static_cast<u32>(eg) << 16) + mg))
{
}

constexpr ScorePair& ScorePair::operator+=(const ScorePair& other)
{
    m_Value += other.m_Value;
    return *this;
}

constexpr ScorePair& ScorePair::operator-=(const ScorePair& other)
{
    m_Value -= other.m_Value;
    return *this;
}

constexpr i32 ScorePair::mg() const
{
    return static_cast<i16>(m_Value);
}

constexpr i32 ScorePair::eg() const
{
    return static_cast<i16>(static_cast<u32>(m_Value + 0x8000) >> 16);
}

constexpr ScorePair operator+(const ScorePair& a, const ScorePair& b)
{
    return ScorePair(a.m_Value + b.m_Value);
}

constexpr ScorePair operator-(const ScorePair& a, const ScorePair& b)
{
    return ScorePair(a.m_Value - b.m_Value);
}

constexpr ScorePair operator-(const ScorePair& p)
{
    return ScorePair(-p.m_Value);
}

constexpr ScorePair operator*(i32 a, const ScorePair& b)
{
    return ScorePair(a * b.m_Value);
}

constexpr ScorePair operator*(const ScorePair& a, i32 b)
{
    return ScorePair(a.m_Value * b);
}
