#include <vector>
#include "../zobrist.h"
#include "../defs.h"

struct PawnEntry
{
    ZKey pawnKey;
    PackedScore score;
};

class PawnTable
{
public:
    static constexpr size_t SIZE = 2048;
    static_assert((SIZE & (SIZE - 1)) == 0, "PawnCache::SIZE must be a power of 2");
    PawnTable();

    PawnEntry probe(ZKey pawnKey) const;
    void store(PawnEntry entry);

    void clear();
private:
    std::vector<PawnEntry> m_Entries;
};

inline PawnTable::PawnTable()
    : m_Entries(SIZE)
{

}

inline PawnEntry PawnTable::probe(ZKey pawnKey) const
{
    return m_Entries[pawnKey.value % SIZE];
}

inline void PawnTable::store(PawnEntry entry)
{
    m_Entries[entry.pawnKey.value % SIZE] = entry;
}

inline void PawnTable::clear()
{
    std::fill(m_Entries.begin(), m_Entries.end(), PawnEntry{});
}