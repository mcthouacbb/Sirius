#pragma once

#include "../util/piece_set.h"
#include "../util/static_vector.h"
#include "pawn_structure.h"
#include "pawn_table.h"
#include "psqt_state.h"
#include <optional>

namespace eval
{

struct EvalUpdates
{
    struct Update
    {
        Piece piece;
        Square square;
    };
    StaticVector<Update, 2> adds;
    StaticVector<Update, 2> removes;
    PieceSet changedPieces;

    void pushAdd(Update update)
    {
        adds.push_back(update);
        changedPieces.add(getPieceType(update.piece));
    }

    void pushRemove(Update update)
    {
        removes.push_back(update);
        changedPieces.add(getPieceType(update.piece));
    }
};

struct EvalState
{
public:
    EvalState();

    void initSingle(const Board& board);
    void init(const Board& board, PawnTable& pawnTable);

    void push(const Board& board, const EvalUpdates& updates);
    void pop();

    ScorePair score(const Board& board) const;
    ScorePair psqtScore(const Board& board, Color c) const;
    ScorePair pawnShieldStormScore(Color c) const;
    const PawnStructure& pawnStructure() const;

private:
    void init(const Board& board, PawnTable* pawnTable);
    struct StackEntry
    {
        EvalUpdates updates;

        PsqtState psqtState;
        PawnStructure pawnStructure;
        ColorArray<ScorePair> pawnShieldStorm;
        ScorePair knightOutposts;
        ScorePair bishopPawns;
        ScorePair rookOpen;
        ScorePair minorBehindPawn;
    };

    StackEntry& currEntry()
    {
        return *m_CurrEntry;
    }

    const StackEntry& currEntry() const
    {
        return *m_CurrEntry;
    }

    std::array<StackEntry, 256> m_Stack;
    StackEntry* m_CurrEntry;
    PawnTable* m_PawnTable;
};

}
