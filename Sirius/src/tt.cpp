#include "tt.h"
#include "eval/eval.h"

#include <climits>

#include <iostream>

TT::TT(size_t size)
    : m_Buckets(size)
{
    m_CurrAge = 0;
}

// I'll change this later
void TT::resize(int mb)
{
    size_t size = mb * 1024 * 1024 / sizeof(TTBucket);
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

TTBucket* TT::probe(ZKey key, bool& found, int ply, int& score, Move& move, int& depth, TTEntry::Bound& bound)
{
    TTBucket& bucket = m_Buckets[key.value % m_Buckets.size()];
    TTEntry* entry = nullptr;
    uint16_t key16 = key.value >> 48;
    for (int i = 0; i < ENTRY_COUNT; i++)
    {
        if (bucket.entries[i].key16 == key16)
        {
            entry = &bucket.entries[i];
            break;
        }
    }

    if (!entry)
    {
        found = false;
        return &bucket;
    }
    found = true;

    score = retrieveScore(entry->score, ply);
    move = entry->bestMove;
    depth = entry->depth;
    bound = entry->bound();

    return &bucket;
}

void TT::store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Bound bound)
{
    // 16 bit keys to save space
    // idea from JW
    uint16_t key16 = key.value >> 48;

    int currQuality = INT_MAX;
    TTEntry* replace = nullptr;
    for (int i = 0; i < ENTRY_COUNT; i++)
    {
        if (bucket->entries[i].key16 == key16)
        {
            replace = &bucket->entries[i];
            break;
        }

        int entryQuality = quality(bucket->entries[i].gen(), bucket->entries[i].depth);
        if (entryQuality < currQuality)
        {
            currQuality = entryQuality;
            replace = &bucket->entries[i];
        }
    }

    // only overwrite the move if new move is not a null move or the entry is from a different position
    // idea from stockfish and ethereal
    if (move != Move() || replace->key16 != key16)
        replace->bestMove = move;

    // only overwrite if we have exact bound, different position, or same position and depth is not significantly worse
    // idea from stockfish and ethereal
    if (bound != TTEntry::Bound::EXACT &&
        replace->key16 == key16 &&
        depth < replace->depth - 2)
        return;

    replace->key16 = key16;
    replace->depth = static_cast<uint8_t>(depth);
    replace->score = static_cast<int16_t>(storeScore(score, ply));
    replace->genBound = TTEntry::makeGenBound(static_cast<uint8_t>(m_CurrAge), bound);
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

uint32_t TT::index(uint64_t key) const
{
    return static_cast<uint32_t>(key % m_Buckets.size());
}
