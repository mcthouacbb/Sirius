#include "tt.h"
#include "eval/eval.h"

#include <climits>

TT::TT(size_t size)
	: m_Size(size)
{
	m_CurrAge = 0;
	m_Entries = new TTEntry[size]();
}

TT::~TT()
{
	delete[] m_Entries;
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

TTEntry* TT::probe(ZKey key, int ply, int& score, Move& move)
{
<<<<<<< Updated upstream
	TTBucket& bucket = m_Buckets[key.value % m_Size];
||||||| constructed merge base
	// if (key.value % m_Size == 784962)
		// std::cout << "OKOK" << std::endl;
	TTBucket& bucket = m_Buckets[key.value % m_Size];
=======
	TTEntry* entry = &m_Entries[key.value & (m_Size - 1)];

	if (entry->upperKey == key.value >> 48)
	{
		score = retrieveScore(entry->score, ply);
		move = entry->bestMove;
	}

	return entry;
	// if (key.value % m_Size == 784962)
		// std::cout << "OKOK" << std::endl;
	/*TTBucket& bucket = m_Buckets[key.value % m_Size];
>>>>>>> Stashed changes
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
		return &bucket;
	}

	if (entry->depth >= depth)
	{
		switch (entry->bound())
		{
			case TTEntry::Bound::NONE:
				break;
			case TTEntry::Bound::EXACT:
				score = retrieveScore(entry->score, ply);
				break;
			case TTEntry::Bound::LOWER_BOUND:
				if (beta <= entry->score)
					score = retrieveScore(entry->score, ply);
				break;
			case TTEntry::Bound::UPPER_BOUND:
				if (alpha >= entry->score)
					score = retrieveScore(entry->score, ply);
				break;
		}
	}

	move = entry->bestMove;

	return &bucket;*/
}

<<<<<<< Updated upstream
void TT::store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Bound bound)
||||||| constructed merge base
void TT::store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Type type)
=======
void TT::store(TTEntry* entry, ZKey key, int depth, int ply, int score, Move move, TTEntry::Bound bound)
>>>>>>> Stashed changes
{
<<<<<<< Updated upstream
	// 16 bit keys to save space
	// idea from JW
	uint16_t key16 = key.value >> 48;
||||||| constructed merge base
	for (int i = 0; i < ENTRY_COUNT; i++)
	{
		if (bucket->entries[i].key == key)
		{
			bucket->entries[i].depth = static_cast<uint8_t>(depth);
			bucket->entries[i].score = static_cast<int16_t>(storeScore(score, ply));
			bucket->entries[i].bestMove = move;
			bucket->entries[i].type = type;
			bucket->entries[i].age = static_cast<uint8_t>(m_CurrAge);
			return;
		}
	}
=======
	int ageDiff;
	if (m_CurrAge >= entry->gen())
		ageDiff = m_CurrAge - entry->gen();
	else
		ageDiff = m_CurrAge - entry->gen() + GEN_CYCLE_LENGTH;
>>>>>>> Stashed changes

<<<<<<< Updated upstream
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
||||||| constructed merge base
	int bestDepth = 256;
	TTEntry* replace = nullptr;
	for (int i = 0; i < ENTRY_COUNT; i++)
	{
		if (bucket->entries[i].age != m_CurrAge && bucket->entries[i].depth < bestDepth)
		{
			bestDepth = bucket->entries[i].depth;
			replace = &bucket->entries[i];
		}
	}

	if (!replace)
	{
		for (int i = 0; i < ENTRY_COUNT; i++)
		{
			if (bucket->entries[i].depth < bestDepth)
			{
				bestDepth = bucket->entries[i].depth;
				replace = &bucket->entries[i];
			}
		}
	}
=======
	// don't replace if depth + ageDiff * 2 < depth
	if (depth + ageDiff * 2 < entry->depth)
		return;
>>>>>>> Stashed changes

<<<<<<< Updated upstream
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
||||||| constructed merge base
	replace->key = key;
	replace->depth = static_cast<uint8_t>(depth);
	replace->score = static_cast<uint16_t>(storeScore(score, ply));
	replace->bestMove = move;
	replace->type = type;
	replace->age = static_cast<uint8_t>(m_CurrAge);
}
=======
	entry->upperKey = key.value >> 48;
	entry->depth = static_cast<uint8_t>(depth);
	entry->score = static_cast<uint16_t>(storeScore(score, ply));
	entry->bestMove = move;
	entry->setGenBound(bound, static_cast<uint8_t>(m_CurrAge));
}
>>>>>>> Stashed changes
