#pragma once

#include "zobrist.h"
#include "defs.h"

struct TTEntry
{
	ZKey key;
	int16_t score;
	Move bestMove;
	uint8_t depth;
	enum class Type : uint8_t
	{
		NONE,
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND
	} type;
	uint8_t age;

	char padding; // ignored
};

static_assert(sizeof(TTEntry) == 16, "TTEntry must be 16 bytes");


constexpr int ENTRY_COUNT = 4;

struct alignas(64) TTBucket
{
	TTEntry entries[ENTRY_COUNT];
};

class TT
{
public:
	TT(size_t size);
	~TT();

	TT(const TT&) = delete;
	TT& operator=(const TT&) = delete;

	TTBucket* probe(ZKey key, int depth, int ply, int alpha, int beta, int& score, Move& move);
	void store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Type type);

	void incAge()
	{
		m_CurrAge = (m_CurrAge + 1) & 255;
	}

	void reset()
	{
		memset(m_Buckets, 0, m_Size * sizeof(TTBucket));
		m_CurrAge = 0;
	}
private:
	TTBucket* m_Buckets;
	size_t m_Size;
	int m_CurrAge;
};