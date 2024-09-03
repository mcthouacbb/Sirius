#pragma once

#include "zobrist.h"
#include "defs.h"

#include <cstring>
#include <vector>
#include <array>
#include <algorithm>
#include <atomic>

struct TTEntry
{
    uint16_t key16;
    int16_t score;
    int16_t staticEval;
    Move bestMove;
    uint8_t depth;
    // 2 bits bound(lower), 6 bits gen(upper)
    uint8_t genBoundPV;

    enum class Bound : uint8_t
    {
        NONE,
        EXACT,
        LOWER_BOUND,
        UPPER_BOUND
    };

    bool pv()
    {
        return (genBoundPV >> 2) & 1;
    }

    uint8_t gen()
    {
        return genBoundPV >> 3;
    }

    Bound bound()
    {
        return static_cast<Bound>(genBoundPV & 3);
    }

    static uint8_t makeGenBoundPV(bool pv, uint8_t gen, Bound bound)
    {
        return static_cast<int>(bound) | (pv << 2) | (gen << 3);
    }
};

static_assert(sizeof(TTEntry) == 10, "TTEntry must be 10 bytes");
static_assert(alignof(TTEntry) == 2, "TTEntry must have 2 byte alignment");

static constexpr int ENTRY_COUNT = 3;

struct alignas(32) TTBucket
{
    std::array<TTEntry, ENTRY_COUNT> entries;
    char padding[2];
};

struct ProbedTTData
{
    int score;
    int staticEval;
    Move move;
    int depth;
    bool pv;
    TTEntry::Bound bound;
};

class TT
{
public:
    static constexpr int GEN_CYCLE_LENGTH = 1 << 5;

    TT(size_t size);
    ~TT() = default;

    void resize(int mb);

    TT(const TT&) = delete;
    TT& operator=(const TT&) = delete;

    bool probe(ZKey key, int ply, ProbedTTData& ttData);
    void store(ZKey key, int ply, int depth, int score, int staticEval, Move move, bool pv, TTEntry::Bound type);
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
