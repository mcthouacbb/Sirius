#include "tt.h"
#include "eval/eval.h"

#include <climits>

#if defined(_MSC_VER) && !defined(__clang__)
#include <mmintrin.h>
void prefetchAddr(const void* addr)
{
    return _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0);
}

uint64_t mulhi64(uint64_t a, uint64_t b)
{
    return __umulh(a, b);
}
#elif defined(_GNU_C) || defined(__clang__)
void prefetchAddr(const void* addr)
{
    return __builtin_prefetch(addr);
}

uint64_t mulhi64(uint64_t a, uint64_t b)
{
    unsigned __int128 result = static_cast<unsigned __int128>(a) * static_cast<unsigned __int128>(b);
    return result >> 64;
}
#else
void prefetchAddr(const void* addr)
{
    // fallback
}

// taken from stockfish https://github.com/official-stockfish/Stockfish/blob/master/src/misc.h#L158
uint64_t mulhi64(uint64_t a, uint64_t b)
{
    uint64_t aL = static_cast<uint32_t>(a), aH = a >> 32;
    uint64_t bL = static_cast<uint32_t>(b), bH = b >> 32;
    uint64_t c1 = (aL * bL) >> 32;
    uint64_t c2 = aH * bL + c1;
    uint64_t c3 = aL * bH + static_cast<uint32_t>(c2);
    return aH * bH + (c2 >> 32) + (c3 >> 32);
}
#endif


TT::TT(size_t size)
    : m_Buckets(size)
{
    m_CurrAge = 0;
}

// I'll change this later
void TT::resize(int mb)
{
    size_t size = static_cast<uint64_t>(mb) * 1024 * 1024 / sizeof(TTBucket);
    m_Buckets.resize(size, {});
    m_CurrAge = 0;
}

inline int retrieveScore(int score, int ply)
{
    if (isMateScore(score))
    {
        score -= score < 0 ? -ply : ply;
    }
    return score;
}

inline int storeScore(int score, int ply)
{
    if (isMateScore(score))
    {
        score += score < 0 ? -ply : ply;
    }
    return score;
}

bool TT::probe(ZKey key, int ply, ProbedTTData& ttData)
{
    size_t idx = index(key.value);
    TTBucket& bucket = m_Buckets[idx];
    int entryIdx = -1;
    uint16_t key16 = key.value & 0xFFFF;

    for (int i = 0; i < ENTRY_COUNT; i++)
    {
        if (bucket.entries[i].key16 == key16)
        {
            entryIdx = i;
            break;
        }
    }

    if (entryIdx == -1)
    {
        return false;
    }

    auto entry = bucket.entries[entryIdx];

    ttData.score = retrieveScore(entry.score, ply);
    ttData.staticEval = entry.staticEval;
    ttData.move = entry.bestMove;
    ttData.depth = entry.depth;
    ttData.bound = entry.bound();
    ttData.pv = entry.pv();

    return true;
}

void TT::store(ZKey key, int depth, int ply, int score, int staticEval, Move move, bool pv, TTEntry::Bound bound)
{
    // 16 bit keys to save space
    // idea from JW
    uint16_t key16 = key.value & 0xFFFF;
    TTBucket& bucket = m_Buckets[index(key.value)];
    int currQuality = INT_MAX;
    int replaceIdx = -1;
    for (int i = 0; i < ENTRY_COUNT; i++)
    {
        if (bucket.entries[i].key16 == key16)
        {
            replaceIdx = i;
            break;
        }

        int entryQuality = quality(bucket.entries[i].gen(), bucket.entries[i].depth);
        if (entryQuality < currQuality)
        {
            currQuality = entryQuality;
            replaceIdx = i;
        }
    }

    // only overwrite the move if new move is not a null move or the entry is from a different position
    // idea from stockfish and ethereal
    TTEntry& replace = bucket.entries[replaceIdx];
    if (move != Move() || replace.key16 != key16)
        replace.bestMove = move;

    // only overwrite if we have exact bound, different position, or same position and depth is not significantly worse
    // idea from stockfish and ethereal
    /*if (bound != TTEntry::Bound::EXACT &&
        entry.key16 == key16 &&
        depth < entry.depth - 2)
        return;*/

    if (bound == TTEntry::Bound::EXACT ||
        replace.key16 != key16 ||
        depth >= replace.depth - 2 - 2 * pv)
    {
        replace.key16 = key16;
        replace.staticEval = staticEval;
        replace.depth = static_cast<uint8_t>(depth);
        replace.score = static_cast<int16_t>(storeScore(score, ply));
        replace.genBoundPV = TTEntry::makeGenBoundPV(pv, static_cast<uint8_t>(m_CurrAge), bound);
    }
}

int TT::quality(int age, int depth) const
{
    int ageDiff = m_CurrAge - age;
    if (ageDiff < 0)
    {
        ageDiff += GEN_CYCLE_LENGTH;
    }
    return depth - 2 * ageDiff;
}

void TT::prefetch(ZKey key) const
{
    prefetchAddr(static_cast<const void*>(& m_Buckets[index(key.value)]));
}

uint32_t TT::index(uint64_t key) const
{
    return static_cast<uint32_t>(mulhi64(key, m_Buckets.size()));
}
