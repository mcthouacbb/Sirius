#pragma once

#include "defs.h"
#include "zobrist.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <thread>
#include <vector>

struct TTEntry
{
    u16 key16;
    i16 score;
    i16 staticEval;
    Move bestMove;
    u8 depth;
    // 2 bits bound, 1 bit pv, 5 bits gen
    u8 genBoundPV;

    enum class Bound : u8
    {
        NONE,
        EXACT,
        LOWER_BOUND,
        UPPER_BOUND
    };

    bool pv() const
    {
        return (genBoundPV >> 2) & 1;
    }

    u8 gen() const
    {
        return genBoundPV >> 3;
    }

    Bound bound() const
    {
        return static_cast<Bound>(genBoundPV & 3);
    }

    void setGenBoundPV(bool pv, u8 gen, Bound bound)
    {
        genBoundPV = static_cast<i32>(bound) | (pv << 2) | (gen << 3);
    }
};

static_assert(sizeof(TTEntry) == 10, "TTEntry must be 10 bytes");
static_assert(alignof(TTEntry) == 2, "TTEntry must have 2 byte alignment");

constexpr i32 ENTRY_COUNT = 3;
constexpr size_t TT_ALIGNMENT = 64;

struct alignas(32) TTBucket
{
    std::array<TTEntry, ENTRY_COUNT> entries;
    char padding[2];
};

struct ProbedTTData
{
    i32 score;
    i32 staticEval;
    Move move;
    i32 depth;
    bool pv;
    TTEntry::Bound bound;
};

class TT
{
public:
    static constexpr i32 GEN_CYCLE_LENGTH = 1 << 5;

    TT(size_t size);
    ~TT();

    void resize(i32 mb, i32 numThreads);

    TT(const TT&) = delete;
    TT& operator=(const TT&) = delete;

    bool probe(ZKey key, i32 ply, ProbedTTData& ttData);
    void store(ZKey key, i32 ply, i32 depth, i32 score, i32 staticEval, Move move, bool pv,
        TTEntry::Bound type);
    i32 quality(i32 age, i32 depth) const;
    void prefetch(ZKey key) const;

    void incAge()
    {
        m_CurrAge = (m_CurrAge + 1) % GEN_CYCLE_LENGTH;
    }

    void reset(i32 numThreads)
    {
        m_CurrAge = 0;
        std::vector<std::jthread> threads;
        threads.reserve(numThreads);

        for (i32 i = 0; i < numThreads; i++)
        {
            threads.emplace_back(
                [i, this, numThreads]()
                {
                    auto begin = m_Buckets + m_Size * i / numThreads;
                    auto end = m_Buckets + m_Size * (i + 1) / numThreads;
                    std::fill(begin, end, TTBucket{});
                });
        }
    }

    i32 hashfull() const;

private:
    u32 index(u64 key) const;

    TTBucket* m_Buckets;
    size_t m_Size;
    i32 m_CurrAge;
};
