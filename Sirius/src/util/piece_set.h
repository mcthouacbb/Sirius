#pragma once

#include "../defs.h"

struct PieceSet
{
public:
    constexpr PieceSet()
        : m_Value(0)
    {

    }

    template<typename Type, typename... Types>
    constexpr PieceSet(Type type, Types... types)
        : m_Value(PieceSet(types...).m_Value)
    {
        static_assert(std::is_same_v<Type, PieceType>, "Piece set constructor argument must be a Piecetype");
        add(type);
    }

    constexpr void add(PieceType piece)
    {
        m_Value |= (1ull << static_cast<int>(piece));
    }
    constexpr void remove(PieceType piece)
    {
        m_Value &= ~(1ull << static_cast<int>(piece));
    }

    constexpr bool has(PieceType piece) const
    {
        return static_cast<bool>((m_Value >> static_cast<int>(piece)) & 1);
    }

    constexpr bool hasAny(const PieceSet& other) const
    {
        return (m_Value & other.m_Value) > 0;
    }
private:
    uint8_t m_Value;
};

