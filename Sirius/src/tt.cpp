#include "tt.h"
#include "eval/eval.h"

TT::TT(size_t size)
	: m_Size(size)
{
	m_CurrAge = 0;
	m_Buckets = new TTBucket[size]();
}

TT::~TT()
{
	delete[] m_Buckets;
}

inline int retrieveScore(int score, int ply)
{
	if (eval::isMateScore(score))
	{
		score -= score < 0 ? -ply : ply;
	}
	return score;
}

inline int storeScore(int score, int ply)
{
	if (eval::isMateScore(score))
	{
		score += score < 0 ? -ply : ply;
	}
	return score;
}

TTBucket* TT::probe(ZKey key, int depth, int ply, int alpha, int beta, int& score, Move& move)
{
	// if (key.value % m_Size == 784962)
		// std::cout << "OKOK" << std::endl;
	TTBucket& bucket = m_Buckets[key.value % m_Size];
	TTEntry* entry = nullptr;
	for (int i = 0; i < ENTRY_COUNT; i++)
	{
		if (bucket.entries[i].key == key)
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
		switch (entry->type)
		{
			case TTEntry::Type::NONE:
				break;
			case TTEntry::Type::EXACT:
				score = retrieveScore(entry->score, ply);
				break;
			case TTEntry::Type::LOWER_BOUND:
				if (beta <= entry->score)
					score = retrieveScore(entry->score, ply);
				break;
			case TTEntry::Type::UPPER_BOUND:
				if (alpha >= entry->score)
					score = retrieveScore(entry->score, ply);
				break;
		}
	}

	move = entry->bestMove;

	return &bucket;
}

void TT::store(TTBucket* bucket, ZKey key, int depth, int ply, int score, Move move, TTEntry::Type type)
{
	// always replace an entry if it was already in the tt
	// idea from crafty
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

	int currQuality = quality(m_CurrAge, depth);
	TTEntry* replace = nullptr;
	for (int i = 0; i < ENTRY_COUNT; i++)
	{
		int entryQuality = quality(bucket->entries[i].age, bucket->entries[i].depth);
		if (entryQuality < currQuality)
		{
			currQuality = entryQuality;
			replace = &bucket->entries[i];
		}
	}

	// don't replace if current quality is worse than quality of entries already present
	if (!replace)
		return;

	replace->key = key;
	replace->depth = static_cast<uint8_t>(depth);
	replace->score = static_cast<uint16_t>(storeScore(score, ply));
	replace->bestMove = move;
	replace->type = type;
	replace->age = static_cast<uint8_t>(m_CurrAge);
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