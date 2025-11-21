#include "tt.h"
#include "eval/eval.h"

#include <climits>
#include <cstdlib>

#if defined(_MSC_VER) && !defined(__clang__)
#include <mmintrin.h>
void prefetchAddr(const void* addr)
{
    return _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0);
}

u64 mulhi64(u64 a, u64 b)
{
    return __umulh(a, b);
}
#elif defined(_GNU_C) || defined(__clang__)
void prefetchAddr(const void* addr)
{
    return __builtin_prefetch(addr);
}

u64 mulhi64(u64 a, u64 b)
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
u64 mulhi64(u64 a, u64 b)
{
    u64 aL = static_cast<u32>(a), aH = a >> 32;
    u64 bL = static_cast<u32>(b), bH = b >> 32;
    u64 c1 = (aL * bL) >> 32;
    u64 c2 = aH * bL + c1;
    u64 c3 = aL * bH + static_cast<u32>(c2);
    return aH * bH + (c2 >> 32) + (c3 >> 32);
}
#endif

void* alignedAlloc(size_t alignment, size_t size)
{
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    return std::aligned_alloc(alignment, size);
#endif
}

void alignedFree(void* ptr)
{
    if (ptr == nullptr)
        return;
#ifdef _WIN32
    return _aligned_free(ptr);
#else
    return std::free(ptr);
#endif
}

i32 retrieveScore(i32 score, i32 ply)
{
    if (isMateScore(score))
        score -= score < 0 ? -ply : ply;
    return score;
}

i32 storeScore(i32 score, i32 ply)
{
    if (isMateScore(score))
        score += score < 0 ? -ply : ply;
    return score;
}

TT::TT(size_t sizeMB)
    : m_Buckets(nullptr), m_Size(0), m_CurrAge(0)
{
    resize(sizeMB, 1);
}

TT::~TT()
{
    alignedFree(m_Buckets);
}

// I'll change this later
void TT::resize(i32 mb, i32 numThreads)
{
    size_t buckets = static_cast<u64>(mb) * 1024 * 1024 / sizeof(TTBucket);

    alignedFree(m_Buckets);
    m_Buckets = static_cast<TTBucket*>(alignedAlloc(TT_ALIGNMENT, buckets * sizeof(TTBucket)));
    m_Size = buckets;
    reset(numThreads);
    m_CurrAge = 0;
}

bool TT::probe(ZKey key, i32 ply, ProbedTTData& ttData)
{
    size_t idx = index(key.value);
    TTBucket& bucket = m_Buckets[idx];
    i32 entryIdx = -1;
    u16 key16 = key.value & 0xFFFF;

    for (i32 i = 0; i < ENTRY_COUNT; i++)
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

void TT::store(ZKey key, i32 ply, i32 depth, i32 score, i32 staticEval, Move move, bool pv,
    TTEntry::Bound bound)
{
    // 16 bit keys to save space
    // idea from JW
    u16 key16 = key.value & 0xFFFF;
    TTBucket& bucket = m_Buckets[index(key.value)];
    i32 currQuality = INT32_MAX;
    i32 replaceIdx = -1;
    for (i32 i = 0; i < ENTRY_COUNT; i++)
    {
        if (bucket.entries[i].key16 == key16)
        {
            replaceIdx = i;
            break;
        }

        i32 entryQuality = quality(bucket.entries[i].gen(), bucket.entries[i].depth);
        if (entryQuality < currQuality)
        {
            currQuality = entryQuality;
            replaceIdx = i;
        }
    }

    // only overwrite the move if new move is not a null move
    // or the entry is from a different position
    // idea from stockfish and ethereal
    TTEntry& replace = bucket.entries[replaceIdx];
    if (move != Move() || replace.key16 != key16)
        replace.bestMove = move;

    if (bound == TTEntry::Bound::EXACT || replace.key16 != key16
        || depth >= replace.depth - 2 - 2 * pv || replace.gen() != m_CurrAge)
    {
        replace.key16 = key16;
        replace.staticEval = staticEval;
        replace.depth = static_cast<u8>(depth);
        replace.score = static_cast<i16>(storeScore(score, ply));
        replace.setGenBoundPV(pv, static_cast<u8>(m_CurrAge), bound);
    }
}

i32 TT::quality(i32 age, i32 depth) const
{
    i32 ageDiff = m_CurrAge - age;
    if (ageDiff < 0)
    {
        ageDiff += GEN_CYCLE_LENGTH;
    }
    return depth - 2 * ageDiff;
}

void TT::prefetch(ZKey key) const
{
    prefetchAddr(static_cast<const void*>(&m_Buckets[index(key.value)]));
}

u32 TT::index(u64 key) const
{
    return static_cast<u32>(mulhi64(key, m_Size));
}

i32 TT::hashfull() const
{
    i32 count = 0;
    for (i32 i = 0; i < 1000; i++)
    {
        for (i32 j = 0; j < ENTRY_COUNT; j++)
        {
            const auto& entry = m_Buckets[i].entries[j];
            if (entry.bound() != TTEntry::Bound::NONE && entry.gen() == m_CurrAge)
                count++;
        }
    }
    return count / ENTRY_COUNT;
}
