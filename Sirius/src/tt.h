#pragma once

#include "zobrist.h"
#include "defs.h"

#include <cstring>

struct TTEntry
{
<<<<<<< Updated upstream
	uint16_t key16;
||||||| constructed merge base
	ZKey key;
=======
	uint16_t upperKey;
>>>>>>> Stashed changes
	int16_t score;
	Move bestMove;
	uint8_t depth;
<<<<<<< Updated upstream
	// 2 bits bound(lower), 6 bits gen(upper)
	uint8_t genBound;

	enum class Bound : uint8_t
||||||| constructed merge base
	enum class Type : uint8_t
=======
	uint8_t genBound;

	enum class Bound
>>>>>>> Stashed changes
	{
		NONE,
<<<<<<< Updated upstream
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND
	};
||||||| constructed merge base
		EXACT,
		LOWER_BOUND,
		UPPER_BOUND
	} type;
	uint8_t age;
=======
		LOWER,
		UPPER,
		EXACT
	};
>>>>>>> Stashed changes

<<<<<<< Updated upstream
	uint8_t gen()
	{
		return genBound >> 2;
	}
||||||| constructed merge base
	char padding; // ignored
};
=======
	inline uint8_t gen() const
	{
		return genBound >> 2;
	}
>>>>>>> Stashed changes

<<<<<<< Updated upstream
	Bound bound()
	{
		return static_cast<Bound>(genBound & 3);
	}
||||||| constructed merge base
static_assert(sizeof(TTEntry) == 16, "TTEntry must be 16 bytes");
=======
	inline Bound bound() const
	{
		return static_cast<Bound>(genBound & 3);
	}
>>>>>>> Stashed changes

<<<<<<< Updated upstream
	static uint8_t makeGenBound(uint8_t gen, Bound bound)
	{
		return static_cast<int>(bound) | (gen << 2);
	}
};
||||||| constructed merge base
=======
	inline void setGenBound(Bound bound, uint8_t gen)
	{
		genBound = static_cast<uint8_t>(bound) | (gen << 2);
	}
};
>>>>>>> Stashed changes

static_assert(sizeof(TTEntry) == 8, "TTEntry must be 8 bytes");

<<<<<<< Updated upstream
static constexpr int ENTRY_COUNT = 4;

struct alignas(32) TTBucket
||||||| constructed merge base
struct alignas(64) TTBucket
=======
constexpr int BUCKET_SIZE = 4;

struct alignas(32) TTBucket
>>>>>>> Stashed changes
{
	TTEntry entries[BUCKET_SIZE];
};

class TT
{
public:
	static constexpr int GEN_CYCLE_LENGTH = 1 << 6;

	TT(size_t size);
	~TT();

	TT(const TT&) = delete;
	TT& operator=(const TT&) = delete;

<<<<<<< Updated upstream
	TTBucket* probe(ZKey key, int depth, int ply, int alpha, int beta, int& score, Move& move);
	void store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Bound type);
	int quality(int age, int depth) const;
||||||| constructed merge base
	TTBucket* probe(ZKey key, int depth, int ply, int alpha, int beta, int& score, Move& move);
	void store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Type type);
=======
	TTEntry* probe(ZKey key, int ply, int& score, Move& move);
	void store(TTEntry* entry, ZKey key, int depth, int ply, int score, Move move, TTEntry::Bound bound);
>>>>>>> Stashed changes

	void incAge()
	{
<<<<<<< Updated upstream
		m_CurrAge = (m_CurrAge + 1) & (GEN_CYCLE_LENGTH);
||||||| constructed merge base
		m_CurrAge = (m_CurrAge + 1) & 255;
=======
		m_CurrAge = (m_CurrAge + 1) & (GEN_CYCLE_LENGTH - 1);
>>>>>>> Stashed changes
	}

	void reset()
	{
		memset(m_Entries, 0, m_Size * sizeof(TTEntry));
		m_CurrAge = 0;
	}
private:
	static constexpr int GEN_CYCLE_LENGTH = 64;

	TTBucket* m_Buckets;
	size_t m_Size;
	int m_CurrAge;
};
