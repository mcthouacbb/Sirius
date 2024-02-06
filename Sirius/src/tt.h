#pragma once

#include "zobrist.h"
#include "defs.h"

#include <cstring>
#include <vector>
#include <array>
#include <algorithm>

struct TTEntry
{
    uint16_t key16;
    int16_t score;
    Move bestMove;
    uint8_t depth;
    // 2 bits bound(lower), 6 bits gen(upper)
    uint8_t genBound;

    enum class Bound : uint8_t
    {
        NONE,
        EXACT,
        LOWER_BOUND,
        UPPER_BOUND
    };

    uint8_t gen()
    {
        return genBound >> 2;
    }

    Bound bound()
    {
        return static_cast<Bound>(genBound & 3);
    }

    static uint8_t makeGenBound(uint8_t gen, Bound bound)
    {
        return static_cast<int>(bound) | (gen << 2);
    }
};

static_assert(sizeof(TTEntry) == 8, "TTEntry must be 8 bytes");

static constexpr int ENTRY_COUNT = 4;

struct alignas(32) TTBucket
{
    std::array<TTEntry, ENTRY_COUNT> entries;
};

class TT
{
public:
    static constexpr int GEN_CYCLE_LENGTH = 1 << 6;

    TT(size_t size);
    ~TT() = default;

    void resize(int mb);

    TT(const TT&) = delete;
    TT& operator=(const TT&) = delete;

    TTBucket* probe(ZKey key, bool& found, int ply, int& score, Move& move, int& depth, TTEntry::Bound& bound);
    void store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Bound type);
    int quality(int age, int depth) const;
    void prefetch(ZKey key) const;

    void incAge()
    {
        m_CurrAge = (m_CurrAge + 1) % GEN_CYCLE_LENGTH;
    }

    void reset()
    {
        std::fill(m_Buckets.begin(), m_Buckets.end(), TTBucket{});
        m_CurrAge = 0;
    }
private:
    uint32_t index(uint64_t key) const;

    std::vector<TTBucket> m_Buckets;
    int m_CurrAge;
};
